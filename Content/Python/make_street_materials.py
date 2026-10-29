"""Creates the two simple materials used by the runtime street.

/Game/Materials/M_StreetBuilding - light concrete grey, rough.
/Game/Materials/M_StreetRoad     - dark asphalt grey, rough.

Run:
  UnrealEditor-Cmd.exe Honeycomb.uproject -run=pythonscript
      -script=Content/Python/make_street_materials.py -unattended -nullrhi
"""

import json
import traceback

import unreal

REPORT_PATH = r"C:\Project\Honeycomb\Tools\AvatarGen\ue_street_materials_report.json"
MATERIALS = [
    ("M_StreetBuilding", unreal.LinearColor(0.58, 0.56, 0.52, 1.0), 0.85),
    ("M_StreetRoad", unreal.LinearColor(0.09, 0.09, 0.10, 1.0), 0.95),
]


def _log(msg):
    unreal.log_warning("[make_street_materials] %s" % msg)


def build_one(name, color, roughness):
    path = "/Game/Materials/%s" % name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)

    tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = tools.create_asset(name, "/Game/Materials", unreal.Material, unreal.MaterialFactoryNew())
    if material is None:
        raise RuntimeError("failed to create %s" % path)

    mel = unreal.MaterialEditingLibrary

    base = mel.create_material_expression(material, unreal.MaterialExpressionConstant3Vector, -400, 0)
    base.set_editor_property("constant", color)
    mel.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)

    rough = mel.create_material_expression(material, unreal.MaterialExpressionConstant, -400, 180)
    rough.set_editor_property("r", roughness)
    mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(path)
    return path


def main():
    try:
        created = []
        for name, color, roughness in MATERIALS:
            created.append(build_one(name, color, roughness))
        report = {"ok": True, "materials": created}
    except Exception:  # noqa: BLE001
        report = {"ok": False, "error": traceback.format_exc()}
        _log("EXCEPTION\n%s" % report["error"])

    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    _log("ok=%s" % report.get("ok"))


main()
