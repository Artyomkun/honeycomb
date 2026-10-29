"""
Inventory every SkeletalMesh in the project and describe BP_MannequinsXR.

Needed because SK_MannequinsXR is a hand-only rig: the avatar body must be
sourced from somewhere else, so we need to know what mesh the existing VR
blueprint actually drives.

Run headless:
  UnrealEditor-Cmd.exe Honeycomb.uproject -run=pythonscript ^
      -script="C:\\Project\\Honeycomb\\Tools\\AvatarGen\\inventory_bodies.py" ^
      -unattended -nopause -nosplash -nullrhi
"""
import json
import os
import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "body_inventory.json")

BLUEPRINT = "/Game/XRMannequins/BP_MannequinsXR"


def describe_mesh(path):
    mesh = unreal.load_asset(path)
    if mesh is None:
        return None

    entry = {"asset": path}

    try:
        skeleton = mesh.get_editor_property("skeleton")
        entry["skeleton"] = skeleton.get_path_name() if skeleton else None
    except Exception as exc:  # noqa: BLE001
        entry["skeleton_error"] = str(exc)

    try:
        mats = [str(m.material_slot_name) for m in mesh.get_editor_property("materials")]
        entry["material_slots"] = mats
    except Exception as exc:  # noqa: BLE001
        entry["materials_error"] = str(exc)

    # Bone count via the editor subsystem walk, from the skeleton root.
    try:
        sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
        for root in ("root", "pelvis"):
            try:
                kids = sub.get_bone_children(mesh, root, False)
            except Exception:  # noqa: BLE001
                continue
            if kids:
                count, stack = 0, [root]
                while stack:
                    b = stack.pop()
                    count += 1
                    try:
                        stack.extend(sub.get_bone_children(mesh, b, False))
                    except Exception:  # noqa: BLE001
                        pass
                entry["bone_count"] = count
                entry["root_bone"] = root
                break
    except Exception as exc:  # noqa: BLE001
        entry["bones_error"] = str(exc)

    return entry


def describe_blueprint(path):
    bp = unreal.load_asset(path)
    if bp is None:
        return {"asset": path, "error": "not found"}

    entry = {"asset": path, "class": bp.get_class().get_name()}

    gen_class = None
    try:
        gen_class = bp.generated_class()
        if gen_class:
            entry["generated_class"] = gen_class.get_name()
    except Exception as exc:  # noqa: BLE001
        entry["generated_class_error"] = str(exc)

    # Mesh assets referenced by the blueprint's skeleton mesh components.
    refs = []
    try:
        meshes = []
        bp.get_all_nodes_of_class(unreal.SkeletalMeshComponent.__class__, None)
    except Exception:  # noqa: BLE001
        pass

    try:
        for obj in unreal.load_asset(path).get_outer() if False else []:
            pass
    except Exception:  # noqa: BLE001
        pass

    if gen_class is not None:
        try:
            cdo = unreal.get_default_object(gen_class)
            entry["cdo"] = cdo.get_name()
            comps = []
            for comp in cdo.get_components_by_class(unreal.SkeletalMeshComponent):
                comps.append(
                    {
                        "name": comp.get_name(),
                        "mesh": str(comp.get_editor_property("skeletal_mesh")),
                        "anim_mode": str(comp.get_editor_property("animation_mode")),
                        "anim_class": str(comp.get_editor_property("anim_class")),
                    }
                )
            entry["skeletal_mesh_components"] = comps
        except Exception as exc:  # noqa: BLE001
            entry["cdo_error"] = str(exc)

    return entry


def main():
    report = {"skeletal_meshes": [], "blueprint": None}

    for path in unreal.EditorAssetLibrary.list_assets("/Game", recursive=True):
        base = path.split(".")[0]
        data = unreal.EditorAssetLibrary.find_asset_data(base)
        if not data.is_valid():
            continue
        cls = str(data.asset_class_path.asset_name)
        if cls == "SkeletalMesh":
            info = describe_mesh(base)
            if info:
                report["skeletal_meshes"].append(info)
        elif cls == "Blueprint" and base == BLUEPRINT:
            report["blueprint"] = describe_blueprint(base)

    with open(OUT, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2, ensure_ascii=False)

    unreal.log(
        "[inventory_bodies] meshes={} -> {}".format(
            len(report["skeletal_meshes"]), OUT
        )
    )


try:
    main()
except Exception as exc:  # noqa: BLE001
    unreal.log("[inventory_bodies] EXCEPTION {}".format(exc))