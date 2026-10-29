"""
Import the generated Honeycomb avatar into a headless UE 5.8 editor and verify
that what arrived in the engine is what the generator claims: bone names, bone
hierarchy, and real-world scale. Those three are what AvatarAutoFit and any
future retargeting depend on, and none of them are visible in a render.

Run (editor must be closed):
  "C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe" ^
      "C:\\Project\\Honeycomb\\Honeycomb.uproject" -run=pythonscript ^
      -script="C:\\Project\\Honeycomb\\Content\\Python\\import_honeycomb_avatar.py" ^
      -unattended -nopause -nosplash -nullrhi

unreal.log() does not reach stdout in commandlet mode, so the whole result is
written to a JSON report file instead. Failures land in report["failures"].
"""
import json
import os
import traceback

import unreal

FBX = r"C:\Project\Honeycomb\Content\Avatars\SK_HoneycombAvatar.fbx"
DEST = "/Game/Avatars"
NAME = "SK_HoneycombAvatar"
MESH_PATH = DEST + "/" + NAME + "." + NAME
REPORT = r"C:\Project\Honeycomb\Tools\AvatarGen\ue_import_report.json"

# Reference stature of the generated mesh. Keep in step with metrics.json, which
# is what build_avatar.py is run against; this is the "authored" height that
# AvatarAutoFitComponent treats as unscaled.
EXPECTED_HEIGHT_CM = 178.0

# UE5 Manny hierarchy, mirrored from Tools/AvatarGen/verify_rig.py. If these two
# lists drift, one of the two was edited without the other.
EXPECTED_BONES = {
    "root": None,
    "pelvis": "root",
    "spine_01": "pelvis",
    "spine_02": "spine_01",
    "spine_03": "spine_02",
    "neck_01": "spine_03",
    "head": "neck_01",
    "clavicle_l": "spine_03",
    "upperarm_l": "clavicle_l",
    "lowerarm_l": "upperarm_l",
    "hand_l": "lowerarm_l",
    "clavicle_r": "spine_03",
    "upperarm_r": "clavicle_r",
    "lowerarm_r": "upperarm_r",
    "hand_r": "lowerarm_r",
    "thigh_l": "pelvis",
    "calf_l": "thigh_l",
    "foot_l": "calf_l",
    "thigh_r": "pelvis",
    "calf_r": "thigh_r",
    "foot_r": "calf_r",
}

report = {"steps": [], "errors": [], "failures": []}


def log(msg):
    report["steps"].append(str(msg))
    unreal.log("[honeycomb_import] " + str(msg))


def _jsonable(value):
    """Coerce UE Name/struct values into something json.dump accepts."""
    try:
        json.dumps(value)
        return value
    except (TypeError, ValueError):
        if isinstance(value, (list, tuple)):
            return [_jsonable(item) for item in value]
        if isinstance(value, dict):
            return {str(k): _jsonable(v) for k, v in value.items()}
        return str(value)


def probe(label, fn):
    """Call fn() and record the outcome, never raising."""
    try:
        value = fn()
        report[label] = _jsonable(value)
        return value
    except Exception as exc:  # noqa: BLE001
        report[label] = {"error": str(exc)}
        log("{} FAILED: {}".format(label, exc))
        return None


def expect(ok, label, detail=""):
    report["steps"].append("{0} {1}".format("ok  " if ok else "FAIL", label))
    if not ok:
        report["failures"].append(
            label + ((" -- " + str(detail)) if detail else "")
        )
    return ok


# --- import ----------------------------------------------------------------


def import_avatar():
    if not os.path.exists(FBX):
        raise RuntimeError("FBX missing: {}".format(FBX))

    task = unreal.AssetImportTask()
    task.filename = FBX
    task.destination_path = DEST
    task.destination_name = NAME
    task.automated = True
    task.replace_existing = True
    task.save = True

    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_as_skeletal = True
    options.import_animations = False
    options.import_materials = True
    options.import_textures = False
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH

    skel = options.skeletal_mesh_import_data
    # The Blender armature is exported in centimetres and already matches UE's
    # unit, so the import must not rescale, reorient or re-pose it.
    for key, value in (
        ("import_mesh_lods", False),
        ("update_skeleton_reference_pose", False),
        ("use_t0_as_ref_pose", False),
        ("convert_scene", True),
        ("preserve_smoothing_groups", True),
        ("update_local_bounds", True),
        ("use_all_morph_targets", True),
        ("force_all_mesh_as_type", False),
    ):
        if hasattr(skel, key):
            setattr(skel, key, value)
        else:
            log("skel option missing: {}".format(key))

    if hasattr(options, "import_uniform_scale"):
        options.import_uniform_scale = 1.0

    task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    report["fbx_size_kb"] = round(os.path.getsize(FBX) / 1024, 1)
    log("imported, fbx {} KB".format(report["fbx_size_kb"]))


# --- verification ----------------------------------------------------------


def enumerate_bones(skeleton, sub, mesh):
    """The full bone list of the imported skeleton.

    Skeleton.get_reference_pose() is the authoritative source and carries the
    reference transforms too. SkeletalMeshEditorSubsystem only exposes
    get_bone_children/get_bone_parent, so the fallback walks the tree from
    "root"; seeding with "" does not work, it logs a "Bone 'None' not found"
    error and returns nothing.
    """
    names = None
    pose = None
    try:
        pose = skeleton.get_reference_pose()
        if isinstance(pose, dict) and pose.get("bone_names"):
            names = [str(b) for b in pose["bone_names"]]
    except Exception as exc:  # noqa: BLE001
        log("get_reference_pose failed: {}".format(exc))

    children = {}

    def of(bone):
        if bone not in children:
            try:
                children[bone] = [str(x) for x in sub.get_bone_children(mesh, bone)]
            except Exception as exc:  # noqa: BLE001
                children[bone] = []
                log("get_bone_children({}) failed: {}".format(bone, exc))
        return children[bone]

    if names is None:
        names = ["root"]
        queue = list(of("root"))
        seen = set()
        while queue:
            bone = queue.pop(0)
            if bone in seen:
                continue
            seen.add(bone)
            names.append(bone)
            queue.extend(c for c in of(bone) if c not in seen)
        log("bone list came from the hierarchy walk, not the reference pose")

    return names, pose


def verify_skeleton(mesh, sub):
    skeleton = mesh.get_editor_property("skeleton")
    report["skeleton"] = skeleton.get_name()

    bones, pose = enumerate_bones(skeleton, sub, mesh)
    report["bone_names"] = bones
    report["bone_count"] = len(bones)

    expect(
        sorted(bones) == sorted(EXPECTED_BONES),
        "bone set matches the UE5 Manny convention",
        "unexpected={} missing={}".format(
            sorted(set(bones) - set(EXPECTED_BONES)),
            sorted(set(EXPECTED_BONES) - set(bones)),
        ),
    )

    parents = {}
    for bone in bones:
        try:
            raw = str(sub.get_bone_parent(mesh, bone))
        except Exception as exc:  # noqa: BLE001
            raw = "ERROR:{}".format(exc)
        # get_bone_parent reports "None" both for a missing bone and for the
        # root, so it cannot be used on its own to prove a bone exists.
        parents[bone] = "" if raw in ("None", "") else raw
    report["bone_parents"] = parents

    for bone in sorted(EXPECTED_BONES):
        if bone not in parents:
            continue
        expected = EXPECTED_BONES[bone] or ""
        if expected == "":
            # The FBX armature object becomes the parent of "root", so the
            # check is that root is not parented to one of our own bones.
            expect(
                parents[bone] not in EXPECTED_BONES,
                "root hangs off the armature, not off another bone",
                "got {!r}".format(parents[bone]),
            )
            continue
        expect(
            parents[bone] == expected,
            "parent of " + bone,
            "got {!r}, expected {!r}".format(parents[bone], expected),
        )

    verify_reference_pose(pose)
    return bones


def verify_reference_pose(pose):
    """Bone lengths measured from the reference pose the engine actually uses."""
    report["reference_pose_type"] = type(pose).__name__
    if not isinstance(pose, dict):
        report["reference_pose_preview"] = str(pose)[:300]
        log("get_reference_pose returned {}, not a dict".format(type(pose).__name__))
        return
    report["reference_pose_keys"] = sorted(pose.keys())

    translations = pose.get("ref_bone_pose") or pose.get("bone_pose")
    names = pose.get("bone_names")
    if not translations or not names:
        log("reference pose has no usable bone transforms")
        return

    # Each entry is a Transform; its location is the bone head in component
    # space, so consecutive names give us lengths without touching the mesh.
    def head(name):
        index = [str(n) for n in names].index(name)
        t = translations[index]
        loc = t.translation if hasattr(t, "translation") else t
        return [loc.x, loc.y, loc.z]

    import math

    def length(a, b):
        pa, pb = head(a), head(b)
        return math.sqrt(sum((pa[i] - pb[i]) ** 2 for i in range(3)))

    report["reference_head_top_cm"] = round(head("head")[2], 2)
    report["reference_foot_cm"] = round(
        min(head("foot_l")[2], head("foot_r")[2]), 2
    )
    expect(
        abs(head("head")[2] - EXPECTED_HEIGHT_CM) < 2.0,
        "skeleton head bone sits at the authored stature",
        "{:.2f} cm".format(head("head")[2]),
    )
    for parent, child in (
        ("pelvis", "thigh_l"),
        ("thigh_l", "calf_l"),
        ("calf_l", "foot_l"),
        ("upperarm_l", "lowerarm_l"),
        ("lowerarm_l", "hand_l"),
        ("spine_02", "spine_03"),
    ):
        expect(
            length(parent, child) > 0.5,
            "reference pose has a real length for " + parent + " -> " + child,
            "{:.3f} cm".format(length(parent, child)),
        )



def verify_scale(mesh, sub):
    # UE 5.8 has no imported_bounds editor property; get_bounds() is the
    # supported accessor and returns a BoxSphereBounds.
    bounds = probe("bounds", lambda: str(mesh.get_bounds()))
    box = mesh.get_bounds()
    height = 2.0 * box.box_extent.z
    width = 2.0 * box.box_extent.x
    report["height_cm"] = round(height, 2)
    report["apose_width_cm"] = round(width, 2)
    report["origin_cm"] = [
        round(box.origin.x, 2), round(box.origin.y, 2), round(box.origin.z, 2)
    ]

    expect(
        abs(height - EXPECTED_HEIGHT_CM) < 1.0,
        "mesh is 1:1 in UE units (metres), so height reads as authored stature",
        "{:.2f} cm vs {:.1f}".format(height, EXPECTED_HEIGHT_CM),
    )
    expect(
        box.box_extent.z > 0.5,
        "mesh is not collapsed to a plane",
        "{:.4f}".format(box.box_extent.z),
    )
    expect(
        abs(box.origin.z - height * 0.5) < 1.0,
        "mesh is centred vertically, soles at Z=0",
        "origin.z={:.2f}".format(box.origin.z),
    )

    # An A-pose avatar is roughly half to four-fifths as wide as it is tall.
    # Much narrower means the FBX axes were rotated or the arms never made it
    # across; much wider means the arms were left in a T-pose.
    expect(
        0.45 * height < width < 0.85 * height,
        "A-pose width is plausible for a standing body",
        "{:.1f} cm wide vs {:.1f} cm tall".format(width, height),
    )


def verify_mesh_data(mesh, sub):
    report["lod_count"] = probe("lod_count", lambda: sub.get_lod_count(mesh))
    report["lod0_verts"] = probe("lod0_verts", lambda: sub.get_num_verts(mesh, 0))
    for method in ("get_num_triangles", "get_num_vertices"):
        if hasattr(sub, method):
            probe("lod0_" + method, lambda m=method: getattr(sub, m)(mesh, 0))
    expect(
        bool(report.get("lod0_verts")),
        "LOD0 has vertices",
        report.get("lod0_verts"),
    )
    report["materials"] = probe(
        "materials", lambda: [m.material_slot_name for m in mesh.get_editor_property("materials")]
    )
    report["skeleton"] = probe(
        "skeleton", lambda: mesh.get_editor_property("skeleton").get_name()
    )


def main():
    import_avatar()

    mesh = unreal.load_asset(MESH_PATH)
    if mesh is None:
        report["errors"].append("mesh did not import")
        return
    report["mesh"] = mesh.get_name()
    report["mesh_class"] = mesh.get_class().get_name()

    sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    if sub is None:
        report["errors"].append("no SkeletalMeshEditorSubsystem")
        return

    verify_mesh_data(mesh, sub)
    verify_skeleton(mesh, sub)
    verify_scale(mesh, sub)

    probe("save", lambda: bool(
        unreal.EditorAssetLibrary.save_asset(MESH_PATH, only_if_is_dirty=False)
    ))


try:
    main()
except Exception:  # noqa: BLE001
    report["errors"].append(traceback.format_exc())
finally:
    report["passed"] = not report["failures"] and not report["errors"]
    try:
        with open(REPORT, "w", encoding="utf-8") as handle:
            # default=str: UE hands back Name/Transform wrappers that the json
            # module does not know about.
            json.dump(
                report, handle, indent=2, ensure_ascii=False, default=str
            )
    except Exception as exc:  # noqa: BLE001
        unreal.log_error("[honeycomb_import] report write failed: {}".format(exc))
