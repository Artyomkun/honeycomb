"""Builds the flat-screen Honeycomb street level.

Creates /Game/Maps/L_HoneycombStreet with a PlayerStart, an AHoneycombGround
floor, basic sky lighting and an unbound post process volume, then points the
level's World Settings at AHoneycombGameMode so the flat pawn and controller are
used without touching the project-wide (VR) game mode.

Run:
  UnrealEditor-Cmd.exe Honeycomb.uproject -run=pythonscript
      -script=Content/Python/make_flat_map.py -unattended -nullrhi
"""

import json
import traceback

import unreal

MAP_PATH = "/Game/Maps/L_HoneycombStreet"
REPORT_PATH = r"C:\Project\Honeycomb\Tools\AvatarGen\ue_flat_map_report.json"


def _log(msg):
    unreal.log_warning("[make_flat_map] %s" % msg)


def _vec(v):
    return [round(v.x, 2), round(v.y, 2), round(v.z, 2)]


def build():
    report = {"map": MAP_PATH, "steps": [], "actors": [], "ok": False}

    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

    if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        unreal.EditorAssetLibrary.delete_asset(MAP_PATH)
        report["steps"].append("deleted_existing_map")

    les.new_level(MAP_PATH)
    report["steps"].append("created_level")

    world = ues.get_editor_world()
    if world is None:
        raise RuntimeError("no editor world after new_level")

    def spawn(cls, location, rotation=None, label=None):
        rot = rotation or unreal.Rotator(0.0, 0.0, 0.0)
        actor = eas.spawn_actor_from_class(cls, location, rot)
        if actor is None:
            raise RuntimeError("failed to spawn %s" % cls)
        if label:
            actor.set_actor_label(label)
        report["actors"].append(
            {"class": actor.get_class().get_name(), "label": actor.get_actor_label(), "loc": _vec(location)}
        )
        return actor

    spawn(unreal.PlayerStart, unreal.Vector(0.0, 0.0, 150.0), label="FlatPlayerStart")

    ground_class = getattr(unreal, "HoneycombGround")
    spawn(ground_class, unreal.Vector(0.0, 0.0, 0.0), label="HoneycombGround")

    sun = spawn(unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 1500.0), label="Sun")
    sun.set_actor_rotation(unreal.Rotator(0.0, -48.0, 35.0), False)
    sun_comp = sun.get_editor_property("directional_light_component")
    if sun_comp:
        sun_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
        sun_comp.set_intensity(5.0)
        sun_comp.set_editor_property("light_color", unreal.Color(255, 244, 214, 255))

    skylight = spawn(unreal.SkyLight, unreal.Vector(0.0, 0.0, 800.0), label="SkyLight")
    sky_comp = skylight.get_editor_property("light_component")
    if sky_comp:
        sky_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
        sky_comp.set_editor_property("intensity", 1.0)

    spawn(unreal.SkyAtmosphere, unreal.Vector(0.0, 0.0, 0.0), label="SkyAtmosphere")

    fog = spawn(unreal.ExponentialHeightFog, unreal.Vector(0.0, 0.0, 0.0), label="HeightFog")
    fog_comp = fog.get_editor_property("component")
    if fog_comp:
        fog_comp.set_editor_property("fog_density", 0.008)

    ppv = spawn(unreal.PostProcessVolume, unreal.Vector(0.0, 0.0, 0.0), label="PostProcess")
    ppv.set_editor_property("unbound", True)
    settings = ppv.get_editor_property("settings")
    settings.set_editor_property("override_auto_exposure_min_brightness", True)
    settings.set_editor_property("auto_exposure_min_brightness", 1.0)
    settings.set_editor_property("override_auto_exposure_max_brightness", True)
    settings.set_editor_property("auto_exposure_max_brightness", 1.0)
    ppv.set_editor_property("settings", settings)

    game_mode_class = getattr(unreal, "HoneycombGameMode")
    ws_set = False
    try:
        for actor in eas.get_all_level_actors():
            if isinstance(actor, unreal.WorldSettings):
                actor.set_editor_property("default_game_mode", game_mode_class)
                ws_set = True
                break
    except Exception as exc:  # noqa: BLE001
        report["steps"].append("worldsettings_error: %s" % exc)

    report["world_settings_game_mode"] = ws_set
    if not ws_set:
        report["steps"].append("world_settings_game_mode_not_set")

    saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP_PATH)
    report["saved"] = bool(saved)
    report["ok"] = bool(saved)
    return report


def main():
    try:
        report = build()
    except Exception:  # noqa: BLE001
        report = {"ok": False, "error": traceback.format_exc()}
        _log("EXCEPTION\n%s" % report["error"])

    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    _log("ok=%s actors=%d saved=%s" % (
        report.get("ok"), len(report.get("actors", [])), report.get("saved")))


main()
