"""
Probe which asset-export pipeline classes exist in this engine build.

UE 5.8 replaced the legacy FBX exporter options with Interchange, so the
class name differs from older scripting references. Print what exists instead
of guessing, and try a default-pipeline export.

Run headless:
  UnrealEditor-Cmd.exe Honeycomb.uproject -run=pythonscript ^
      -script="C:\\Project\\Honeycomb\\Tools\\AvatarGen\\probe_export.py" ^
      -unattended -nopause -nosplash -nullrhi
"""
import json
import os
import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "export_probe.json")
SCRATCH = os.path.join(HERE, "reference_fbx")

MESH = "/Game/XRMannequins/Meshes/SKM_MannyXR_right"


def main():
    report = {}

    report["export_classes"] = sorted(
        n
        for n in dir(unreal)
        if "Export" in n
        and any(k in n for k in ("Pipeline", "Options", "Settings", "UI", "Task", "Fbx", "FBX"))
    )
    report["interchange_classes"] = sorted(
        n for n in dir(unreal) if n.startswith("Interchange") and "Node" not in n
    )

    mesh = unreal.load_asset(MESH)
    if mesh is None:
        report["error"] = "mesh missing"
    else:
        if not os.path.isdir(SCRATCH):
            os.makedirs(SCRATCH)
        target = os.path.join(SCRATCH, "SKM_MannyXR_right.fbx")

        task = unreal.AssetExportTask()
        task.set_editor_property("object", mesh)
        task.set_editor_property("filename", target)
        task.set_editor_property("automated", True)
        task.set_editor_property("prompt", False)
        task.set_editor_property("replace_identical", True)

        # Try a named pipeline first, then fall back to engine defaults.
        for class_name in ("InterchangeGenericAssetsPipeline",):
            cls = getattr(unreal, class_name, None)
            if cls is None:
                continue
            try:
                pipeline = cls()
                for prop, value in (
                    ("import_animations", False),
                    ("import_static_meshes", False),
                    ("import_materials", False),
                    ("import_textures", False),
                    ("import_skeletal_meshes", True),
                    ("export_skeletal_meshes", True),
                    ("export_animations", False),
                    ("export_static_meshes", False),
                    ("force_overwrite", True),
                ):
                    try:
                        pipeline.set_editor_property(prop, value)
                    except Exception:  # noqa: BLE001
                        pass
                task.set_editor_property("options", pipeline)
                report["pipeline_used"] = class_name
                break
            except Exception as exc:  # noqa: BLE001
                report["pipeline_error"] = str(exc)

        try:
            unreal.AssetToolsHelpers.get_asset_tools().export_assets_tasks([task])
            report["exported"] = os.path.exists(target)
            if report["exported"]:
                report["size_kb"] = round(os.path.getsize(target) / 1024, 1)
        except Exception as exc:  # noqa: BLE001
            report["export_error"] = str(exc)

    with open(OUT, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2, ensure_ascii=False)
    unreal.log("[probe_export] {}".format(json.dumps({k: report[k] for k in ("exported", "export_error", "size_kb") if k in report})))


try:
    main()
except Exception as exc:  # noqa: BLE001
    unreal.log("[probe_export] EXCEPTION {}".format(exc))