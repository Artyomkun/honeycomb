"""Verifies AHoneycombGround, the runtime honeycomb floor.

Two layers of checks:

1. The flat-top axial hex maths is static and world independent, so it can be
   exercised without a level: cell counts per ring count, a CellToLocal ->
   LocalToCell round trip over every cell, and neighbour symmetry.
2. The actor is spawned into the editor world when that is possible, so the
   generated sections, bounds, collision, relief and the highlight tint are
   checked on the real component instead of on the maths alone.

UE Python drops the bool return of a function with a struct out param and hands
back the out struct instead, or None when the function reported failure. The
checks below rely on that: None means "not a cell of this grid".

Run:
  UnrealEditor-Cmd.exe Honeycomb.uproject -run=pythonscript
      -script=Content/Python/verify_hex_ground.py -unattended -nullrhi
"""

import json
import math
import traceback

import unreal

CLASS_NAME = "HoneycombGround"
CLASS_PATH = "/Script/Honeycomb.HoneycombGround"
REPORT_PATH = r"C:\Project\Honeycomb\Tools\AvatarGen\ue_hex_ground_report.json"

# Class defaults of AHoneycombGround.
RINGS = 3
CELL_RADIUS_CM = 100.0
CELL_GAP_CM = 12.0
CELL_DEPTH_CM = 30.0
RELIEF_SCALE_CM = 14.0

EFFECTIVE_RADIUS_CM = CELL_RADIUS_CM - CELL_GAP_CM * 0.5
EXPECTED_CELLS = 1 + 3 * RINGS * (RINGS + 1)
# 26 verts and 24 tris per hex prism, see BuildCellGeometry.
EXPECTED_VERTS_PER_CELL = 26
EXPECTED_TRIS_PER_CELL = 24

TOL_CM = 0.5
ORIGIN = unreal.IntPoint(0, 0)


def _log(msg):
    # unreal.log is filtered out of the commandlet log; warning is not.
    unreal.log_warning("[verify_hex_ground] %s" % msg)


def _fail(checks, name, detail):
    checks.append({"check": name, "passed": False, "detail": detail})
    _log("FAIL %s: %s" % (name, detail))


def _pass(checks, name, detail=""):
    checks.append({"check": name, "passed": True, "detail": detail})


def _key(cell):
    if cell is None:
        return None
    return (int(cell.x), int(cell.y))


def _show(cell):
    return "None" if cell is None else "[%d, %d]" % (cell.x, cell.y)


def _vec(v):
    # unreal.Vector is not iterable.
    return [round(v.x, 2), round(v.y, 2), round(v.z, 2)]


def _colour(c):
    return [round(c.r, 3), round(c.g, 3), round(c.b, 3), round(c.a, 3)]


def _check_math(checks, cls):
    counts = {}
    for rings in (0, 1, 2, 3, 5):
        expected = cls.compute_cell_count_for_rings(rings)
        actual = len(cls.enumerate_cells(rings))
        counts[rings] = actual
        if actual != expected or actual != 1 + 3 * rings * (rings + 1):
            _fail(checks, "cell_count_rings_%d" % rings,
                  "enumerate=%d formula=%d" % (actual, expected))
    if all(c["passed"] for c in checks):
        _pass(checks, "cell_counts", json.dumps(counts))

    axial_cells = cls.enumerate_cells(RINGS)
    centers = [cls.cell_to_local(cell, EFFECTIVE_RADIUS_CM) for cell in axial_cells]

    # Every cell centre must resolve back to the same cell, and no two centres
    # may land on the same cell.
    bad_roundtrip = []
    resolved = {}
    for axial, center in zip(axial_cells, centers):
        found = cls.local_to_cell(center, EFFECTIVE_RADIUS_CM, RINGS)
        if _key(found) != _key(axial):
            bad_roundtrip.append("%s -> %s" % (_show(axial), _show(found)))
        resolved.setdefault(_key(found), []).append(axial)
    if bad_roundtrip:
        _fail(checks, "cell_roundtrip", "%d of %d wrong: %s"
              % (len(bad_roundtrip), len(centers), "; ".join(bad_roundtrip[:5])))
    else:
        _pass(checks, "cell_roundtrip", "%d centres" % len(centers))

    duplicates = {"%s" % (k,): [ _show(c) for c in v] for k, v in resolved.items() if len(v) > 1}
    if duplicates:
        _fail(checks, "cell_unique", json.dumps(duplicates))
    else:
        _pass(checks, "cell_unique", "%d distinct cells" % len(resolved))

    # A point far outside the grid must not resolve to any cell of it.
    outside = cls.local_to_cell(
        unreal.Vector(100000.0, 100000.0, 0.0), EFFECTIVE_RADIUS_CM, RINGS)
    if outside is not None:
        _fail(checks, "cell_outside_rejected", "resolved to %s" % _show(outside))
    else:
        _pass(checks, "cell_outside_rejected", "")

    # A point just past the outer ring must also be rejected, otherwise the
    # grid would grow by a cell in every direction at the boundary.
    outermost = cls.cell_to_local(
        unreal.IntPoint(RINGS, -RINGS), EFFECTIVE_RADIUS_CM)
    edge = cls.local_to_cell(
        unreal.Vector(outermost.x * 2.0, outermost.y * 2.0, 0.0),
        EFFECTIVE_RADIUS_CM, RINGS)
    if edge is not None:
        _fail(checks, "cell_outside_ring", "resolved to %s" % _show(edge))
    else:
        _pass(checks, "cell_outside_ring", "")

    # Neighbours: six of them, each exactly one step away, symmetric.
    asymmetric = []
    for cell in axial_cells:
        neighbours = [_key(n) for n in cls.cell_neighbors(cell)]
        if len(neighbours) != 6 or len(set(neighbours)) != 6:
            asymmetric.append("count %s" % neighbours)
            continue
        for neighbour in cls.cell_neighbors(cell):
            if cls.cell_distance(cell, neighbour) != 1:
                asymmetric.append("distance %s->%s" % (_show(cell), _show(neighbour)))
            back = [_key(m) for m in cls.cell_neighbors(neighbour)]
            if _key(cell) not in back:
                asymmetric.append("asymmetric %s<->%s" % (_show(cell), _show(neighbour)))
        if len(asymmetric) > 5:
            break
    if asymmetric:
        _fail(checks, "cell_neighbors", "; ".join(asymmetric[:5]))
    else:
        _pass(checks, "cell_neighbors", "%d cells" % len(axial_cells))

    # Neighbour centres must sit one cell pitch apart on the ground plane.
    expected_pitch = EFFECTIVE_RADIUS_CM * math.sqrt(3.0)
    pitch_errors = []
    for cell in axial_cells:
        here = cls.cell_to_local(cell, EFFECTIVE_RADIUS_CM)
        for neighbour in cls.cell_neighbors(cell):
            there = cls.cell_to_local(neighbour, EFFECTIVE_RADIUS_CM)
            actual = math.hypot(there.x - here.x, there.y - here.y)
            if abs(actual - expected_pitch) > 0.01:
                pitch_errors.append("%s->%s %.3f != %.3f"
                                    % (_show(cell), _show(neighbour), actual, expected_pitch))
        if len(pitch_errors) > 5:
            break
    if pitch_errors:
        _fail(checks, "cell_pitch", "; ".join(pitch_errors[:5]))
    else:
        _pass(checks, "cell_pitch", "%.3f cm" % expected_pitch)

    # Flat-top layout: 1.5 * R horizontal spacing, sqrt(3) * R vertical spacing.
    origin = cls.cell_to_local(ORIGIN, EFFECTIVE_RADIUS_CM)
    east = cls.cell_to_local(unreal.IntPoint(1, 0), EFFECTIVE_RADIUS_CM)
    north = cls.cell_to_local(unreal.IntPoint(0, 1), EFFECTIVE_RADIUS_CM)
    spacing = {
        "x": round(east.x - origin.x, 4),
        "y_north": round(north.y - origin.y, 4),
        "expected_x": round(EFFECTIVE_RADIUS_CM * 1.5, 4),
        "expected_y": round(EFFECTIVE_RADIUS_CM * math.sqrt(3.0), 4),
    }
    if (abs(spacing["x"] - spacing["expected_x"]) > 0.01
            or abs(spacing["y_north"] - spacing["expected_y"]) > 0.01):
        _fail(checks, "flat_top_spacing", json.dumps(spacing))
    else:
        _pass(checks, "flat_top_spacing", json.dumps(spacing))

    # Relief noise: deterministic, in range, and actually varying.
    samples = [cls.value_noise2d(x * 0.7, y * 0.7) for x in range(-6, 7) for y in range(-6, 7)]
    if min(samples) < 0.0 or max(samples) > 1.0:
        _fail(checks, "noise_range", "min=%.4f max=%.4f" % (min(samples), max(samples)))
    elif max(samples) - min(samples) < 0.2:
        _fail(checks, "noise_range", "noise is flat: %.4f" % (max(samples) - min(samples)))
    else:
        _pass(checks, "noise_range", "min=%.4f max=%.4f" % (min(samples), max(samples)))

    repeat = [cls.value_noise2d(x * 0.7, y * 0.7) for x in range(-6, 7) for y in range(-6, 7)]
    if repeat != samples:
        _fail(checks, "noise_deterministic", "value_noise2d is not stable")
    else:
        _pass(checks, "noise_deterministic", "%d samples" % len(samples))


def _check_spawned(checks, cls, ground):
    cells = ground.get_cell_count()
    if cells != EXPECTED_CELLS:
        _fail(checks, "spawn_cell_count", "%d != %d" % (cells, EXPECTED_CELLS))
    else:
        _pass(checks, "spawn_cell_count", str(cells))

    mesh = ground.get_ground_mesh()
    sections = mesh.get_num_sections()
    if sections != EXPECTED_CELLS:
        _fail(checks, "spawn_sections", "%d != %d" % (sections, EXPECTED_CELLS))
    else:
        _pass(checks, "spawn_sections", str(sections))

    verts = ground.get_grid_vertex_count()
    expected_verts = EXPECTED_CELLS * EXPECTED_VERTS_PER_CELL
    if verts != expected_verts:
        _fail(checks, "spawn_vertex_count", "%d != %d" % (verts, expected_verts))
    else:
        _pass(checks, "spawn_vertex_count", str(verts))

    tris = ground.get_grid_triangle_count()
    expected_tris = EXPECTED_CELLS * EXPECTED_TRIS_PER_CELL
    if tris != expected_tris:
        _fail(checks, "spawn_triangle_count", "%d != %d" % (tris, expected_tris))
    else:
        _pass(checks, "spawn_triangle_count", str(tris))

    origin, box_extent = ground.get_actor_bounds(False, True)
    # Flat-top: X spans 2 * (Rings * 1.5 * R + R), Y spans 2 * R * sqrt(3) * (Rings + 0.5)
    # because the outermost cells sit at r = +/- Rings and a hexagon reaches
    # R * sin(60) = R * sqrt(3) / 2 past its centre.
    expected_x = 2.0 * (RINGS * 1.5 * EFFECTIVE_RADIUS_CM + EFFECTIVE_RADIUS_CM)
    expected_y = 2.0 * EFFECTIVE_RADIUS_CM * math.sqrt(3.0) * (RINGS + 0.5)
    measured = {
        "origin": _vec(origin),
        "extent_x": round(box_extent.x * 2.0, 2),
        "extent_y": round(box_extent.y * 2.0, 2),
        "extent_z": round(box_extent.z * 2.0, 2),
    }
    if (abs(measured["extent_x"] - expected_x) > TOL_CM * 4
            or abs(measured["extent_y"] - expected_y) > TOL_CM * 4):
        _fail(checks, "spawn_bounds", "%s vs expected %.1f x %.1f"
              % (json.dumps(measured), expected_x, expected_y))
    else:
        _pass(checks, "spawn_bounds", json.dumps(measured))

    # Relief must actually change the floor, and stay inside ReliefScale.
    heights = [ground.get_cell_height(cell) for cell in cls.enumerate_cells(RINGS)]
    spread = max(heights) - min(heights)
    if spread <= 0.5:
        _fail(checks, "spawn_relief", "flat: spread=%.3f" % spread)
    elif max(abs(h) for h in heights) > RELIEF_SCALE_CM + 0.01:
        _fail(checks, "spawn_relief", "%.3f exceeds scale %.1f"
              % (max(abs(h) for h in heights), RELIEF_SCALE_CM))
    else:
        _pass(checks, "spawn_relief", "min=%.2f max=%.2f" % (min(heights), max(heights)))

    if mesh.get_collision_enabled() == unreal.CollisionEnabled.QUERY_AND_PHYSICS:
        _pass(checks, "spawn_collision", str(mesh.get_collision_enabled()))
    else:
        _fail(checks, "spawn_collision", str(mesh.get_collision_enabled()))

    # Highlighting is a material tint, so geometry must not move and the colour
    # has to return to its base value after ClearHighlight.
    highlight_color = ground.get_editor_property("highlight_color")
    target = unreal.IntPoint(1, 0)
    base = ground.get_cell_color(target)
    before = _vec(ground.get_cell_center(target))
    ground.set_highlighted_cell(target)
    lit = ground.get_cell_color(target)
    found = ground.get_highlighted_cell()
    if _key(found) != _key(target):
        _fail(checks, "highlight_set", "cell is %s" % _show(found))
    elif lit != highlight_color:
        _fail(checks, "highlight_set", "colour stayed %s" % _colour(lit))
    elif _vec(ground.get_cell_center(target)) != before:
        _fail(checks, "highlight_set", "cell centre moved")
    else:
        _pass(checks, "highlight_set", "%s -> %s" % (_colour(base), _colour(lit)))

    ground.clear_highlight()
    if ground.get_highlighted_cell() is not None:
        _fail(checks, "highlight_clear", "still %s" % _show(ground.get_highlighted_cell()))
    elif ground.get_cell_color(target) != base:
        _fail(checks, "highlight_clear", "colour stayed %s" % ground.get_cell_color(target))
    else:
        _pass(checks, "highlight_clear", "")

    # A cell outside the grid must not be highlightable.
    outside_cell = unreal.IntPoint(RINGS + 1, 0)
    ground.set_highlighted_cell(outside_cell)
    if ground.get_highlighted_cell() is not None:
        _fail(checks, "highlight_outside", "highlighted %s" % _show(outside_cell))
    else:
        _pass(checks, "highlight_outside", "")

    # WorldToCell must resolve a world position back onto its own cell.
    probe = unreal.IntPoint(2, -1)
    resolved = ground.world_to_cell(ground.get_cell_center(probe))
    if _key(resolved) != _key(probe):
        _fail(checks, "world_to_cell", "%s -> %s" % (_show(probe), _show(resolved)))
    else:
        _pass(checks, "world_to_cell", _show(resolved))

    far = ground.world_to_cell(unreal.Vector(100000.0, 100000.0, 0.0))
    if far is not None:
        _fail(checks, "world_to_cell_outside", "resolved to %s" % _show(far))
    else:
        _pass(checks, "world_to_cell_outside", "")

    if not ground.contains_cell(unreal.IntPoint(RINGS, 0)) or ground.contains_cell(outside_cell):
        _fail(checks, "contains_cell", "ring bounds wrong")
    else:
        _pass(checks, "contains_cell", "ring %d" % RINGS)

    # On the spawned actor every neighbour pair must be one pitch apart.
    spacing = []
    for cell in cls.enumerate_cells(RINGS):
        here = ground.get_cell_center(cell)
        for neighbour in cls.cell_neighbors(cell):
            there = ground.get_cell_center(neighbour)
            spacing.append(math.hypot(there.x - here.x, there.y - here.y))
    if not spacing:
        _fail(checks, "spawn_cell_pitch", "no neighbour pairs")
    else:
        lo, hi = min(spacing), max(spacing)
        if abs(lo - hi) > 0.01:
            _fail(checks, "spawn_cell_pitch", "pitch varies %.3f .. %.3f" % (lo, hi))
        else:
            _pass(checks, "spawn_cell_pitch", "%.3f cm over %d pairs" % (lo, len(spacing)))

    return {
        "cells": cells,
        "sections": sections,
        "vertices": verts,
        "triangles": tris,
        "bounds": measured,
        "relief": {"min_cm": round(min(heights), 2), "max_cm": round(max(heights), 2)},
        "collision": str(mesh.get_collision_enabled()),
    }


def _check_defaults(checks, cdo):
    defaults = {
        "rings": cdo.get_editor_property("rings"),
        "cell_radius": cdo.get_editor_property("cell_radius"),
        "cell_gap": cdo.get_editor_property("cell_gap"),
        "cell_depth": cdo.get_editor_property("cell_depth"),
        "relief_scale": cdo.get_editor_property("relief_scale"),
        # UE Python strips the bool "b" prefix, so bCreateCollision is create_collision.
        "create_collision": cdo.get_editor_property("create_collision"),
    }
    expected = {
        "rings": RINGS,
        "cell_radius": CELL_RADIUS_CM,
        "cell_gap": CELL_GAP_CM,
        "cell_depth": CELL_DEPTH_CM,
        "relief_scale": RELIEF_SCALE_CM,
    }
    for key, want in expected.items():
        if abs(float(defaults[key]) - float(want)) > 0.01:
            _fail(checks, "default_%s" % key, "%s != %s" % (defaults[key], want))
    if defaults["create_collision"] is not True:
        _fail(checks, "default_collision", str(defaults["create_collision"]))
    return defaults


def _run():
    checks = []
    report = {"class": CLASS_PATH, "spawned": False, "checks": checks}

    # UFUNCTIONs, including the static hex maths, live on the generated Python
    # type; the unreal.Class from load_class only exposes the CDO.
    cls = getattr(unreal, CLASS_NAME, None)
    if cls is None:
        report["passed"] = False
        report["error"] = "class not exposed to Python: %s" % CLASS_PATH
        return report

    report["class_defaults"] = _check_defaults(checks, unreal.get_default_object(cls))
    _check_math(checks, cls)

    # Spawning needs an editor world; report rather than fail when there is none.
    try:
        subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        ground = subsystem.spawn_actor_from_class(cls, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator())
    except Exception:  # noqa: BLE001
        ground = None
        report["spawn_error"] = traceback.format_exc().strip().splitlines()[-1]

    if ground is None:
        report["spawned"] = False
        _log("WARN no editor world, mesh checks skipped")
    else:
        report["spawned"] = True
        try:
            report["mesh"] = _check_spawned(checks, cls, ground)
        finally:
            subsystem.destroy_actor(ground)

    report["passed"] = all(c["passed"] for c in checks)
    report["checks_run"] = len(checks)
    report["failures"] = [c for c in checks if not c["passed"]]
    return report


def run():
    report = {}
    try:
        report = _run()
    except Exception:  # noqa: BLE001
        report = {"passed": False, "error": traceback.format_exc()}
        unreal.log_error("[verify_hex_ground] EXCEPTION\n%s" % traceback.format_exc())

    try:
        with open(REPORT_PATH, "w", encoding="utf-8") as handle:
            json.dump(report, handle, indent=2, ensure_ascii=False)
        _log("report written to %s" % REPORT_PATH)
    except Exception:  # noqa: BLE001
        unreal.log_error("[verify_hex_ground] report write failed:\n%s" % traceback.format_exc())

    _log("%s: %d checks, %d failures"
         % ("PASS" if report.get("passed") else "FAIL",
            report.get("checks_run", 0), len(report.get("failures", []))))
    for failure in report.get("failures", []):
        unreal.log_error("[verify_hex_ground] FAIL %s: %s" % (failure["check"], failure["detail"]))


run()