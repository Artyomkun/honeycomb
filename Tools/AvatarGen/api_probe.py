"""
Probe the UE 5.8 Python surface for skeleton/bone introspection.

Property names shift between engine versions; this prints what actually
exists instead of guessing, so the rig generator can target real API.
"""
import json
import os
import unreal

SKELETON = "/Game/XRMannequins/Meshes/SK_MannequinsXR.SK_MannequinsXR"
MESH = "/Game/XRMannequins/Meshes/SKM_MannyXR_left.SKM_MannyXR_left"

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "api_probe.json")

report = {}


def public_names(obj):
    return sorted(n for n in dir(obj) if not n.startswith("_"))


def main():
    skel = unreal.load_asset(SKELETON)
    mesh = unreal.load_asset(MESH)

    report["skeleton_class"] = skel.get_class().get_name() if skel else None
    report["skeleton_dir"] = public_names(skel) if skel else []
    report["mesh_class"] = mesh.get_class().get_name() if mesh else None
    report["mesh_dir"] = public_names(mesh) if mesh else []

    # BoneNode field names.
    if skel:
        try:
            tree = skel.get_editor_property("bone_tree")
            report["bone_tree_len"] = len(tree)
            node = tree[0]
            report["bonenode_dir"] = public_names(node)
            report["bonenode_values"] = {}
            for name in report["bonenode_dir"]:
                try:
                    report["bonenode_values"][name] = str(getattr(node, name))
                except Exception as exc:  # noqa: BLE001
                    report["bonenode_values"][name] = "ERR {}".format(exc)
        except Exception as exc:  # noqa: BLE001
            report["bone_tree_error"] = str(exc)

    # Candidate ref-skeleton accessors across engine versions.
    candidates = [
        "ref_skeleton",
        "reference_skeleton",
        "skeleton",
        "skeletal_mesh_asset_skeleton",
        "bone_names",
        "bones",
    ]
    for label, obj in (("skeleton", skel), ("mesh", mesh)):
        found = {}
        for name in candidates:
            if obj is None:
                continue
            try:
                value = obj.get_editor_property(name)
                found[name] = "OK -> {}".format(type(value).__name__)
            except Exception as exc:  # noqa: BLE001
                found[name] = "FAIL {}".format(exc)
        report["{}_props".format(label)] = found

    # Python-side skeletal helpers that could expose bone parents.
    helpers = {}
    for name in public_names(unreal):
        lowered = name.lower()
        if any(k in lowered for k in ("skeleton", "skeletalmesh", "anim")):
            helpers[name] = [m for m in public_names(getattr(unreal, name))
                             if any(k in m.lower() for k in ("bone", "parent", "root"))]
    report["helpers"] = helpers

    with open(OUT, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2, ensure_ascii=False, default=str)
    unreal.log("[api_probe] wrote {}".format(OUT))


try:
    main()
except Exception as exc:  # noqa: BLE001
    unreal.log("[api_probe] EXCEPTION {}".format(exc))
    with open(OUT, "w", encoding="utf-8") as handle:
        json.dump({"error": str(exc)}, handle, indent=2, ensure_ascii=False)