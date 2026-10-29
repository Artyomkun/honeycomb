"""Builds the finale MoonLevel, the map AGameManager::CheckMoonAccess travels to.

Creates /Game/Maps/MoonLevel with a PlayerStart (so the flat pawn spawns), an
AMoonScene actor (which sets up its own environment, curator and final
challenge in BeginPlay) and basic lighting.

Run:
  UnrealEditor-Cmd.exe Honeycomb.uproject -run=pythonscript
      -script=Content/Python/make_moon_map.py -unattended -nullrhi
"""

import json
import traceback

import unreal

MAP_PATH = "/Game/Maps/MoonLevel"
REPORT_PATH = r"C:\Project\Honeycomb\Tools\AvatarGen\ue_moon_map_report.json"


def _log(msg):
    unreal.log_warning("[make_moon_map] %s" % msg)


def _vec(v):
    return [round(v.x, 2), round(v.y, 2), round(v.z, 2)]


def build():
    report = {"map": MAP_PATH, "actors": [], "ok": False}

    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

    if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        unreal.EditorAssetLibrary.delete_asset(MAP_PATH)

    les.new_level(MAP_PATH)
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

    spawn(unreal.PlayerStart, unreal.Vector(0.0, 0.0, 200.0), label="MoonPlayerStart")

    moon_class = getattr(unreal, "MoonScene")
    spawn(moon_class, unreal.Vector(0.0, 0.0, 0.0), label="MoonScene")

    sun = spawn(unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 1200.0), label="Sun")
    sun.set_actor_rotation(unreal.Rotator(0.0, -55.0, 20.0), False)
    comp = sun.get_editor_property("directional_light_component")
    if comp:
        comp.set_mobility(unreal.ComponentMobility.MOVABLE)
        comp.set_intensity(3.0)

    skylight = spawn(unreal.SkyLight, unreal.Vector(0.0, 0.0, 600.0), label="SkyLight")
    sky_comp = skylight.get_editor_property("light_component")
    if sky_comp:
        sky_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
        sky_comp.set_editor_property("intensity", 0.6)

    spawn(unreal.SkyAtmosphere, unreal.Vector(0.0, 0.0, 0.0), label="SkyAtmosphere")

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

    _log("ok=%s actors=%d" % (report.get("ok"), len(report.get("actors", []))))


main()
