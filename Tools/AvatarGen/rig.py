"""
Skeleton definition and automatic skin weighting.

Bone names are the UE5 Manny names (root/pelvis/spine_01..03/neck_01/head/
clavicle_l/upperarm_l/lowerarm_l/hand_l/thigh_l/calf_l/foot_l). Two reasons:

* the previous dot convention ("upper_arm.L") is not a valid UE bone name, so
  the FBX importer rewrote it to "upper_arm_L" anyway - the intended name was
  never what reached the game;
* naming the bones after the standard skeleton means retargeting, IK and the
  engine's own animation tooling recognise the avatar without a mapping table.

Clavicles exist because the shoulder-to-arm transition needs a joint to bend
at, which the original placeholder rig lacked. The torso carries three spine
bones (waist, chest, mid-thorax-to-neck-base) to match the Manny spine chain.

Weights are assigned from distance to the bone segments, with a soft
inter-pole falloff so elbows and knees bend instead of creasing.
"""
import math

import geometry

# Local-space bone list: (name, parent, head_xyz_cm, tail_xyz_cm).
# Built from the anthropometric landmarks so bone lengths follow the player.


def build_bones(s):
    z = s["z"]
    torso = s["torso"]
    arm = s["arm"]
    leg = s["leg"]

    pelvis_z = z["crotch"] + 2.0
    spine_z = z["waist"]
    chest_z = z["chest"]
    thorax_z = 0.5 * (z["chest"] + z["neck_base"])
    neck_z = z["neck_base"]

    bones = [
        ("root", None, (0.0, 0.0, 0.0), (0.0, 0.0, pelvis_z)),
        ("pelvis", "root", (0.0, 0.0, pelvis_z), (0.0, 0.0, spine_z)),
        ("spine_01", "pelvis", (0.0, 0.0, spine_z), (0.0, 0.0, chest_z)),
        ("spine_02", "spine_01", (0.0, 0.0, chest_z), (0.0, 0.0, thorax_z)),
        ("spine_03", "spine_02", (0.0, 0.0, thorax_z), (0.0, 0.0, neck_z)),
        ("neck_01", "spine_03", (0.0, 0.0, neck_z), (0.0, 0.0, z["chin"] - 1.0)),
        ("head", "neck_01", (0.0, 0.0, z["chin"] - 1.0), (0.0, 0.0, z["head_top"])),
    ]

    # Same A-pose angle as geometry.py: bones must run through the limb volume
    # the mesh was lofted along, otherwise automatic weights land off-centre.
    tan_pose = math.tan(math.radians(geometry.ARM_POSE_DEG))
    shoulder_x = torso["shoulder_w"] * 0.5 * 0.92
    clavicle_x = torso["shoulder_w"] * 0.5 * 0.30

    for side, sign in (("l", 1.0), ("r", -1.0)):
        shoulder_z = arm["shoulder_z"]
        shoulder_drop = shoulder_z - arm["elbow_z"]
        forearm_drop = arm["elbow_z"] - arm["wrist_z"]
        elbow_x = shoulder_x * sign + shoulder_drop * tan_pose * sign
        wrist_x = elbow_x + forearm_drop * tan_pose * sign
        pose_rad = math.radians(geometry.ARM_POSE_DEG)
        hand_x = wrist_x + arm["hand_len"] * math.sin(pose_rad) * sign
        hand_z = arm["wrist_z"] - arm["hand_len"] * math.cos(pose_rad)

        bones.append((
            "clavicle_" + side, "spine_03",
            (clavicle_x * sign, 0.0, shoulder_z + 1.0),
            (shoulder_x * sign, 0.0, shoulder_z),
        ))
        bones.append((
            "upperarm_" + side, "clavicle_" + side,
            (shoulder_x * sign, 0.0, shoulder_z),
            (elbow_x, 0.0, arm["elbow_z"]),
        ))
        bones.append((
            "lowerarm_" + side, "upperarm_" + side,
            (elbow_x, 0.0, arm["elbow_z"]),
            (wrist_x, 0.0, arm["wrist_z"]),
        ))
        bones.append((
            "hand_" + side, "lowerarm_" + side,
            (wrist_x, 0.0, arm["wrist_z"]),
            (hand_x, 0.0, hand_z),
        ))

        bones.append((
            "thigh_" + side, "pelvis",
            (leg["thigh_x"] * sign, 0.0, z["hip_joint"]),
            (leg["thigh_x"] * 0.92 * sign, 0.0, z["knee"]),
        ))
        bones.append((
            "calf_" + side, "thigh_" + side,
            (leg["thigh_x"] * 0.92 * sign, 0.0, z["knee"]),
            (leg["ankle_x"] * sign, 0.0, z["ankle"]),
        ))
        # The foot bone runs forward along -Y to the ball of the foot, so the
        # toes rotate with the foot instead of with the shin. Length matches the
        # lofted foot mesh, which is itself scaled by stature.
        foot_len = geometry.FOOT_LENGTH_RATIO * z["head_top"]
        bones.append((
            "foot_" + side, "calf_" + side,
            (leg["ankle_x"] * sign, 0.0, z["ankle"]),
            (leg["ankle_x"] * sign, -0.68 * foot_len, z["ankle"] * 0.15),
        ))

    return bones


# --- weighting ------------------------------------------------------------


def _point_segment_distance(px, py, pz, a, b):
    abx, aby, abz = b[0] - a[0], b[1] - a[1], b[2] - a[2]
    apx, apy, apz = px - a[0], py - a[1], pz - a[2]
    denom = abx * abx + aby * aby + abz * abz
    if denom < 1e-9:
        t = 0.0
    else:
        t = (apx * abx + apy * aby + apz * abz) / denom
        t = max(0.0, min(1.0, t))
    dx = apx - abx * t
    dy = apy - aby * t
    dz = apz - abz * t
    return math.sqrt(dx * dx + dy * dy + dz * dz), t


# Which bones may influence which body region. Pure distance cannot separate a
# forearm from the hip when the A-pose hangs the arm right beside the body, so
# every vertex is restricted to the bones that belong to its region.
PART_BONES = {
    "torso": (
        "root", "pelvis", "spine_01", "spine_02", "spine_03",
        "clavicle_l", "clavicle_r",
    ),
    "head": ("spine_03", "neck_01", "head"),
    "arm_L": ("spine_03", "clavicle_l", "upperarm_l", "lowerarm_l", "hand_l"),
    "arm_R": ("spine_03", "clavicle_r", "upperarm_r", "lowerarm_r", "hand_r"),
    "leg_L": ("pelvis", "thigh_l", "calf_l"),
    "leg_R": ("pelvis", "thigh_r", "calf_r"),
    "foot_L": ("calf_l", "foot_l"),
    "foot_R": ("calf_r", "foot_r"),
}


def compute_weights(verts, bones, vparts=None, max_influences=4, falloff=3.2):
    """Per-vertex bone weights from distance to each bone segment.

    Falls off as 1/d^falloff, keeps the nearest max_influences bones, and
    normalises. When vparts is supplied, only the bones listed for that
    vertex's region are considered, which stops limbs from bleeding into the
    torso. The inter-pole softness comes from the shared joint positions: a
    vertex near the elbow is close to both upper arm and forearm segments.
    """
    segments = [(name, head, tail) for (name, _p, head, tail) in bones]
    result = []

    for index, v in enumerate(verts):
        candidates = segments
        if vparts is not None:
            allowed = PART_BONES.get(vparts[index])
            if allowed is not None:
                candidates = [s for s in segments if s[0] in allowed]

        scored = []
        for name, head, tail in candidates:
            dist, _t = _point_segment_distance(v[0], v[1], v[2], head, tail)
            scored.append((dist, name))

        scored.sort(key=lambda item: item[0])
        picked = scored[:max_influences]

        total = 0.0
        weights = []
        for dist, name in picked:
            w = 1.0 / (max(dist, 0.6) ** falloff)
            weights.append([name, w])
            total += w

        if total <= 0.0:
            weights = [["pelvis", 1.0]]
        else:
            for item in weights:
                item[1] /= total

        result.append(weights)

    return result


def head_bone_names(bones):
    return [name for (name, _p, _h, _t) in bones]
