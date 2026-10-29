"""
Export the project's Mannequin XR rig: bone hierarchy, parents, reference pose.

The generator needs the exact hierarchy the project animates against, read out
of the engine rather than from documentation.

Walk uses SkeletalMeshEditorSubsystem.get_bone_children / get_bone_parent,
which is the API that actually exists in 5.8 (ref_skeleton is not exposed to
Python any more).

Run headless:
  UnrealEditor-Cmd.exe Honeycomb.uproject -run=pythonscript ^
      -script="C:\\Project\\Honeycomb\\Tools\\AvatarGen\\export_rig.py" ^
      -unattended -nopause -nosplash -nullrhi
"""
import json
import os
import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "mannequin_xr_rig.json")

SKELETON = "/Game/XRMannequins/Meshes/SK_MannequinsXR.SK_MannequinsXR"
MESHES = [
    "/Game/XRMannequins/Meshes/SKM_MannyXR_left.SKM_MannyXR_left",
    "/Game/XRMannequins/Meshes/SKM_MannyXR_right.SKM_MannyXR_right",
    "/Game/XRMannequins/Meshes/SKM_QuinnXR_left.SKM_QuinnXR_left",
    "/Game/XRMannequins/Meshes/SKM_QuinnXR_right.SKM_QuinnXR_right",
]

ROOT_CANDIDATES = ["root", "Root", "pelvis", "Pelvis"]


def children_of(mesh, bone):
    """Direct children of a bone; virtual bones are excluded from the rig."""
    sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    for args in (
        (mesh, bone, False),
        (mesh, bone),
    ):
        try:
            return [str(n) for n in sub.get_bone_children(*args)]
        except Exception:  # noqa: BLE001
            continue
    raise RuntimeError("get_bone_children failed for {}".format(bone))


def parent_of(mesh, bone):
    sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    try:
        return str(sub.get_bone_parent(mesh, bone, False))
    except Exception:  # noqa: BLE001
        try:
            return str(sub.get_bone_parent(mesh, bone))
        except Exception:  # noqa: BLE001
            return None


def walk(mesh):
    """Depth-first from the root so bone order matches the engine's own."""
    root = None
    for candidate in ROOT_CANDIDATES:
        try:
            if unreal.AnimationLibrary.does_bone_name_exist(SKELETON, candidate):
                root = candidate
                break
        except Exception:  # noqa: BLE001
            pass
        if children_of(mesh, candidate):
            root = candidate
            break

    if root is None:
        raise RuntimeError("no root bone found")

    order = []
    seen = set()
    stack = [root]
    while stack:
        name = stack.pop()
        if name in seen:
            continue
        seen.add(name)
        order.append({"name": name, "parent": parent_of(mesh, name)})
        kids = children_of(mesh, name)
        stack.extend(reversed(kids))

    return root, order


def mesh_report(mesh, path):
    entry = {"asset": path}
    try:
        entry["materials"] = [
            str(m.material_slot_name)
            for m in mesh.get_editor_property("materials")
        ]
    except Exception as exc:  # noqa: BLE001
        entry["materials_error"] = str(exc)
    try:
        entry["skeleton"] = mesh.get_editor_property("skeleton").get_path_name()
    except Exception as exc:  # noqa: BLE001
        entry["skeleton_error"] = str(exc)
    try:
        entry["bone_count_hint"] = len(entry.get("materials", []))
    except Exception:  # noqa: BLE001
        pass
    return entry


def main():
    report = {"skeleton": SKELETON, "meshes": []}

    primary = unreal.load_asset(MESHES[0])
    if primary is None:
        raise RuntimeError("mesh not found: {}".format(MESHES[0]))

    root, bones = walk(primary)
    report["root_bone"] = root
    report["bone_count"] = len(bones)
    report["bones"] = bones

    for path in MESHES:
        mesh = unreal.load_asset(path)
        report["meshes"].append(
            mesh_report(mesh, path) if mesh else {"asset": path, "error": "not found"}
        )

    with open(OUT, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2, ensure_ascii=False)
    unreal.log(
        "[export_rig] root={} bones={} -> {}".format(root, len(bones), OUT)
    )


try:
    main()
except Exception as exc:  # noqa: BLE001
    unreal.log("[export_rig] EXCEPTION {}".format(exc))