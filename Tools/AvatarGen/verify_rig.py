"""
Numeric verification harness for the Honeycomb avatar generator.

Runs under plain Python, no bpy: build_avatar.py needs Blender for the mesh
objects and the FBX export, but the anatomy, the rig and the skin weights are
pure data. This environment has no way to look at a render, so every guarantee
the avatar is supposed to make is checked here as a measurement.

    python verify_rig.py

Exit code is non-zero when anything fails, so it can gate a rebuild.
"""
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import geometry  # noqa: E402
import metrics as metrics_mod  # noqa: E402
import rig as rig_mod  # noqa: E402

# UE5 Manny naming. The dot-separated convention this generator used before
# ("upper_arm.L") is mangled into "upper_arm_L" by the UE FBX importer, and it
# never matched Manny anyway, so bones are authored with the real Manny names.
EXPECTED_BONES = {
    "root": None,
    "pelvis": "root",
    "spine_01": "pelvis",
    "spine_02": "spine_01",
    "spine_03": "spine_02",
    "neck_01": "spine_03",
    "head": "neck_01",
    "clavicle_l": "spine_03",
    "upperarm_l": "clavicle_l",
    "lowerarm_l": "upperarm_l",
    "hand_l": "lowerarm_l",
    "clavicle_r": "spine_03",
    "upperarm_r": "clavicle_r",
    "lowerarm_r": "upperarm_r",
    "hand_r": "lowerarm_r",
    "thigh_l": "pelvis",
    "calf_l": "thigh_l",
    "foot_l": "calf_l",
    "thigh_r": "pelvis",
    "calf_r": "thigh_r",
    "foot_r": "calf_r",
}

# Regions that must not move when an arm is raised. Anything in here means a
# weight leaked from the arm chain into the torso.
STATIC_REGIONS = ("torso", "head", "leg_L", "leg_R", "foot_L", "foot_R")

# Regions that must move, and roughly how far, for an 80 degree shoulder raise.
MOVING_REGIONS = ("arm_L",)

GARMENTS = ("tee", "trousers", "shoes")

POSE_ANGLE_DEG = 80.0
POSE_AXIS = (0.0, 1.0, 0.0)

WEIGHT_EPS = 1e-6
LENGTH_TOL = 0.6  # cm, bones may be slightly longer than the vertical drop


class Results(object):
    def __init__(self):
        self.failures = []
        self.count = 0

    def check(self, ok, label, detail=""):
        self.count += 1
        if not ok:
            self.failures.append(label + ((" -- " + detail) if detail else ""))
        return ok

    def near(self, value, expected, tol, label):
        return self.check(
            abs(value - expected) <= tol,
            label,
            "got {0:.3f}, expected {1:.3f} +/- {2}".format(value, expected, tol),
        )


def make_metrics(label, sex, height, span, weight):
    m = metrics_mod.Metrics()
    m.sex = sex
    m.height = height
    m.arm_span = span
    m.weight = weight
    m.provenance["label"] = label
    return m


PROFILES = [
    ("male_178", make_metrics("male_178", "male", 178.0, 181.0, 74.0)),
    ("female_160", make_metrics("female_160", "female", 160.0, 152.8, 58.0)),
    ("tall_195", make_metrics("tall_195", "male", 195.0, 199.0, 95.0)),
    ("short_150", make_metrics("short_150", "male", 150.0, 148.0, 55.0)),
    ("derived_span", make_metrics("derived_span", "female", 172.0, 0.0, 0.0)),
]


# --- geometry helpers ------------------------------------------------------


def bone_map(bones):
    return {name: (parent, head, tail) for (name, parent, head, tail) in bones}


def seg_length(head, tail):
    return math.sqrt(
        (tail[0] - head[0]) ** 2 + (tail[1] - head[1]) ** 2 + (tail[2] - head[2]) ** 2
    )


def bounds(verts):
    xs = [v[0] for v in verts]
    ys = [v[1] for v in verts]
    zs = [v[2] for v in verts]
    return {
        "min": (min(xs), min(ys), min(zs)),
        "max": (max(xs), max(ys), max(zs)),
    }


def descendants(bones, root_name):
    """The bone itself plus everything under it, in hierarchy order."""
    out = [root_name]
    index = 0
    while index < len(out):
        current = out[index]
        index += 1
        for (name, parent, _h, _t) in bones:
            if parent == current and name not in out:
                out.append(name)
    return out


def rotate_vector(v, axis, angle_rad):
    """Rotate a direction (no translation), i.e. the linear part of rotate_about."""
    ax, ay, az = axis
    cos_a = math.cos(angle_rad)
    sin_a = math.sin(angle_rad)
    dot = ax * v[0] + ay * v[1] + az * v[2]
    return (
        v[0] * cos_a + (ay * v[2] - az * v[1]) * sin_a + ax * dot * (1.0 - cos_a),
        v[1] * cos_a + (az * v[0] - ax * v[2]) * sin_a + ay * dot * (1.0 - cos_a),
        v[2] * cos_a + (ax * v[1] - ay * v[0]) * sin_a + az * dot * (1.0 - cos_a),
    )


def rotate_about(point, origin, axis, angle_rad):
    """Rodrigues rotation of a point around an axis through an origin."""
    r = rotate_vector([point[i] - origin[i] for i in range(3)], axis, angle_rad)
    return tuple(r[i] + origin[i] for i in range(3))


def skin(verts, weights, posed_names, origin, axis, angle):
    """Linear blend skinning for a rigid pose of one bone chain.

    Rotating a bone about a world axis gives the skinning matrix
    M_pose * M_rest^-1 = R(origin, axis, angle), and R(x) = A*x + b. Applying
    that to the weighted sum means the posed share becomes A*moved + b*s, where
    s is the posed weight and b = origin - A*origin (the pivot is fixed, so it
    cancels out of rotate_about and has to be restored by hand).
    """
    b = tuple(origin[i] - rotate_vector(origin, axis, angle)[i] for i in range(3))
    out = []
    for index, v in enumerate(verts):
        held = [0.0, 0.0, 0.0]
        moved = [0.0, 0.0, 0.0]
        posed_weight = 0.0
        for name, weight in weights[index]:
            target = moved if name in posed_names else held
            target[0] += weight * v[0]
            target[1] += weight * v[1]
            target[2] += weight * v[2]
            if name in posed_names:
                posed_weight += weight
        rotated = rotate_vector(moved, axis, angle)
        out.append(tuple(
            held[i] + rotated[i] + posed_weight * b[i] for i in range(3)
        ))
    return out


# --- checks ----------------------------------------------------------------


def check_rig(res, label, s, bones):
    names = [b[0] for b in bones]
    res.check(
        len(names) == len(set(names)),
        label + ": bone names unique",
        str(sorted(n for n in names if names.count(n) > 1)),
    )
    res.check(
        sorted(names) == sorted(EXPECTED_BONES),
        label + ": bone name set matches UE5 Manny convention",
        "unexpected={0} missing={1}".format(
            sorted(set(names) - set(EXPECTED_BONES)),
            sorted(set(EXPECTED_BONES) - set(names)),
        ),
    )
    bmap = bone_map(bones)
    for name in names:
        if name not in EXPECTED_BONES:
            continue
        res.check(
            bmap[name][0] == EXPECTED_BONES[name],
            label + ": parent of " + name,
            "got {0}, expected {1}".format(bmap[name][0], EXPECTED_BONES[name]),
        )
    roots = [b[0] for b in bones if b[1] is None]
    res.check(roots == ["root"], label + ": exactly one root", str(roots))

    arm = s["arm"]
    leg = s["leg"]
    z = s["z"]
    length_pairs = [
        ("upperarm_l", arm["upper_len"]),
        ("lowerarm_l", arm["lower_len"]),
        ("hand_l", arm["hand_len"]),
        ("upperarm_r", arm["upper_len"]),
        ("lowerarm_r", arm["lower_len"]),
        ("hand_r", arm["hand_len"]),
        ("thigh_l", leg["thigh_len"]),
        ("calf_l", leg["shin_len"]),
        ("thigh_r", leg["thigh_len"]),
        ("calf_r", leg["shin_len"]),
    ]
    for name, expected in length_pairs:
        parent, head, tail = bmap[name]
        res.near(
            seg_length(head, tail), expected, LENGTH_TOL,
            label + ": " + name + " length",
        )

    foot_len = geometry.FOOT_LENGTH_RATIO * z["head_top"]
    for side in ("foot_l", "foot_r"):
        _p, head, tail = bmap[side]
        res.check(
            abs(head[1]) < 1e-6 and tail[1] < -0.6 * foot_len,
            label + ": " + side + " points forward along -Y",
            "head_y={0:.2f} tail_y={1:.2f}".format(head[1], tail[1]),
        )

    # The neck was once 3.4 cm long because it spanned only the visible neck.
    # It must now cover the neck_base -> chin gap.
    _p, head, tail = bmap["neck_01"]
    res.near(head[2], z["neck_base"], 1e-6, label + ": neck_01 starts at neck_base")
    res.check(
        seg_length(head, tail) > 5.0,
        label + ": neck_01 is anatomically long",
        "{0:.2f} cm".format(seg_length(head, tail)),
    )

    # Head must not be larger than the neck it sits on.
    res.check(
        s["head"]["width"] < 0.5 * s["torso"]["shoulder_w"],
        label + ": head narrower than half the shoulder width",
        "{0:.1f} vs {1:.1f}".format(s["head"]["width"], 0.5 * s["torso"]["shoulder_w"]),
    )


def check_measurements(res, label, m, s):
    h = m.height
    res.near(s["z"]["head_top"], h, 1e-6, label + ": stature preserved in landmarks")
    if m.arm_span > 0.0:
        res.near(
            s["arm"]["tpose_span"], m.arm_span, 0.01,
            label + ": T-pose reach equals the measured arm span",
        )
    res.check(
        s["arm"]["arm_scale"] <= 1.15 + 1e-9,
        label + ": arm chain stayed inside the clamp",
        str(s["arm"]["arm_scale"]),
    )


def check_mesh(res, label, m, s, segments=28):
    verts, faces, parts, vparts = geometry.build_body(s, segments, 1.0)
    bb = bounds(verts)
    h = m.height
    res.near(bb["max"][2], h, 1e-6, label + ": crown reaches stature")
    res.near(bb["min"][2], 0.0, 1e-6, label + ": soles touch the floor")
    res.check(
        all(vparts[i] is not None for i in range(len(verts))),
        label + ": every vertex is tagged with a region",
        str(sum(1 for v in vparts if v is None)),
    )
    res.check(len(faces) > 0 and len(verts) > 0, label + ": body mesh is non-empty")
    return verts, faces, parts, vparts


def check_weights(res, label, verts, vparts, bones):
    weights = rig_mod.compute_weights(verts, bones, vparts)
    names = {b[0] for b in bones}
    worst_sum = 0.0
    worst_count = 0
    leaked = 0
    for index, entry in enumerate(weights):
        total = sum(w for _n, w in entry)
        worst_sum = max(worst_sum, abs(total - 1.0))
        worst_count = max(worst_count, len(entry))
        allowed = rig_mod.PART_BONES.get(vparts[index])
        if allowed is not None:
            if any(n not in allowed for n, _w in entry):
                leaked += 1
        if any(n not in names for n, _w in entry):
            leaked += 1
    res.near(worst_sum, 0.0, WEIGHT_EPS, label + ": weights normalised")
    res.check(
        worst_count <= 4, label + ": at most 4 influences per vertex", str(worst_count)
    )
    res.check(
        leaked == 0, label + ": no weights outside the region", str(leaked)
    )
    return weights


def check_pose(res, label, verts, vparts, weights, bones):
    """Raising one shoulder must move only that arm chain."""
    origin = bone_map(bones)["upperarm_l"][1]
    angle = math.radians(POSE_ANGLE_DEG)
    posed_names = set(descendants(bones, "upperarm_l"))

    moved = skin(verts, weights, posed_names, origin, POSE_AXIS, angle)
    worst = {}
    for index in range(len(verts)):
        region = vparts[index]
        d = math.sqrt(
            (moved[index][0] - verts[index][0]) ** 2
            + (moved[index][1] - verts[index][1]) ** 2
            + (moved[index][2] - verts[index][2]) ** 2
        )
        worst[region] = max(worst.get(region, 0.0), d)

    for region in STATIC_REGIONS:
        res.check(
            worst.get(region, 0.0) < 0.05,
            label + ": " + region + " stays put when the arm raises",
            "{0:.3f} cm".format(worst.get(region, 0.0)),
        )
    for region in MOVING_REGIONS:
        res.check(
            worst.get(region, 0.0) > 20.0,
            label + ": " + region + " actually moves",
            "{0:.3f} cm".format(worst.get(region, 0.0)),
        )
    res.check(
        worst.get("arm_R", 0.0) < 0.05,
        label + ": the opposite arm stays put",
        "{0:.3f} cm".format(worst.get("arm_R", 0.0)),
    )
    return worst


def check_garments(res, label, s, bones):
    """Garments must exist and must be weighted by the same regional rules."""
    for name in GARMENTS:
        gv, gf, gvparts = geometry.build_garment(name, s, 28, 1.0)
        if not res.check(
            gf, label + ": garment " + name + " has geometry"
        ):
            continue
        gweights = rig_mod.compute_weights(gv, bones, gvparts)
        res.check(
            all(sum(w for _n, w in entry) > 0.99 for entry in gweights),
            label + ": garment " + name + " weights normalised",
        )
        res.check(
            len(gv) == len(gvparts),
            label + ": garment " + name + " region tags match vertex count",
        )


def check_scaling(res, label, m, bones):
    """The same measurements must produce a consistent body at any stature."""
    reference = metrics_mod.Metrics()
    reference.sex = m.sex
    reference.weight = m.weight
    reference.height = 180.0
    reference.arm_span = 0.0
    ref_bones = bone_map(rig_mod.build_bones(reference.sections()))

    bmap = bone_map(bones)
    for name in ("upperarm_l", "lowerarm_l", "thigh_l", "calf_l", "foot_l"):
        this = seg_length(bmap[name][1], bmap[name][2])
        ref = seg_length(ref_bones[name][1], ref_bones[name][2])
        ratio = this / ref
        res.near(
            ratio,
            m.height / 180.0,
            0.05,
            label + ": " + name + " scales with stature",
        )


def run():
    res = Results()
    for label, m in PROFILES:
        s = m.sections()
        bones = rig_mod.build_bones(s)
        check_rig(res, label, s, bones)
        check_measurements(res, label, m, s)
        verts, _faces, _parts, vparts = check_mesh(res, label, m, s)
        weights = check_weights(res, label, verts, vparts, bones)
        check_pose(res, label, verts, vparts, weights, bones)
        check_garments(res, label, s, bones)
        check_scaling(res, label, m, bones)

    print("checks run: {0}".format(res.count))
    if res.failures:
        print("FAILURES ({0}):".format(len(res.failures)))
        for failure in res.failures:
            print("  - " + failure)
        return 1
    print("all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(run())
