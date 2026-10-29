"""Walk the imported avatar's bone hierarchy to confirm bone names/order."""
import json

import unreal

MESH = "/Game/Avatars/SK_HoneycombAvatar.SK_HoneycombAvatar"
REPORT = r"C:\Project\Honeycomb\Tools\AvatarGen\ue_bones_probe.json"

mesh = unreal.load_asset(MESH)
sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)

report = {}


def children(bone):
    try:
        return [str(x) for x in sub.get_bone_children(mesh, bone)]
    except Exception as exc:  # noqa: BLE001
        return ["ERROR:{}".format(exc)]


report["children_of_root"] = children("root")
report["children_of_empty"] = children("")

# Breadth-first walk from every seed that yields anything.
visited = []
queue = []
for seed in ("root", ""):
    for c in children(seed):
        if not c.startswith("ERROR"):
            queue.append(c)
seen = set()
while queue:
    b = queue.pop(0)
    if b in seen:
        continue
    seen.add(b)
    visited.append(b)
    for c in children(b):
        if not c.startswith("ERROR") and c not in seen:
            queue.append(c)

report["all_bones"] = visited
report["bone_count"] = len(visited)

# Parent lookup for a couple of key bones.
parents = {}
for b in ("upper_arm.L", "hand.L", "thigh.L", "foot.R", "head", "pelvis"):
    try:
        parents[b] = str(sub.get_bone_parent(mesh, b))
    except Exception as exc:  # noqa: BLE001
        parents[b] = "ERROR:{}".format(exc)
report["parents"] = parents

report["lod0_verts"] = sub.get_num_verts(mesh, 0)

with open(REPORT, "w", encoding="utf-8") as handle:
    json.dump(report, handle, indent=2, ensure_ascii=False)
unreal.log("[probe_bones] written")
