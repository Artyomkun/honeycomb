"""Verifies UAvatarAutoFitComponent's stature scale and per-bone arm fit in UE.

The stature fit is a uniform component scale, so it only has to reproduce the
proportions the generator would emit at that height. The arm fit is per bone, so
this measures the resulting chain length and compares it with the span that was
requested.

Two subjects are checked: the native AHoneycombAvatar, whose mesh the test
assigns itself, and /Game/Avatars/BP_PlayerAvatar, the Blueprint the game
spawns. The Blueprint is not given the mesh here on purpose: it has to carry it
from its own class default, so this also proves make_avatar_bp.py wired it.

Run:
  UnrealEditor-Cmd.exe Honeycomb.uproject -run=pythonscript
      -script=Content/Python/verify_arm_fit.py -unattended -nullrhi
"""

import json
import math
import os
import traceback

import unreal

MESH_PATH = "/Game/Avatars/SK_HoneycombAvatar"
BP_PATH = "/Game/Avatars/BP_PlayerAvatar"
REPORT_PATH = r"C:\Project\Honeycomb\Tools\AvatarGen\ue_arm_fit_report.json"

# The Blueprint is expected to carry these, straight from the generator.
AUTHORED_HEIGHT_CM = 178.0
AUTHORED_ARM_SPAN_CM = 181.0

BONES = {
    "shoulder_l": "upperarm_l",
    "shoulder_r": "upperarm_r",
    "elbow_l": "lowerarm_l",
    "elbow_r": "lowerarm_r",
    "wrist_l": "hand_l",
    "wrist_r": "hand_r",
    "head": "head",
    "foot_l": "foot_l",
    "foot_r": "foot_r",
}

# (name, stature cm, arm span cm)
CASES = [
    ("authored", 178.0, 181.0),
    ("short_narrow", 160.0, 163.0),
    ("tall_wide", 195.0, 202.0),
    ("stout_short_arms", 190.0, 172.0),
    ("petite", 152.0, 150.0),
]

TOL_HEIGHT_CM = 1.0
TOL_SCALE = 0.002
TOL_ARM_SCALE = 0.01
TOL_CHAIN_CM = 0.5


def _bone_space():
    spaces = getattr(unreal, "BoneSpaces", None)
    if spaces is None:
        raise RuntimeError("unreal.BoneSpaces is not exposed")
    # Component space: the uniform stature scale lives on the component
    # transform, so composing it into a bone query mixes two different spaces.
    return spaces.COMPONENT_SPACE


def _blueprint_class():
    generated = unreal.load_object(None, BP_PATH + ".BP_PlayerAvatar_C")
    if generated is None:
        blueprint = unreal.load_asset(BP_PATH)
        if blueprint is None:
            raise RuntimeError("blueprint not found: %s" % BP_PATH)
        generated = blueprint.generated_class()
    if generated is None:
        raise RuntimeError("blueprint generated class unavailable")
    return generated


def _spawn(subject, mesh):
    # Python cannot AddInstanceComponent/RegisterComponent, so the avatar has to
    # come from a native class, which owns its components as default subobjects.
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if subsystem is None:
        raise RuntimeError("EditorActorSubsystem unavailable")

    if subject == "native":
        actor = subsystem.spawn_actor_from_class(unreal.HoneycombAvatar, unreal.Vector(0.0, 0.0, 0.0))
    else:
        actor = subsystem.spawn_actor_from_class(_blueprint_class(), unreal.Vector(0.0, 0.0, 0.0))

    if actor is None:
        raise RuntimeError("failed to spawn %s" % subject)

    if subject == "native":
        actor.set_body_mesh_asset(mesh)
    return actor


def _dist(a, b):
    return math.sqrt((a.x - b.x) ** 2 + (a.y - b.y) ** 2 + (a.z - b.z) ** 2)


def _chain(pose, shoulder, elbow, wrist, space):
    """Length of the measurable part of the arm chain, glenohumeral socket to wrist.

    The hand bone is a leaf in this skeleton, so its length cannot be read back;
    the bone stretch scales it exactly like the two segments below the shoulder,
    so their ratio is what identifies the applied stretch.
    """
    a = pose.get_bone_location_by_name(shoulder, space)
    b = pose.get_bone_location_by_name(elbow, space)
    c = pose.get_bone_location_by_name(wrist, space)
    return _dist(a, b) + _dist(b, c), abs(a.x)


def _destroy(actor):
    try:
        if actor is not None:
            actor.destroy_actor()
    except Exception:
        unreal.log_warning("verify_arm_fit: destroy failed:\n%s" % traceback.format_exc())


def _run_subject(subject, mesh, ref_height, space):
    """Runs every case against one avatar class. Returns (results, failures)."""
    results = []
    failures = []

    reference_chain = None
    reference_pivot = None

    for name, height_cm, span_cm in CASES:
        actor = None
        try:
            actor = _spawn(subject, mesh)
            pose = actor.get_editor_property("body_mesh")
            fit = actor.get_editor_property("auto_fit")
            if pose is None or fit is None:
                raise RuntimeError("avatar subobjects not reachable")

            if subject != "native":
                # The Blueprint has to arrive fully wired from its own defaults.
                if pose.get_skinned_asset() != mesh:
                    raise RuntimeError("blueprint body mesh is %r" % pose.get_skinned_asset())
                stored_height = fit.get_editor_property("authored_height_cm")
                stored_span = fit.get_editor_property("authored_arm_span_cm")
                if abs(stored_height - AUTHORED_HEIGHT_CM) > 1e-3:
                    failures.append("%s: authored height on the BP is %.3f, expected %.1f" % (subject, stored_height, AUTHORED_HEIGHT_CM))
                if abs(stored_span - AUTHORED_ARM_SPAN_CM) > 1e-3:
                    failures.append("%s: authored span on the BP is %.3f, expected %.1f" % (subject, stored_span, AUTHORED_ARM_SPAN_CM))
            else:
                fit.set_editor_property("authored_height_cm", AUTHORED_HEIGHT_CM)
                fit.set_editor_property("authored_arm_span_cm", AUTHORED_ARM_SPAN_CM)

            fit.set_editor_property("default_player_height_cm", height_cm)
            fit.set_editor_property("per_bone_arm_fit", True)

            fit.fit_now()
            fit.apply_fit_immediate()
            fit.set_measured_arm_span(span_cm)
            fit.apply_fit_immediate()

            scale = pose.relative_scale3d.x
            expected_scale = height_cm / AUTHORED_HEIGHT_CM
            stature_measured = ref_height * scale

            chain_l, pivot_l = _chain(pose, BONES["shoulder_l"], BONES["elbow_l"], BONES["wrist_l"], space)
            chain_r, pivot_r = _chain(pose, BONES["shoulder_r"], BONES["elbow_r"], BONES["wrist_r"], space)

            arm_scale = fit.get_editor_property("current_arm_bone_scale")
            applied = fit.get_editor_property("arm_bone_fit_applied")

            if reference_chain is None:
                reference_chain = chain_l
                reference_pivot = pivot_l

            # span / 2 == pivot + chain, in component space before the uniform
            # scale, so the stretch the span requires is fully determined by the
            # pivot offset and the stature scale.
            wanted_chain = 0.5 * span_cm / scale - reference_pivot
            authored_chain = 0.5 * AUTHORED_ARM_SPAN_CM - reference_pivot
            expected_arm_scale = wanted_chain / authored_chain

            entry = {
                "subject": subject,
                "case": name,
                "requested_height_cm": height_cm,
                "requested_span_cm": span_cm,
                "component_scale": scale,
                "expected_scale": expected_scale,
                "measured_stature_cm": stature_measured,
                "arm_bone_scale": arm_scale,
                "expected_arm_bone_scale": expected_arm_scale,
                "arm_fit_applied": bool(applied),
                "chain_l_cm": chain_l,
                "chain_r_cm": chain_r,
                "pivot_x_cm": reference_pivot,
            }
            results.append(entry)

            tag = "%s/%s" % (subject, name)

            if abs(scale - expected_scale) > TOL_SCALE:
                failures.append("%s: component scale %.4f, expected %.4f" % (tag, scale, expected_scale))
            if abs(stature_measured - height_cm) > TOL_HEIGHT_CM:
                failures.append(
                    "%s: stature %.2f cm, requested %.2f cm" % (tag, stature_measured, height_cm)
                )
            if abs(arm_scale - expected_arm_scale) > TOL_ARM_SCALE:
                failures.append(
                    "%s: arm bone scale %.4f, expected %.4f" % (tag, arm_scale, expected_arm_scale)
                )
            if abs(chain_l - chain_r) > TOL_CHAIN_CM:
                failures.append(
                    "%s: arm chains differ, left %.2f right %.2f" % (tag, chain_l, chain_r)
                )
            expected_chain_measured = reference_chain * expected_arm_scale
            if abs(chain_l - expected_chain_measured) > TOL_CHAIN_CM:
                failures.append(
                    "%s: chain %.2f cm, expected %.2f cm" % (tag, chain_l, expected_chain_measured)
                )
            if not applied:
                failures.append("%s: arm bone fit not applied" % tag)

            unreal.log_warning(
                "verify_arm_fit %-22s scale=%.4f/%.4f stature=%.1f/%.1f arm=%.4f/%.4f chain=%.2f applied=%s"
                % (
                    tag,
                    scale,
                    expected_scale,
                    stature_measured,
                    height_cm,
                    arm_scale,
                    expected_arm_scale,
                    chain_l,
                    bool(applied),
                )
            )
        except Exception:
            failures.append("%s/%s: %s" % (subject, name, traceback.format_exc()))
            unreal.log_error("verify_arm_fit %s/%s failed:\n%s" % (subject, name, traceback.format_exc()))
        finally:
            _destroy(actor)

    return results, failures


def run():
    mesh = unreal.load_asset(MESH_PATH)
    if mesh is None:
        raise RuntimeError("mesh not found: %s" % MESH_PATH)

    space = _bone_space()

    # Reference mesh height, so the stature check does not depend on the bones.
    ref_height = mesh.get_bounds().box_extent.z * 2.0

    all_results = []
    all_failures = []
    for subject in ("native", "blueprint"):
        results, failures = _run_subject(subject, mesh, ref_height, space)
        all_results.extend(results)
        all_failures.extend(failures)

    report = {
        "mesh": MESH_PATH,
        "blueprint": BP_PATH,
        "subjects": ["native", "blueprint"],
        "reference_height_cm": ref_height,
        "authored_height_cm": AUTHORED_HEIGHT_CM,
        "authored_span_cm": AUTHORED_ARM_SPAN_CM,
        "cases": all_results,
        "failures": all_failures,
        "passed": not all_failures,
    }

    try:
        with open(REPORT_PATH, "w", encoding="utf-8") as handle:
            json.dump(report, handle, indent=2)
        unreal.log_warning("verify_arm_fit: report written to %s" % REPORT_PATH)
    except Exception:
        unreal.log_error("verify_arm_fit: report write failed:\n%s" % traceback.format_exc())

    if all_failures:
        for failure in all_failures:
            unreal.log_error("verify_arm_fit FAIL " + failure.splitlines()[0])
        raise RuntimeError("verify_arm_fit failed: %d problem(s) in %d cases" % (len(all_failures), len(all_results)))

    unreal.log_warning("verify_arm_fit: all %d cases passed" % len(all_results))


try:
    run()
except Exception:
    unreal.log_error("verify_arm_fit fatal:\n%s" % traceback.format_exc())
    raise
