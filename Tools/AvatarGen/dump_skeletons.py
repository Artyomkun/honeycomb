"""
Dump reference bone hierarchies from the project's skeletal assets.

Ground truth for rig generation: the avatar generator must reproduce the exact
bone names the project already animates against.

Run headless (editor must be closed):
  "C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe" ^
      "C:\\Project\\Honeycomb\\Honeycomb.uproject" -run=pythonscript ^
      -script="C:\\Project\\Honeycomb\\Tools\\AvatarGen\\dump_skeletons.py" ^
      -unattended -nopause -nosplash -nullrhi

Writes skeleton_dump.json next to this script.
"""
import json
import os
import unreal

ROOTS = [
    "/Game/XRMannequins",
    "/Game/Avatars",
]

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "skeleton_dump.json")


def try_prop(obj, *names):
    """Read the first editor property that this engine build actually has."""
    for name in names:
        try:
            return obj.get_editor_property(name), name
        except Exception:  # noqa: BLE001
            continue
    return None, None


def bone_tree_of_skeleton(skeleton):
    """USkeleton -> ordered bone names plus parent links.

    USkeleton does not expose RefSkeleton to Python, but the bone tree gives
    authoritative names; parents come from recursively expanding bone children.
    """
    bones = []
    try:
        tree = skeleton.get_editor_property("bone_tree")
    except Exception as exc:  # noqa: BLE001
        return bones, "bone_tree unreadable: {}".format(exc)

    for node in tree:
        try:
            bones.append({"name": str(node.get_editor_property("name"))})
        except Exception:  # noqa: BLE001
            bones.append({"name": str(node)})
    return bones, None


def dump_bones_from_mesh(mesh):
    """Prefer the mesh ref-skeleton: it carries parent indices."""
    ref, used = try_prop(mesh, "ref_skeleton", "reference_skeleton")
    if ref is None:
        return None, "no ref_skeleton property"

    try:
        count = ref.get_num()
    except Exception as exc:  # noqa: BLE001
        return None, "ref_skeleton not iterable: {}".format(exc)

    bones = []
    for i in range(count):
        try:
            parent = ref.get_parent_index(i)
            pname = str(ref.get_bone_name(parent)) if parent >= 0 else None
        except Exception:  # noqa: BLE001
            pname = None
        bones.append(
            {
                "index": i,
                "name": str(ref.get_bone_name(i)),
                "parent": pname,
            }
        )
    return bones, None


def collect_socket_owner(skeleton):
    """Which mesh advertises this skeleton, so we can read the full ref pose."""
    try:
        return str(skeleton.get_editor_property("skeletal_mesh_asset_skeleton"))
    except Exception:  # noqa: BLE001
        return None


def main():
    inventory = []
    for root in ROOTS:
        for path in unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False):
            asset_path = path.split(".")[0]
            data = unreal.EditorAssetLibrary.find_asset_data(asset_path)
            if not data.is_valid():
                continue
            inventory.append(
                {
                    "path": asset_path,
                    "name": str(data.asset_name),
                    "class": str(data.asset_class_path.asset_name),
                }
            )

    report = {"inventory": inventory, "skeletons": [], "meshes": []}

    for item in inventory:
        cls = item["class"]
        if cls not in ("Skeleton", "SkeletalMesh"):
            continue
        asset = unreal.load_asset(item["path"])
        if asset is None:
            continue

        entry = {"asset": item["path"], "class": cls, "name": item["name"]}

        if cls == "SkeletalMesh":
            bones, err = dump_bones_from_mesh(asset)
            if bones:
                entry["bones"] = bones
                entry["bone_count"] = len(bones)
            if err:
                entry["bone_error"] = err

            for prop in ("skeleton", "skeletal_mesh_asset_skeleton"):
                val, used = try_prop(asset, prop)
                if val is not None:
                    entry["skeleton_asset"] = val.get_path_name()
                    break

            try:
                entry["materials"] = [
                    str(m.material_slot_name)
                    for m in asset.get_editor_property("materials")
                ]
            except Exception as exc:  # noqa: BLE001
                entry["materials_error"] = str(exc)

            try:
                entry["lod_count"] = asset.get_editor_property("lod_settings").num_lods
            except Exception:  # noqa: BLE001
                pass

            report["meshes"].append(entry)

        else:  # Skeleton
            bones, err = bone_tree_of_skeleton(asset)
            if bones:
                entry["bone_tree"] = bones
                entry["bone_count"] = len(bones)
            if err:
                entry["bone_error"] = err

            owner = collect_socket_owner(asset)
            if owner:
                entry["owner_mesh"] = owner
            report["skeletons"].append(entry)

    with open(OUT, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2, ensure_ascii=False)
    unreal.log(
        "[dump_skeletons] inventory={} meshes={} skeletons={} -> {}".format(
            len(inventory), len(report["meshes"]), len(report["skeletons"]), OUT
        )
    )


try:
    main()
except Exception as exc:  # noqa: BLE001
    unreal.log("[dump_skeletons] EXCEPTION {}".format(exc))
    with open(OUT, "w", encoding="utf-8") as handle:
        json.dump({"error": str(exc)}, handle, indent=2, ensure_ascii=False)