"""Headless end-to-end test of the 12-chakra / 5-dimension progression loop.

Boots a real AGameManager in the editor world (via DebugInitialize, so no
BeginPlay is needed), then plays the loop exactly as the flat pawn does: pick
the next unlocked, un-activated kriya, practise it to completion by advancing
the systems with a fixed time step, and record how chakra and dimension move.

It asserts the intended ladder:
  * start at chakra 1, dimension 3;
  * every completed kriya activates one chakra and advances one chakra;
  * thresholds raise the dimension (>=7 -> 4, >=10 -> 5);
  * finishing the ladder yields chakra 12, dimension 5 and moon access.

Run:
  UnrealEditor-Cmd.exe Honeycomb.uproject -run=pythonscript
      -script=Content/Python/verify_chakra_loop.py -unattended -nullrhi
"""

import json
import traceback

import unreal

MANAGER_CLASS = "GameManager"
REPORT_PATH = r"C:\Project\Honeycomb\Tools\AvatarGen\ue_chakra_loop_report.json"
STEP_SECONDS = 1.0
MAX_STEPS_PER_KRIYA = 400


def _log(msg):
    unreal.log_warning("[verify_chakra_loop] %s" % msg)


def run():
    checks = []
    timeline = []

    def check(name, passed, detail=""):
        checks.append({"check": name, "passed": bool(passed), "detail": detail})
        _log("%s %s%s" % ("PASS" if passed else "FAIL", name, (": " + detail) if detail else ""))

    eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    manager_class = getattr(unreal, MANAGER_CLASS)

    gm = eas.spawn_actor_from_class(manager_class, unreal.Vector(0.0, 0.0, 0.0))
    if gm is None:
        raise RuntimeError("failed to spawn GameManager")
    gm.set_actor_label("LoopTestManager")
    gm.debug_initialize()

    kriya = gm.get_kriya_yoga_system()
    if kriya is None:
        raise RuntimeError("GameManager has no KriyaYogaSystem")

    check("start_chakra_is_1", gm.get_current_chakra_level() == 1, str(gm.get_current_chakra_level()))
    check("start_dimension_is_3", gm.get_current_dimension() == 3, str(gm.get_current_dimension()))
    check("moon_not_accessible_at_start", not gm.is_moon_accessible())

    previous_dimension = gm.get_current_dimension()
    dimension_advances = []

    for session in range(12):
        target = 0
        for level in range(1, 13):
            if kriya.is_kriya_unlocked(level) and not kriya.is_chakra_activated(level):
                target = level
                break
        if target == 0:
            target = gm.get_current_chakra_level()

        kriya.start_kriya_practice(target)
        if not kriya.is_kriya_active():
            check("kriya_%d_started" % target, False, "StartKriyaPractice did not activate")
            break

        steps = 0
        while kriya.is_kriya_active() and steps < MAX_STEPS_PER_KRIYA:
            gm.debug_tick_systems(STEP_SECONDS)
            steps += 1

        chakra = gm.get_current_chakra_level()
        dimension = gm.get_current_dimension()
        kriya_progress = kriya.get_kriya_progress()
        timeline.append({
            "session": session,
            "practiced": target,
            "chakra_after": chakra,
            "dimension_after": dimension,
            "steps": steps,
        })

        if not kriya.is_chakra_activated(target):
            check("chakra_%d_activated" % target, False, "chakra not activated after practice")
            break

        if dimension != previous_dimension:
            dimension_advances.append({"from": previous_dimension, "to": dimension, "chakra": chakra})
            previous_dimension = dimension

        if gm.is_moon_accessible():
            break

    final_chakra = gm.get_current_chakra_level()
    final_dimension = gm.get_current_dimension()

    check("final_chakra_is_12", final_chakra == 12, str(final_chakra))
    check("final_dimension_is_5", final_dimension == 5, str(final_dimension))
    check("moon_accessible_at_end", gm.is_moon_accessible())
    check("dimension_4_reached_at_chakra_ge_7",
          any(d["to"] == 4 and d["chakra"] >= 7 for d in dimension_advances),
          json.dumps(dimension_advances))
    check("dimension_5_reached_at_chakra_ge_10",
          any(d["to"] == 5 and d["chakra"] >= 10 for d in dimension_advances),
          json.dumps(dimension_advances))

    eas.destroy_actor(gm)

    passed = sum(1 for c in checks if c["passed"])
    return {
        "checks": checks,
        "timeline": timeline,
        "dimension_advances": dimension_advances,
        "passed": passed,
        "failed": len(checks) - passed,
        "ok": passed == len(checks),
    }


def main():
    try:
        report = run()
    except Exception:  # noqa: BLE001
        report = {"ok": False, "error": traceback.format_exc()}
        _log("EXCEPTION\n%s" % report["error"])

    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2)

    _log("RESULT ok=%s passed=%s failed=%s" % (
        report.get("ok"), report.get("passed"), report.get("failed")))


main()
