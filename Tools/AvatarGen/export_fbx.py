"""
Export existing skeletal meshes to FBX for reference inspection.

Blender cannot read .uasset, so the reference meshes are exported to FBX and
probed there. This tells us exactly what geometry the VR rig already uses
before the generator replaces the placeholder body.

Run headless:
  UnrealEditor-Cmd.exe Honeycomb.uproject -run=pythonscript ^
      -script="C:\\Project\\Honeycomb\\Tools\\AvatarGen\\export_fbx.py" ^
      -unattended -nopause -nosplash -nullrhi
"""
import json
import os
import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(HERE, "reference_fbx")
OUT_JSON = os.path.join(HERE, "reference_export.json")

ASSETS = [
    "/Game/XRMannequins/Meshes/SKM_MannyXR_left",
    "/Game/XRMannequins/Meshes/SKM_MannyXR_right",
    "/Game/XRMannequins/Meshes/SKM_QuinnXR_left",
    "/Game/XRMannequins/Meshes/SKM_QuinnXR_right",
    "/Game/Avatars/SK_ParametricMannequin",
]


def bone_count(mesh):
    """Bone count via the working two-argument children call."""
    sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    for root in ("root", "pelvis"):
        try:
            kids = [str(k) for k in sub.get_bone_children(mesh, root)]
        except Exception:  # noqa: BLE001
            continue
        if kids:
            count, stack = 0, [root]
            while stack:
                bone = stack.pop()
                count += 1
                try:
                    stack.extend(str(k) for k in sub.get_bone_children(mesh, bone))
                except Exception:  # noqa: BLE001
                    pass
            return count, root
    return None, None


def main():
    if not os.path.isdir(OUT_DIR):
        os.makedirs(OUT_DIR)

    report = []

    for path in ASSETS:
        mesh = unreal.load_asset(path)
        if mesh is None:
            report.append({"asset": path, "error": "not found"})
            continue

        count, root = bone_count(mesh)
        entry = {
            "asset": path,
            "bone_count": count,
            "root_bone": root,
            "materials": [
                str(m.material_slot_name) for m in mesh.get_editor_property("materials")
            ],
        }

        task = unreal.AssetExportTask()
        task.set_editor_property("object", mesh)
        task.set_editor_property("filename", os.path.join(OUT_DIR, os.path.basename(path) + ".fbx"))
        task.set_editor_property("automated", True)
        task.set_editor_property("prompt", False)
        task.set_editor_property("replace_identical", True)
        task.set_editor_property("use_file_archive", False)

        options = unreal.FbxExportUI()
        options.set_editor_property("skeletal_mesh_export_content", True)
        options.set_editor_property("skeleton_export", True)
        options.set_editor_property("mesh", True)
        options.set_editor_property("anim_sequence_export", False)
        options.set_editor_property("level_sequence_export", False)
        options.set_editor_property("static_mesh_export", False)
        options.set_editor_property("lighting_export", False)
        options.set_editor_property("camera_export", False)
        options.set_editor_property("material_export", False)
        options.set_editor_property("texture_export", False)
        options.set_editor_property("bake_anim", False)
        options.set_editor_property("path_mode", unreal.FBXExportPathMode.FBXPATHMODE_AUTOMATIC)
        task.set_editor_property("options", options)

        try:
            unreal.AssetToolsHelpers.get_asset_tools().export_assets_tasks([task])
            entry["fbx"] = os.path.basename(task.get_editor_property("filename"))
        except Exception as exc:  # noqa: BLE001
            entry["export_error"] = str(exc)

        report.append(entry)

    with open(OUT_JSON, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2, ensure_ascii=False)

    for item in report:
        unreal.log(
            "[export_fbx] {} bones={} -> {}".format(
                item.get("asset"), item.get("bone_count"), item.get("export_error", "ok")
            )
        )


try:
    main()
except Exception as exc:  # noqa: BLE001
    unreal.log("[export_fbx] EXCEPTION {}".format(exc))