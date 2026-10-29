"""
Validate the project after removing weapon assets: load the startup map and
report any errors, missing assets or broken references.

Run headless (editor must be closed):
  UnrealEditor-Cmd.exe Honeycomb.uproject -run=pythonscript -script=<this>
"""
import unreal

MAP = "/Game/XRFramework/Levels/L_XRTemplate"
REPORT = r"C:\Users\selem\AppData\Local\Temp\validate_after_cleanup.txt"

lines = []


def _log(msg):
    lines.append(str(msg))
    unreal.log("[validate] {}".format(msg))


def _main():
    _log("start")
    ok = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
    _log("map load result: {}".format(ok))

    world = unreal.EditorLevelLibrary.get_editor_world()
    if world is not None:
        actors = unreal.EditorLevelLibrary.get_all_level_actors()
        _log("level: {} actors={}".format(world.get_name(), len(actors)))
        weapon_like = [
            a.get_name() for a in actors
            if any(k in a.get_name().lower() for k in ("pistol", "rifle", "weapon", "projectile", "shoot"))
        ]
        _log("weapon actors in level: {}".format(weapon_like if weapon_like else "none"))
    else:
        _log("WARN: no world")

    # Report broken references across loaded content.
    try:
        subsystems = unreal.get_editor_subsystem(unreal.AssetRegistryHelpers) if False else None
    except Exception:  # noqa: BLE001
        pass

    try:
        results = unreal.EditorValidator.validate_loaded_assets()
        bad = [r for r in results if r.is_valid() is False]
        _log("validator: {} assets checked, {} invalid".format(len(results), len(bad)))
        for item in bad[:10]:
            _log("  INVALID: {}".format(item.asset_name))
    except Exception as exc:  # noqa: BLE001
        _log("validator unavailable: {}".format(exc))

    _log("done")


try:
    _main()
except Exception as exc:  # noqa: BLE001
    _log("EXCEPTION: {}".format(exc))
finally:
    with open(REPORT, "w", encoding="utf-8") as handle:
        handle.write("\n".join(lines))
