"""
Parametric body and garment surface construction.

The body is built as a stack of elliptical cross-sections (lofting) driven by
the anthropometric landmarks from metrics.py. Garments reuse the exact same
ring system with an outward offset over a sub-range of sections, so clothing
fits by construction instead of needing shrinkwrap against a finished body.
"""
import math

TAU = math.pi * 2.0

# A-pose arm angle from vertical, shared by the upper arm and forearm so the
# chain reads as one straight limb instead of a broken elbow. UE animates this
# skeleton in a relaxed A-pose by default.
ARM_POSE_DEG = 19.0

# Foot length as a fraction of stature. A fixed foot length would leave tall
# players with stubs and short players with flippers, so both the foot mesh and
# the foot bone derive their length from this ratio.
FOOT_LENGTH_RATIO = 0.152


# --- primitives -----------------------------------------------------------


def superellipse_ring(cx, cy, cz, rx, ry, segments, squareness=2.0, z_scale=1.0):
    """One cross-section as a list of (x, y, z).

    squareness 2.0 gives a pure ellipse (limbs, head); 3.5-4.5 gives the
    rounded-rectangle sections a ribcage and pelvis need.
    """
    power = 2.0 / max(squareness, 0.5)
    points = []
    for i in range(segments):
        theta = TAU * i / segments
        cos_t = math.cos(theta)
        sin_t = math.sin(theta)
        x = rx * math.copysign(abs(cos_t) ** power, cos_t)
        y = ry * math.copysign(abs(sin_t) ** power, sin_t)
        points.append((cx + x, cy + y * z_scale, cz))
    return points


def taper(p0, p1, radius0, radius1, steps, segments, squareness=2.0):
    """A limb: rings interpolated from one end cap to the other."""
    rings = []
    for i in range(steps + 1):
        t = i / float(steps)
        cx = p0[0] + (p1[0] - p0[0]) * t
        cy = p0[1] + (p1[1] - p0[1]) * t
        cz = p0[2] + (p1[2] - p0[2]) * t
        # Smoothstep keeps the radius change from creasing at the joints.
        s = t * t * (3.0 - 2.0 * t)
        r = radius0 + (radius1 - radius0) * s
        rings.append(superellipse_ring(cx, cy, cz, r, r, segments, squareness))
    return rings


def loft(rings, cap_start=True, cap_end=True):
    """Stitch consecutive rings into a quad tube, returns (verts, faces).

    Ring vertex order is shared by every ring, so the quad strip is direct.
    """
    segments = len(rings[0])
    verts = []
    for ring in rings:
        verts.extend(ring)

    faces = []
    for r in range(len(rings) - 1):
        base_a = r * segments
        base_b = (r + 1) * segments
        for i in range(segments):
            j = (i + 1) % segments
            faces.append((base_a + i, base_a + j, base_b + j, base_b + i))

    if cap_start:
        centre = len(verts)
        verts.append(_centroid(rings[0]))
        base = 0
        for i in range(segments):
            j = (i + 1) % segments
            faces.append((centre, base + j, base + i))

    if cap_end:
        centre = len(verts)
        verts.append(_centroid(rings[-1]))
        base = (len(rings) - 1) * segments
        for i in range(segments):
            j = (i + 1) % segments
            faces.append((centre, base + i, base + j))

    return verts, faces


def _centroid(ring):
    n = float(len(ring))
    return (
        sum(p[0] for p in ring) / n,
        sum(p[1] for p in ring) / n,
        sum(p[2] for p in ring) / n,
    )


def append_mesh(target_verts, target_faces, verts, faces):
    """Merge a part into an accumulating mesh."""
    offset = len(target_verts)
    target_verts.extend(verts)
    for face in faces:
        target_faces.append(tuple(offset + i for i in face))


def scale_xyz(verts, sx, sy, sz):
    return [(v[0] * sx, v[1] * sy, v[2] * sz) for v in verts]


# --- body -----------------------------------------------------------------


def build_body(s, segments=28, quality=1.0):
    """Return (verts, faces, parts) for the whole body.

    parts maps a face-range label to the vertex index range it covers, so
    skin weights can be assigned per region instead of per-vertex guesswork.
    """
    z = s["z"]
    torso = s["torso"]
    arm = s["arm"]
    leg = s["leg"]
    head = s["head"]

    verts, faces, parts = [], [], {}

    # --- torso: crotch up to the neck, widening through the shoulders ---
    trunk_profile = [
        (z["crotch"] - 4.0, torso["hip_w"] * 0.50, torso["hip_depth"] * 0.58, 2.6),
        (z["crotch"] + 3.0, torso["hip_w"] * 0.96, torso["hip_depth"] * 0.97, 3.0),
        (z["iliac"], torso["hip_w"] * 1.00, torso["hip_depth"] * 1.00, 3.2),
        (z["waist"], torso["waist_w"], torso["waist_depth"], 2.8),
        (z["chest"] - 8.0, torso["chest_w"] * 0.97, torso["chest_depth"] * 0.99, 2.9),
        (z["chest"] + 4.0, torso["chest_w"], torso["chest_depth"], 2.8),
        (z["shoulder"] - 6.0, torso["shoulder_w"] * 0.92, torso["shoulder_depth"] * 0.90, 3.0),
        (z["shoulder"] - 1.0, torso["shoulder_w"] * 0.99, torso["shoulder_depth"] * 0.86, 3.4),
    ]
    rings = []
    for cz, rx, ry, sq in trunk_profile:
        rings.append(superellipse_ring(0.0, 0.0, cz, rx * 0.5, ry * 0.5, segments, sq))

    # Neck: a narrow continuation of the top trunk ring. The neck is wider in
    # depth than in width, so the two radii are set separately.
    neck_rx = torso["neck_r"] * 0.56
    neck_ry = torso["neck_r"] * 0.64
    for cz, factor in (
        (z["shoulder"] + 3.0, 1.10),
        (z["neck_base"], 1.00),
    ):
        rings.append(superellipse_ring(0.0, 0.0, cz, neck_rx * factor, neck_ry * factor, segments, 2.2))

    start = len(faces)
    tv, tf = loft(rings, cap_start=True, cap_end=False)
    append_mesh(verts, faces, tv, tf)
    parts["torso"] = (start, len(faces))

    # --- head: neck top, jaw, eye line, crown ---
    head_rings = []
    hw, hd = head["width"] * 0.5, head["depth"] * 0.5
    head_profile = [
        (z["neck_base"] + (z["chin"] - z["neck_base"]) * 0.35, neck_rx * 1.02, neck_ry * 1.06, 2.2),
        (z["chin"] - 1.0, hw * 0.62, hd * 0.70, 2.3),
        (z["chin"] + 2.5, hw * 0.84, hd * 0.94, 2.4),
        (z["eye"] - 2.0, hw * 1.00, hd * 1.00, 2.5),
        (z["eye"] + 5.0, hw * 0.98, hd * 1.00, 2.5),
        (z["head_top"] - 3.0, hw * 0.78, hd * 0.84, 2.4),
        (z["head_top"] - 0.6, hw * 0.44, hd * 0.46, 2.2),
        (z["head_top"], hw * 0.16, hd * 0.18, 2.1),
    ]
    for cz, rx, ry, sq in head_profile:
        head_rings.append(superellipse_ring(0.0, -hd * 0.06, cz, rx, ry, segments, sq))
    start = len(faces)
    tv, tf = loft(head_rings, cap_start=False, cap_end=True)
    append_mesh(verts, faces, tv, tf)
    parts["head"] = (start, len(faces))

    # --- arms: shoulder socket to wrist, mirrored ---
    tan_pose = math.tan(math.radians(ARM_POSE_DEG))
    for side, sign in (("L", 1.0), ("R", -1.0)):
        sx = torso["shoulder_w"] * 0.5 * 0.92 * sign
        shoulder = (sx, 0.0, z["shoulder"] - 3.0)
        # Lateral offset follows the shared pose angle from the vertical drop,
        # so upper arm and forearm stay collinear.
        upper_dx = (shoulder[2] - arm["elbow_z"]) * tan_pose
        lower_dx = (arm["elbow_z"] - arm["wrist_z"]) * tan_pose
        elbow = (shoulder[0] + sign * upper_dx, 0.0, arm["elbow_z"])
        wrist = (elbow[0] + sign * lower_dx, 0.0, arm["wrist_z"])
        pose_rad = math.radians(ARM_POSE_DEG)
        hand_end = (
            wrist[0] + sign * arm["hand_len"] * math.sin(pose_rad),
            0.0,
            arm["wrist_z"] - arm["hand_len"] * math.cos(pose_rad),
        )

        steps = max(3, int(round(6 * quality)))
        upper = taper(shoulder, elbow, arm["upper_r"], arm["elbow_r"], steps, segments, 2.2)
        lower = taper(elbow, wrist, arm["elbow_r"], arm["wrist_r"], steps, segments, 2.2)
        hand = taper(wrist, hand_end, arm["wrist_r"] * 1.18, arm["wrist_r"] * 0.72, steps, segments, 2.2)

        start = len(faces)
        tv, tf = loft(upper + lower[1:] + hand[1:], cap_start=True, cap_end=True)
        append_mesh(verts, faces, tv, tf)
        parts["arm_" + side] = (start, len(faces))

    # --- legs: hip to ankle ---
    for side, sign in (("L", 1.0), ("R", -1.0)):
        hip = (leg["thigh_x"] * sign, 0.0, z["hip_joint"])
        knee = (leg["thigh_x"] * 0.92 * sign, 0.0, z["knee"])
        ankle = (leg["ankle_x"] * sign, 0.0, z["ankle"])

        steps = max(3, int(round(7 * quality)))
        thigh = taper(hip, knee, leg["thigh_r"], leg["knee_r"], steps, segments, 2.3)
        shin = taper(knee, ankle, leg["knee_r"], leg["ankle_r"], steps, segments, 2.2)

        start = len(faces)
        tv, tf = loft(thigh + shin[1:], cap_start=True, cap_end=True)
        append_mesh(verts, faces, tv, tf)
        parts["leg_" + side] = (start, len(faces))

        # Foot: without it the leg ends at ankle height and the body floats
        # above the floor. Shoes are generated as a shell over this.
        foot_len = FOOT_LENGTH_RATIO * z["head_top"]
        foot = [
            (0.0, 0.0, z["ankle"] + 1.0, leg["ankle_r"] * 1.12),
            (0.0, -0.30 * foot_len, z["ankle"] - 1.0, leg["ankle_r"] * 1.16),
            (0.0, -0.68 * foot_len, 1.6, leg["ankle_r"] * 1.10),
            (0.0, -1.00 * foot_len, 0.0, leg["ankle_r"] * 0.80),
        ]
        foot_rings = [
            superellipse_ring(ankle[0] + px, py, pz, r, r, segments, 2.3)
            for px, py, pz, r in foot
        ]
        start = len(faces)
        tv, tf = loft(foot_rings, cap_start=False, cap_end=True)
        append_mesh(verts, faces, tv, tf)
        parts["foot_" + side] = (start, len(faces))

    vparts = vertex_parts(len(verts), faces, parts)
    return verts, faces, parts, vparts


def vertex_parts(vert_count, faces, parts):
    """Map every vertex index to the body region its faces belong to.

    Parts are separate lofts that do not share vertices, so this is exact.
    """
    vp = [None] * vert_count
    for name, (start, count) in parts.items():
        for face in faces[start:start + count]:
            for index in face:
                vp[index] = name
    return vp


# --- garments (the "skin" layer) -----------------------------------------


# Each garment is a range of the body profile with an outward offset. Offsets
# are in centimetres of cloth plus the air gap that stops z-fighting.
GARMENTS = {
    "tee": {"torso": 0.55, "sleeve": 0.45, "cover_torso": ("waist", "neck_base")},
    "longsleeve": {"torso": 0.55, "sleeve": 0.45, "cover_torso": ("waist", "neck_base"),
                   "sleeve_full": True},
    "tank": {"torso": 0.40, "sleeve": None, "cover_torso": ("iliac", "neck_base")},
    "trousers": {"torso": 0.60, "sleeve": 0.50, "cover_torso": ("crotch", "iliac"),
                 "cover_leg": True},
    "shorts": {"torso": 0.60, "sleeve": 0.50, "cover_torso": ("crotch", "iliac"),
               "cover_leg": "half"},
    "shoes": {"torso": 0.0, "sleeve": 0.0, "cover_torso": None, "foot_only": True},
}


def build_garment(name, s, segments=28, quality=1.0):
    """Build one garment as an offset shell of the body profile.

    Returns (verts, faces, vparts) or (None, None, None) when it does not
    apply. vparts tags every vertex with its body region so that skin
    weighting can restrict which bones are allowed to influence it.
    """
    spec = GARMENTS.get(name)
    if spec is None:
        return None, None, None

    verts, faces = [], []
    vparts = []

    def add(tv, tf, part):
        start = len(verts)
        append_mesh(verts, faces, tv, tf)
        vparts.extend([part] * (len(verts) - start))

    if spec.get("foot_only"):
        return _build_feet(s, spec, segments, quality)

    z = s["z"]
    torso = s["torso"]
    arm = s["arm"]
    leg = s["leg"]
    off = spec["torso"]

    # Torso shell: same profile as the body, pushed out, trimmed to the range.
    low_key, high_key = spec["cover_torso"]
    low_z, high_z = z[low_key], z[high_key]

    trunk_profile = [
        (z["crotch"] - 4.0, torso["hip_w"] * 0.50, torso["hip_depth"] * 0.58, 2.6),
        (z["crotch"] + 3.0, torso["hip_w"] * 0.96, torso["hip_depth"] * 0.97, 3.0),
        (z["iliac"], torso["hip_w"] * 1.00, torso["hip_depth"] * 1.00, 3.2),
        (z["waist"], torso["waist_w"], torso["waist_depth"], 2.8),
        (z["chest"] - 8.0, torso["chest_w"] * 0.97, torso["chest_depth"] * 0.99, 2.9),
        (z["chest"] + 4.0, torso["chest_w"], torso["chest_depth"], 2.8),
        (z["shoulder"] - 6.0, torso["shoulder_w"] * 0.92, torso["shoulder_depth"] * 0.90, 3.0),
        (z["shoulder"] - 1.0, torso["shoulder_w"] * 0.99, torso["shoulder_depth"] * 0.86, 3.4),
    ]
    rings = []
    for cz, rx, ry, sq in trunk_profile:
        if cz < low_z - 0.5 or cz > high_z + 0.5:
            continue
        rings.append(
            superellipse_ring(0.0, 0.0, cz, (rx * 0.5) + off, (ry * 0.5) + off, segments, sq)
        )
    if len(rings) >= 2:
        tv, tf = loft(rings, cap_start=True, cap_end=True)
        add(tv, tf, "torso")

    # Sleeves: upper arm, and the whole arm for long sleeves. The sleeve must
    # follow the same A-pose chain as the body arm, otherwise it cuts across the
    # elbow that the mesh actually has.
    sleeve_off = spec.get("sleeve")
    if sleeve_off:
        tan_pose = math.tan(math.radians(ARM_POSE_DEG))
        for side, sign in (("L", 1.0), ("R", -1.0)):
            sx = torso["shoulder_w"] * 0.5 * 0.92 * sign
            shoulder = (sx, 0.0, arm["shoulder_z"])
            upper_dx = (shoulder[2] - arm["elbow_z"]) * tan_pose
            lower_dx = (arm["elbow_z"] - arm["wrist_z"]) * tan_pose
            elbow = (sx + sign * upper_dx, 0.0, arm["elbow_z"])
            wrist = (sx + sign * (upper_dx + lower_dx), 0.0, arm["wrist_z"])
            steps = max(3, int(round(6 * quality)))
            upper = taper(shoulder, elbow, arm["upper_r"] + sleeve_off, arm["elbow_r"] + sleeve_off,
                          steps, segments, 2.2)
            if spec.get("sleeve_full"):
                lower = taper(elbow, wrist, arm["elbow_r"] + sleeve_off, arm["wrist_r"] + sleeve_off,
                              steps, segments, 2.2)
                tv, tf = loft(upper + lower[1:], cap_start=True, cap_end=True)
            else:
                # Short sleeve ends above the elbow.
                mid = (shoulder[0] + (elbow[0] - shoulder[0]) * 0.52,
                       0.0, shoulder[2] + (elbow[2] - shoulder[2]) * 0.52)
                mid_r = arm["upper_r"] + (arm["elbow_r"] - arm["upper_r"]) * 0.52 + sleeve_off
                lower = taper(elbow, mid, arm["elbow_r"] + sleeve_off, mid_r, 2, segments, 2.2)
                tv, tf = loft(upper + lower[1:], cap_start=True, cap_end=True)
            add(tv, tf, "arm_" + side)

    # Trouser legs.
    cover_leg = spec.get("cover_leg")
    if cover_leg:
        leg_off = spec.get("sleeve", 0.5)
        frac = 0.52 if cover_leg == "half" else 0.0
        for side, sign in (("L", 1.0), ("R", -1.0)):
            hip = (leg["thigh_x"] * sign, 0.0, z["hip_joint"])
            knee = (leg["thigh_x"] * 0.92 * sign, 0.0, z["knee"])
            ankle = (leg["ankle_x"] * sign, 0.0, z["ankle"])
            end_z = ankle[2] + (knee[2] - ankle[2]) * frac
            end = (ankle[0], 0.0, end_z)
            steps = max(3, int(round(7 * quality)))
            thigh = taper(hip, knee, leg["thigh_r"] + leg_off, leg["knee_r"] + leg_off, steps, segments, 2.3)
            shin = taper(knee, end, leg["knee_r"] + leg_off, leg["ankle_r"] + leg_off, steps, segments, 2.2)
            tv, tf = loft(thigh + shin[1:], cap_start=True, cap_end=True)
            add(tv, tf, "leg_" + side)

    if not faces:
        return None, None, None
    return verts, faces, vparts


def _build_feet(s, spec, segments, quality):
    """Shoes: a wedge from the ankle forward, slightly wider than the foot."""
    z = s["z"]
    leg = s["leg"]
    off = 0.5
    foot_len = FOOT_LENGTH_RATIO * z["head_top"]
    verts, faces, vparts = [], [], []
    for side, sign in (("L", 1.0), ("R", -1.0)):
        ax = leg["ankle_x"] * sign
        rings = []
        profile = [
            (0.0, 0.10, z["ankle"] + 6.0, leg["ankle_r"] + off),
            (0.0, -0.20 * foot_len, z["ankle"] + 1.0, leg["ankle_r"] + off * 1.25),
            (0.0, -0.48 * foot_len, z["ankle"] - 1.0, leg["ankle_r"] + off * 1.5),
            (0.0, -0.74 * foot_len, 1.5, leg["ankle_r"] + off * 1.7),
            (0.0, -1.00 * foot_len, 0.0, leg["ankle_r"] + off * 0.9),
        ]
        for px, py, pz, r in profile:
            rings.append(superellipse_ring(ax + px, py, pz, r, r, segments, 2.3))
        tv, tf = loft(rings, cap_start=True, cap_end=True)
        start = len(verts)
        append_mesh(verts, faces, tv, tf)
        vparts.extend(["foot_" + side] * (len(verts) - start))
    return verts, faces, vparts
