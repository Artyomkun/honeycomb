"""
Anthropometric body model for the Honeycomb VR avatar.

The avatar body must match the player. What XR hardware can actually measure
is limited, so every parameter carries a provenance flag:

    MEASURED   read directly from tracking (HMD height, hand-controller span)
    CALIBRATED entered by the player in the calibration step
    DERIVED    regressed from measured values via anthropometric ratios

The generator never silently invents a value: anything DERIVED is scaled off
the player's own measured stature, so changing measured height reshapes the
whole body consistently, and changing arm span overrides the arm chain
independently of stature.

Units are centimetres throughout, matching the C++ side. Blender works in
metres, so build_avatar.py scales on export.
"""
import json
import math

import geometry

# --- provenance ------------------------------------------------------------

MEASURED = "measured"
CALIBRATED = "calibrated"
DERIVED = "derived"
DEFAULT = "default"

# Minimum shoulder-hip slope for a plausible trunk, guards bad calibration.
MIN_TRUNK_SLOPE = 0.55
MAX_TRUNK_SLOPE = 1.45

# The arm tube is inset into the deltoid rather than starting at the acromion.
# geometry.py places the shoulder pivot at this fraction of the biacromial
# half-width, and the arm chain is derived from that same pivot so the body's
# horizontal fingertip reach matches the measured span.
ARM_PIVOT_FRACTION = 0.92

# The head is lofted as an ellipse. These ratios convert a head circumference
# into the bounding ellipse's breadth and length, solved so the lofted perimeter
# lands back on the target circumference. Adult breadth/depth is roughly
# 0.272/0.342 of the circumference (e.g. 15.5 x 19.5 cm at 57 cm).
HEAD_WIDTH_RATIO = 0.272
HEAD_DEPTH_RATIO = 0.342

# How far the glenohumeral (shoulder ball) joint sits below the acromion. The
# acromion is the surface landmark used for the biacromial width; the bone
# pivot is inside the body, below it.
GLENOHUMERAL_DROP = 3.0


def clamp(value, low, high):
    return max(low, min(high, value))


class Metrics(object):
    """A body description plus where each number came from."""

    def __init__(self):
        self.sex = "male"
        self.age = 30
        self.height = 175.0
        self.arm_span = 0.0
        self.weight = 0.0

        # Optional direct measurements, 0.0 means "not measured".
        self.chest = 0.0
        self.waist = 0.0
        self.hip = 0.0
        self.inseam = 0.0
        self.shoulder_width = 0.0
        self.head_circumference = 0.0

        self.provenance = {}
        self.notes = []

    # -- provenance bookkeeping --

    def _set(self, name, value, source):
        self.provenance[name] = source
        return value

    def source_of(self, name):
        return self.provenance.get(name, DEFAULT)

    def is_direct(self, name):
        return self.source_of(name) in (MEASURED, CALIBRATED)

    # -- serialisation --

    def to_dict(self):
        data = {
            "sex": self.sex,
            "age": self.age,
            "height": self.height,
            "arm_span": self.arm_span,
            "weight": self.weight,
            "chest": self.chest,
            "waist": self.waist,
            "hip": self.hip,
            "inseam": self.inseam,
            "shoulder_width": self.shoulder_width,
            "head_circumference": self.head_circumference,
            "provenance": self.provenance,
            "notes": self.notes,
        }
        data["sections"] = self.sections()
        return data

    @classmethod
    def from_dict(cls, data):
        m = cls()
        for key in (
            "sex", "age", "height", "arm_span", "weight",
            "chest", "waist", "hip", "inseam",
            "shoulder_width", "head_circumference",
        ):
            if key in data and data[key] is not None:
                setattr(m, key, data[key])
        m.provenance = dict(data.get("provenance", {}))
        m.notes = list(data.get("notes", []))
        return m

    # -- derived body description --

    def sections(self):
        """Resolve every skeleton landmark and girth.

        Returns a flat dict of segment endpoints (Z heights) and radii the
        mesh builder needs. Landmarks are fractions of stature, which is why
        they stay anatomically consistent for any player height.
        """
        sex = self.sex if self.sex in ("male", "female") else "male"
        f = RATIOS[sex]
        h = clamp(self.height, 120.0, 230.0)

        # Arm span overrides the arm chain. Deriving the chain from the
        # measurement instead of scaling stature is what makes the result
        # self-consistent: the fingertip-to-fingertip reach of the finished
        # body then equals the span the controllers actually reported.
        # The arm tube is inset into the deltoid, so the mesh pivot sits at
        # ARM_PIVOT_FRACTION of the biacromial half-width, not at the full
        # width. Deriving the chain from that same pivot is what makes the
        # finished body's horizontal fingertip reach equal the measured span.
        shoulder_w_nominal = f["biacromial"] * h
        chain_ratio = f["upper_arm"] + f["forearm"] + f["hand"]
        nominal_chain = chain_ratio * h
        span = self.arm_span if self.arm_span > 0.0 else 0.0
        if span > 0.0:
            self._set("arm_span", span, MEASURED)
        else:
            span = f["arm_span"] * h
            self._set("arm_span", span, DERIVED)

        arm_pivot_x = shoulder_w_nominal * 0.5 * ARM_PIVOT_FRACTION
        available = span * 0.5 - arm_pivot_x
        arm_scale = clamp(available / nominal_chain, 0.70, 1.15)
        self._set("arm_chain", arm_scale, DERIVED)

        bmi = 0.0
        if self.weight > 0.0 and h > 0.0:
            bmi = self.weight / ((h / 100.0) ** 2)
        bmi = clamp(bmi if bmi > 0.0 else 21.5, 15.0, 36.0)
        girth = clamp((bmi - 21.5) / 14.5, -0.45, 1.0)
        self._set("bmi", bmi, MEASURED if self.weight > 0.0 else DERIVED)

        z = {}
        z["ground"] = 0.0
        z["ankle"] = f["ankle"] * h
        z["knee"] = f["knee"] * h
        z["crotch"] = f["crotch"] * h
        z["hip_joint"] = f["hip_joint"] * h
        z["waist"] = f["waist"] * h
        z["iliac"] = f["iliac"] * h
        z["chest"] = f["chest"] * h
        z["nipple"] = f["nipple"] * h
        z["shoulder"] = f["shoulder"] * h
        z["neck_base"] = f["neck_base"] * h
        z["chin"] = f["chin"] * h
        z["eye"] = f["eye"] * h
        z["head_top"] = h

        if self.inseam > 0.0:
            z["crotch"] = self.inseam
            self._set("inseam", self.inseam, CALIBRATED)

        # --- widths and depths (elliptical cross-sections) ---

        shoulder_w = (
            self.shoulder_width
            if self.shoulder_width > 0.0
            else f["biacromial"] * h
        )
        if not self.is_direct("shoulder_width"):
            self._set("shoulder_width", shoulder_w, DERIVED)
        else:
            shoulder_w = self.shoulder_width

        hip_w = f["biiliac"] * h * (1.0 + 0.10 * girth)
        self._set("biiliac", hip_w, DERIVED)

        # Trunk depth: shoulders are always wider than hips, and the player
        # measurement overrides the girth estimate rather than fighting it.
        shoulder_depth = 0.72 * shoulder_w * (0.94 + 0.10 * girth)
        hip_depth = f["hip_depth_ratio"] * hip_w * (1.0 + 0.16 * girth)

        chest_c = self.chest if self.chest > 0.0 else f["chest_c"] * h * (1.0 + 0.10 * girth)
        waist_c = self.waist if self.waist > 0.0 else f["waist_c"] * h * (1.0 + 0.16 * girth)
        hip_c = self.hip if self.hip > 0.0 else f["hip_c"] * h * (1.0 + 0.14 * girth)

        for name, value in (
            ("chest_c", chest_c), ("waist_c", waist_c), ("hip_c", hip_c)
        ):
            self._set(name, value, CALIBRATED if self.is_direct(name.replace("_c", "")) else DERIVED)

        # A measured chest that is narrower than the hips means the player
        # entered numbers that do not describe a standing body. Widen the
        # chest instead of building an inverted trunk.
        if chest_c < hip_c * 0.78:
            chest_c = hip_c * 0.86
            self.notes.append("chest widened: measurement was below hip girth")

        waist_depth = 0.78 * waist_c / math.pi * 1.34
        chest_depth = 0.74 * chest_c / math.pi * 1.34
        hip_depth = max(hip_depth, 0.72 * hip_c / math.pi * 1.34)

        # --- arms ---

        arm = {
            # The arm bone starts at the glenohumeral socket, which sits below
            # the acromion landmark. Measuring the chain from the socket (rather
            # than from the acromion) makes every bone exactly as long as the
            # segment it drives, so skinning does not stretch at the shoulder.
            "shoulder_z": z["shoulder"] - GLENOHUMERAL_DROP,
            "upper_len": f["upper_arm"] * h * arm_scale,
            "lower_len": f["forearm"] * h * arm_scale,
            "hand_len": f["hand"] * h * arm_scale,
            "upper_r": 0.052 * h * (1.0 + 0.11 * girth),
            "elbow_r": 0.040 * h * (1.0 + 0.10 * girth),
            "wrist_r": 0.030 * h * (1.0 + 0.08 * girth),
        }
        arm["elbow_z"] = arm["shoulder_z"] - arm["upper_len"] * math.cos(
            math.radians(geometry.ARM_POSE_DEG))
        arm["wrist_z"] = arm["elbow_z"] - arm["lower_len"] * math.cos(
            math.radians(geometry.ARM_POSE_DEG))
        # Fingertips hang below the wrist in the A-pose; the rest of the arm
        # chain continues the same downward angle the segments already have.
        arm["drop"] = 0.18 * (arm["upper_len"] + arm["lower_len"]) / 0.9
        arm["forward"] = 0.10 * (arm["upper_len"] + arm["lower_len"]) / 0.9
        arm["arm_scale"] = arm_scale
        # Horizontal fingertip-to-fingertip reach, reported so callers can
        # verify the finished body against the measured span.
        arm["tpose_span"] = 2.0 * (arm_pivot_x + arm["upper_len"] + arm["lower_len"] + arm["hand_len"])

        # --- legs ---

        leg = {
            # Thigh runs from the hip joint to the knee; the crotch sits lower
            # than the femoral head and would make the thigh read short.
            "thigh_len": z["hip_joint"] - z["knee"],
            "shin_len": z["knee"] - z["ankle"],
            "thigh_r": 0.062 * h * (1.0 + 0.13 * girth),
            "knee_r": 0.046 * h * (1.0 + 0.12 * girth),
            "calf_r": 0.049 * h * (1.0 + 0.12 * girth),
            "ankle_r": 0.030 * h * (1.0 + 0.08 * girth),
            "thigh_x": 0.062 * h,
            "ankle_x": 0.040 * h,
        }

        # --- head ---

        head_c = (
            self.head_circumference
            if self.head_circumference > 0.0
            else f["head_c"] * h
        )
        if not self.is_direct("head_circumference"):
            self._set("head_circumference", head_c, DERIVED)
        head = {
            "height": z["head_top"] - z["neck_base"],
            # Ellipse width/depth for a target head circumference. A linear
            # circumference-to-diameter conversion overestimates the head,
            # because an ellipse of that aspect ratio has a longer perimeter
            # than its bounding circle.
            "width": head_c * HEAD_WIDTH_RATIO,
            "depth": head_c * HEAD_DEPTH_RATIO,
            "chin_z": z["chin"],
            "eye_z": z["eye"],
        }

        return {
            "z": z,
            "girth": girth,
            "bmi": bmi,
            "torso": {
                "shoulder_w": shoulder_w,
                "shoulder_depth": shoulder_depth,
                "hip_w": hip_w,
                "hip_depth": hip_depth,
                "chest_w": 0.86 * chest_c / math.pi * 1.30,
                "chest_depth": chest_depth,
                "waist_w": 0.86 * waist_c / math.pi * 1.30,
                "waist_depth": waist_depth,
                "neck_r": 0.052 * h,
                "shoulder_r": 0.085 * h,
            },
            "arm": arm,
            "leg": leg,
            "head": head,
        }


# Anthropometric ratios as fractions of stature, plus girth coefficients.
# Values follow standard adult body-proportion tables; they are regression
# estimates, not measurements, which is why everything is overridable.
RATIOS = {
    "male": {
        "ankle": 0.039,
        "knee": 0.285,
        "crotch": 0.470,
        "hip_joint": 0.530,
        "iliac": 0.585,
        "waist": 0.620,
        "chest": 0.720,
        "nipple": 0.735,
        "shoulder": 0.818,
        "neck_base": 0.826,
        "chin": 0.871,
        "eye": 0.935,
        "arm_span": 1.000,
        "upper_arm": 0.186,
        "forearm": 0.146,
        "hand": 0.108,
        "biacromial": 0.259,
        "biiliac": 0.191,
        "hip_depth_ratio": 0.78,
        "chest_c": 0.545,
        "waist_c": 0.470,
        "hip_c": 0.510,
        "head_c": 0.320,
    },
    "female": {
        "ankle": 0.039,
        "knee": 0.285,
        "crotch": 0.485,
        "hip_joint": 0.545,
        "iliac": 0.585,
        "waist": 0.615,
        "chest": 0.695,
        "nipple": 0.715,
        "shoulder": 0.813,
        "neck_base": 0.824,
        "chin": 0.869,
        "eye": 0.933,
        "arm_span": 0.955,
        "upper_arm": 0.178,
        "forearm": 0.140,
        "hand": 0.098,
        "biacromial": 0.240,
        "biiliac": 0.200,
        "hip_depth_ratio": 0.82,
        "chest_c": 0.505,
        "waist_c": 0.430,
        "hip_c": 0.540,
        "head_c": 0.313,
    },
}


def load(path):
    with open(path, "r", encoding="utf-8") as handle:
        return Metrics.from_dict(json.load(handle))


def dump(metrics, path):
    with open(path, "w", encoding="utf-8") as handle:
        json.dump(metrics.to_dict(), handle, indent=2, ensure_ascii=False)