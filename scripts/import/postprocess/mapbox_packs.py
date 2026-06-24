#!/usr/bin/env python3

from pathlib import Path
from datetime import datetime
import shutil


PLANET_TILES = Path("mapbox/tiles")
PACKAGES_DIR = Path("mapbox/packages")
PACKAGES_META = Path("mapbox/packages_meta")
PACKAGES_TIMESTAMP = PACKAGES_DIR / "timestamp"
VERSION = "3"


def package_targets(tile):
    package = PACKAGES_DIR / tile.name
    targets = [
        package.with_name(package.name + ".bz2"),
        package.with_name(package.name + ".size"),
        package.with_name(package.name + ".size-compressed"),
        package.with_name(package.name + ".timestamp"),
        package.with_name(package.name + ".version"),
    ]

    bbox = tile.with_name(tile.name + ".bbox")
    if bbox.exists():
        targets.append(PACKAGES_META / bbox.name)

    return targets


def pack_tile(tile):
    bname = tile.name
    package = PACKAGES_DIR / bname
    bbox = tile.with_name(tile.name + ".bbox")
    package_bbox = PACKAGES_META / bbox.name

    def clear_targets():
        for target in targets:
            Path(target).unlink(missing_ok=True)

    def copy_inputs():
        shutil.copy2(tile, package)
        if bbox.exists():
            shutil.copy2(bbox, package_bbox)

    file_dep = [tile]
    targets = package_targets(tile)

    if bbox.exists():
        file_dep.append(bbox)

    return {
        "name": bname,
        "file_dep": file_dep,
        "targets": targets,
        "actions": [clear_targets, copy_inputs, f"./pack.sh {package} {VERSION}"],
        "clean": True,
    }


def task_pack_mapbox_tiles():
    for tile in sorted(PLANET_TILES.glob("*.sqlite")):
        task = pack_tile(tile)
        yield task


def task_timestamp():
    def write_timestamp():
        PACKAGES_TIMESTAMP.write_text(datetime.now().strftime("%Y-%m-%d_%H:%M") + "\n")

    tiles = sorted(PLANET_TILES.glob("*.sqlite"))

    return {
        "actions": [write_timestamp],
        "file_dep": [target for tile in tiles for target in package_targets(tile)],
        "targets": [PACKAGES_TIMESTAMP],
    }
