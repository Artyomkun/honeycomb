"""
Place the imported Blender avatar into the XR template level.

This answers "will the Blender avatar actually show up in the engine?":
the FBX becomes a real SkeletalMesh actor placed in a real level, and the
level is saved as /Game/Avatars/L_AVATAR_TEST so it can be opened.

Run (editor must be closed):
  "C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe" ^
      "C:\\Project\\Honeycomb\\Honeycomb.uproject" -run=pythonscript ^
      -script="C:\\Project\\Honeycomb\\Content\\Python\\place_avatar.py"

Note: a high-res screenshot is only taken when a viewport exists. Under
UnrealEditor-Cmd there is no RHI viewport and requesting one crashes the
process, so commandlet runs save the level and stop there.
"""
import unreal

MESH = "/Game/Avatars/SK_ParametricMannequin.SK_ParametricMannequin"
SOURCE_LEVEL = "/Game/XRFramework/Levels/L_XRTemplate"
TARGET_LEVEL = "/Game/Avatars/L_AVATAR_TEST"
OUT_PNG = r"C:\Users\selem\Pictures\unreal_avatar_render.png"
REPORT = r"C:\Users\selem\AppData\Local\Temp\place_avatar_report.txt"

lines = []


def _is_commandlet():
    """True when running under UnrealEditor-Cmd (no viewport, rendering unsafe).

    IsRunningCommandlet() is a plain C++ helper and is not exposed to
    Python, so probe a few shapes and default to the safe answer: when in
    doubt, skip rendering rather than crash the process.
    """
    for holder, name in (
        (unreal.SystemLibrary, "is_running_commandlet"),
        (unreal.SystemLibrary, "is_commandlet"),
        (unreal.EditorLevelLibrary, "is_playing_in_editor"),
    ):
        fn = getattr(holder, name, None)
        if callable(fn):
            try:
                return bool(fn())
            except Exception:  # noqa: BLE001
                pass
    return True


def _log(msg):
    lines.append(str(msg))
    unreal.log("[place_avatar] {}".format(msg))


def _main():
    _log("start")
    commandlet = _is_commandlet()
    if commandlet:
        _log("running as commandlet: no viewport, rendering skipped")

    unreal.EditorLoadingAndSavingUtils.load_map(SOURCE_LEVEL)
    _log("loaded {}".format(SOURCE_LEVEL))

    world = unreal.EditorLevelLibrary.get_editor_world()
    _log("world = {}".format(world.get_name()))

    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if subsystem is None:
        _log("no EditorActorSubsystem")
        return

    # Remove previous test actor so re-runs are idempotent.
    for actor in subsystem.get_all_level_actors():
        if actor.get_name().startswith("AvatarTest"):
            subsystem.destroy_actor(actor)

    loc = unreal.Vector(-300.0, 0.0, 0.0)
    rot = unreal.Rotator(0.0, 0.0, 0.0)
    avatar = subsystem.spawn_actor_from_class(unreal.SkeletalMeshActor, loc, rot)
    if avatar is None:
        _log("failed to spawn SkeletalMeshActor")
        return

    avatar.set_actor_label("AvatarTest_Mannnequin")
    avatar.set_actor_scale3d(unreal.Vector(1.0, 1.0, 1.0))

    mesh = unreal.load_asset(MESH)
    if mesh is None:
        _log("mesh not found at {}".format(MESH))
    else:
        avatar.skeletal_mesh_component.set_skeletal_mesh_asset(mesh)
        _log("mesh assigned: {}".format(mesh.get_name()))
        try:
            count = avatar.skeletal_mesh_component.get_num_materials()
            _log("material slots: {}".format(count))
        except Exception as exc:  # noqa: BLE001
            _log("slot count unavailable: {}".format(exc))

    # Give the scene a light source if the template level has none.
    has_light = any(
        isinstance(a, unreal.DirectionalLight) for a in subsystem.get_all_level_actors()
    )
    if not has_light:
        light = subsystem.spawn_actor_from_class(
            unreal.DirectionalLight, unreal.Vector(0, 0, 400), unreal.Rotator(-45, 35, 0)
        )
        if light is not None:
            light.set_actor_label("AvatarTest_Sun")
            _log("added directional light")

    # save_map requires the world as its first positional argument.
    try:
        saved = unreal.EditorLoadingAndSavingUtils.save_map(world, TARGET_LEVEL)
        _log("saved level: {} ({})".format(TARGET_LEVEL, saved))
    except Exception as exc:  # noqa: BLE001
        _log("save_map failed: {}".format(exc))

    # High-res screenshots need a live RHI viewport. In commandlet mode
    # (UnrealEditor-Cmd) there is none and the call crashes the process
    # with an access violation that try/except cannot catch.
    if not commandlet:
        try:
            unreal.AutomationLibrary.take_high_res_screenshot(1280, 960, OUT_PNG)
            _log("screenshot -> {}".format(OUT_PNG))
        except Exception as exc:  # noqa: BLE001
            _log("screenshot failed: {}".format(exc))
    else:
        _log("skipped screenshot: commandlet has no viewport")
        _log("open the level in the editor and use editor_take_screenshot")

    with open(REPORT, "w", encoding="utf-8") as handle:
        handle.write("\n".join(lines))
    _log("done")


try:
    _main()
except Exception as exc:  # noqa: BLE001
    _log("EXCEPTION: {}".format(exc))
finally:
    with open(REPORT, "w", encoding="utf-8") as handle:
        handle.write("\n".join(lines))
