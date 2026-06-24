#!/usr/bin/env python3

from datetime import datetime
from pathlib import Path
from doit.tools import run_once
import json


PLANET_TILES = Path("valhalla/tiles")
TILES_TIMESTAMP = PLANET_TILES / "timestamp"
PACKAGES_DIR = Path("valhalla/packages")
PACKAGES_META = Path("valhalla/packages_meta")
TILE_PACKAGES_CONF = Path("valhalla/tiles_in_packages.json")
VERSION = "2"
ACCEPTABLE_PACKAGE_SIZE = 30 * 1024 * 1024  # in bytes

##############################################
# testing of package sizes:
# acceptable : number of packages
#     0           16768
#    10            3222
#    25            1829
#    30            1585
#    35            1427
#    40            1277
#    50            1060
#   100             643
##############################################


def load_packages():
    with TILE_PACKAGES_CONF.open("r") as f:
        return json.load(f)


def walkfiles(packages, tiles):
    for curr in packages:
        for filename in curr["files"]:
            if filename not in tiles:
                fname = PLANET_TILES / filename
                fullname = None
                if fname.exists():
                    fullname = fname
                elif fname.with_name(fname.name + ".gz").exists():
                    fullname = fname.with_name(fname.name + ".gz")

                if fullname is not None:
                    tiles[filename] = {
                        "file": fullname,
                        "size": fullname.stat().st_size,
                    }
                else:
                    tiles[filename] = None

        if "children" in curr:
            walkfiles(curr["children"], tiles)


def getsize(package, tiles):
    return sum(tiles[i]["size"] for i in package["files"] if tiles[i] is not None)


def package_outputs(package_id):
    tarname = PACKAGES_DIR / f"{package_id}.tar"
    return [
        tarname.with_name(tarname.name + ".bz2"),
        tarname.with_name(tarname.name + ".size"),
        tarname.with_name(tarname.name + ".size-compressed"),
        tarname.with_name(tarname.name + ".timestamp"),
        tarname.with_name(tarname.name + ".version"),
        PACKAGES_META / f"{package_id}.bbox",
    ]


def make_manifest(packages, tiles):
    manifest = []

    def makepacks(items):
        for curr in items:
            sz = getsize(curr, tiles)
            if sz == 0:
                continue

            if "children" in curr and sz > ACCEPTABLE_PACKAGE_SIZE:
                makepacks(curr["children"])
            else:
                package_id = len(manifest)
                files = [
                    tiles[i]["file"] for i in curr["files"] if tiles[i] is not None
                ]
                manifest.append(
                    {
                        "id": package_id,
                        "size": sz,
                        "files": files,
                        "bbox": curr["bbox"],
                    }
                )

    makepacks(packages)
    return manifest

PACKAGES = load_packages()
TILES = {}
walkfiles(PACKAGES, TILES)
MANIFEST = make_manifest(PACKAGES, TILES)


def task_pack_valhalla_tiles():
    for package in MANIFEST:
        package_id = package["id"]
        tarname = PACKAGES_DIR / f"{package_id}.tar"
        flist_name = tarname.with_name(tarname.name + ".list")
        polyname = PACKAGES_META / f"{package_id}.bbox"
        targets = package_outputs(package_id)

        def clear_targets(targets):
            for target in targets:
                Path(target).unlink(missing_ok=True)

        def write_list(files=package["files"], flist_name=flist_name):
            flist_name.write_text("".join(f"{filename}\n" for filename in files))

        def write_bbox(bbox=package["bbox"], polyname=polyname):
            polyname.write_text(" ".join(str(i) for i in bbox) + "\n")

        yield {
            "name": str(package_id),
            "file_dep": package["files"] + [TILES_TIMESTAMP],
            "targets": targets,
            "actions": [
                clear_targets,
                write_list,
                f"tar cf {tarname} -T {flist_name} {flist_name} {TILES_TIMESTAMP}",
                f"./pack.sh {tarname} {VERSION}",
                write_bbox,
            ],
            "clean": True,
        }


def task_timestamp():
    def write_timestamp():
        TILES_TIMESTAMP.write_text(datetime.now().strftime("%Y-%m-%d_%H:%M") + "\n")

    return {
        "actions": [write_timestamp],
        "targets": [TILES_TIMESTAMP],
        "uptodate": [run_once],
    }
