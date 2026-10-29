"""Probe the imported Honeycomb avatar: bone names, scale, bounds."""
import json

import unreal

MESH = "/Game/Avatars/SK_HoneycombAvatar.SK_HoneycombAvatar"
REPORT = r"C:\Project\Honeycomb\Tools\AvatarGen\ue_mesh_probe.json"

report = {}


def safe(label, fn):
    try:
        value = fn()
        try:
            json.dumps(value)
            report[label] = value
        except (TypeError, ValueError):
            report[label] = str(value)
        return value
    except Exception as exc:  # noqa: BLE001
        report[label] = "ERROR: {}".format(exc)
        return None


mesh = unreal.load_asset(MESH)
skeleton = mesh.get_editor_property("skeleton")

# 1. bounds: try every plausible property name.
for name in ("extended_bounds", "imported_bounds", "bounds",
             "positive_bounds_extent", "negative_bounds_extent"):
    def read(n=name):
        b = mesh.get_editor_property(n)
        try:
            return {
                "origin": [b.origin.x, b.origin.y, b.origin.z],
                "extent": [b.box_extent.x, b.box_extent.y, b.box_extent.z],
            }
        except AttributeError:
            return str(b)
    safe("mesh_prop_" + name, read)

for name in ("get_bounds", "get_extended_bounds"):
    if hasattr(mesh, name):
        safe("mesh_call_" + name, lambda n=name: str(getattr(mesh, n)()))

# 2. bone tree: try attribute then get_editor_property.
def bone_nodes():
    tree = skeleton.get_editor_property("bone_tree")
    out = []
    for node in tree:
        rec = {}
        for field in ("name", "parent_index", "translation", "depth"):
            for accessor in (lambda f=field: getattr(node, f),
                             lambda f=field: node.get_editor_property(f)):
                try:
                    v = accessor()
                    if hasattr(v, "x"):
                        v = [v.x, v.y, v.z]
                    rec[field] = v
                    break
                except Exception:  # noqa: BLE001
                    continue
        out.append(rec)
    return out

nodes = safe("bone_tree", bone_nodes)

if isinstance(nodes, list) and nodes:
    report["bone_names_from_tree"] = [n.get("name") for n in nodes]
    # cumulative height: walk parents, summing node translations.
    try:
        positions = {}
        def pos(i):
            if i in positions:
                return positions[i]
            n = nodes[i]
            t = n.get("translation") or [0, 0, 0]
            p = n.get("parent_index", -1)
            if p is None or p < 0 or p >= len(nodes):
                positions[i] = list(t)
            else:
                base = pos(p)
                positions[i] = [base[k] + t[k] for k in range(3)]
            return positions[i]
        for i in range(len(nodes)):
            pos(i)
        zs = [positions[i][2] for i in range(len(nodes))]
        report["skeleton_height_cm"] = round(max(zs) - min(zs), 1)
        report["skeleton_top_cm"] = round(max(zs), 1)
    except Exception as exc:  # noqa: BLE001
        report["height_error"] = str(exc)

# 3. subsystem helpers with the right arity.
sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
if sub is not None:
    safe("subsystem_methods",
         lambda: [m for m in dir(sub) if "bone" in m.lower() or "vert" in m.lower()])
    safe("get_bone_children_root",
         lambda: [str(x) for x in sub.get_bone_children(skeleton, "root")])
    safe("get_bone_children_empty",
         lambda: [str(x) for x in sub.get_bone_children(skeleton, "")])

# 4. vertex / triangle counts per LOD via every method that exists.
if sub is not None:
    safe("mesh_methods",
         lambda: [m for m in dir(sub) if "get_" in m][:60])

report["lod_count"] = safe("lod_count", lambda: sub.get_lod_count(mesh))

with open(REPORT, "w", encoding="utf-8") as handle:
    json.dump(report, handle, indent=2, ensure_ascii=False)
unreal.log("[probe] written {}".format(REPORT))
