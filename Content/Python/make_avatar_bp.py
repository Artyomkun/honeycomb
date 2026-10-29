"""
Build /Game/Avatars/BP_PlayerAvatar as a Blueprint child of AHoneycombAvatar.

The native class owns the avatar: a UPoseableMeshComponent root plus
UAvatarAutoFitComponent. A Blueprint child only has to carry the mesh asset and
the reference values the fit is derived from, which is what this script writes to
the class default object.

Nothing is added to the Simple Construction Script, because the components come
from the native constructor and appear in the child as inherited components.

Run headless (editor must be closed):
    "C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe" ^
        "C:\\Project\\Honeycomb\\Honeycomb.uproject" -run=pythonscript ^
        -script="C:\\Project\\Honeycomb\\Content\\Python\\make_avatar_bp.py" ^
        -unattended -nullrhi
"""
import json
import traceback

import unreal

MESH_PATH = "/Game/Avatars/SK_HoneycombAvatar"
BP_PATH = "/Game/Avatars/BP_PlayerAvatar"
BP_NAME = "BP_PlayerAvatar"
REPORT = r"C:\Project\Honeycomb\Tools\AvatarGen\ue_avatar_bp_report.json"

# Reference values of the generated body, see Tools/AvatarGen/metrics.json.
AUTHORED_HEIGHT_CM = 178.0
AUTHORED_ARM_SPAN_CM = 181.0

# UPROPERTY bools lose their b prefix in the Python bindings.
AUTOFIT_DEFAULTS = {
    "authored_height_cm": AUTHORED_HEIGHT_CM,
    "authored_arm_span_cm": AUTHORED_ARM_SPAN_CM,
    "per_bone_arm_fit": True,
    "auto_fit_arm_span": True,
    "min_arm_bone_scale": 0.70,
    "max_arm_bone_scale": 1.15,
    "min_arm_span_cm": 120.0,
    "max_arm_span_cm": 230.0,
    "fit_on_begin_play": True,
    "continuous_refit": True,
    "refit_interval_seconds": 2.0,
}

report = {"blueprint": BP_PATH, "steps": [], "errors": []}


def _step(msg):
    report["steps"].append(str(msg))
    unreal.log_warning("[make_avatar_bp] %s" % msg)


def _fail(msg):
    report["errors"].append(str(msg))
    unreal.log_error("[make_avatar_bp] %s" % msg)


def _create_blueprint():
    if unreal.EditorAssetLibrary.does_asset_exist(BP_PATH):
        _step("deleting previous %s" % BP_NAME)
        if not unreal.EditorAssetLibrary.delete_asset(BP_PATH):
            _fail("could not delete %s" % BP_PATH)
            return None

        # Deleting only marks the package, the loaded asset keeps its name, so
        # AssetTools would refuse to create a new one under the same path. This
        # script is meant to run against a freshly started editor; if the asset
        # is still around here it is held in memory and the run has to be repeated
        # without the editor holding it.
        if unreal.EditorAssetLibrary.does_asset_exist(BP_PATH):
            _fail(
                "%s still exists after delete; delete the .uasset on disk and rerun"
                % BP_PATH
            )
            return None

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.HoneycombAvatar)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    blueprint = asset_tools.create_asset(BP_NAME, "/Game/Avatars", unreal.Blueprint, factory)
    if blueprint is None:
        _fail("BlueprintFactory produced no asset")
        return None

    _step("created %s from AHoneycombAvatar" % BP_NAME)
    return blueprint


def _configure_defaults(blueprint, mesh):
    """Writes the mesh and the reference values onto the class default object.

    SCS components are not instantiated in commandlet mode, but the CDO's
    inherited native components are reachable, and that is where a Blueprint
    child stores its per-instance defaults anyway.
    """
    generated = blueprint.generated_class()
    if generated is None:
        _fail("generated class unavailable")
        return False

    cdo = unreal.get_default_object(generated)
    if cdo is None:
        _fail("class default object unavailable")
        return False

    body = cdo.get_editor_property("body_mesh")
    if body is None:
        _fail("inherited BodyMesh subobject missing on the CDO")
        return False

    fit = cdo.get_editor_property("auto_fit")
    if fit is None:
        _fail("inherited AutoFit subobject missing on the CDO")
        return False

    for name, value in AUTOFIT_DEFAULTS.items():
        try:
            fit.set_editor_property(name, value)
        except Exception as exc:  # noqa: BLE001
            _fail("AutoFit.%s = %r failed: %s" % (name, value, exc))

    cdo.set_editor_property("body_mesh_asset", mesh)
    _step("body mesh asset assigned on the CDO")
    _step("auto-fit defaults applied on the CDO")
    return True


def _verify(blueprint, mesh):
    """Confirms the asset really carries the mesh and the reference values."""
    generated = blueprint.generated_class()
    cdo = unreal.get_default_object(generated)
    fit = cdo.get_editor_property("auto_fit")

    # The CDO being a HoneycombAvatar proves the reparenting; UBlueprint's
    # ParentClass is not exposed to Python.
    report["cdo_is_honeycomb_avatar"] = isinstance(cdo, unreal.HoneycombAvatar)
    if not report["cdo_is_honeycomb_avatar"]:
        _fail("CDO is not an AHoneycombAvatar instance")

    stored_mesh = cdo.get_editor_property("body_mesh_asset")
    report["mesh_asset"] = stored_mesh.get_path_name() if stored_mesh else None
    if stored_mesh is None:
        _fail("BodyMeshAsset did not persist on the CDO")

    for name, value in AUTOFIT_DEFAULTS.items():
        stored = fit.get_editor_property(name)
        report.setdefault("auto_fit", {})[name] = stored
        if isinstance(value, float):
            mismatched = abs(float(stored) - value) > 1e-4
        else:
            mismatched = bool(stored) != value
        if mismatched:
            _fail("AutoFit.%s stored as %r, expected %r" % (name, stored, value))

    if stored_mesh is not None:
        report["mesh_vertices"] = stored_mesh.get_bounds().box_extent.z * 2.0
        report["mesh_matches_request"] = stored_mesh == mesh
        if stored_mesh != mesh:
            _fail("CDO holds %s, expected %s" % (stored_mesh.get_name(), mesh.get_name()))
        _step("mesh on the CDO bounds height %.3f cm" % report["mesh_vertices"])


def _save(blueprint):
    if not unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False):
        _fail("save failed for %s" % BP_PATH)
        return False

    _step("saved %s" % BP_PATH)
    return True


def run():
    mesh = unreal.load_asset(MESH_PATH)
    if mesh is None:
        _fail("mesh not found: %s" % MESH_PATH)
        return

    _step("mesh %s loaded" % mesh.get_name())

    blueprint = _create_blueprint()
    if blueprint is None:
        return

    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    _step("blueprint compiled")

    if _configure_defaults(blueprint, mesh):
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)

    _save(blueprint)
    _verify(blueprint, mesh)


try:
    run()
except Exception:  # noqa: BLE001
    _fail("EXCEPTION\n%s" % traceback.format_exc())
finally:
    report["passed"] = not report["errors"]
    try:
        with open(REPORT, "w", encoding="utf-8") as handle:
            json.dump(report, handle, indent=2)
        unreal.log_warning("[make_avatar_bp] report written to %s" % REPORT)
    except Exception:  # noqa: BLE001
        unreal.log_error("[make_avatar_bp] report write failed:\n%s" % traceback.format_exc())
