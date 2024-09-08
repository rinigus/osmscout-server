#!/usr/bin/env python3

import argparse
import os, glob, shutil

parser = argparse.ArgumentParser(description='Pack MBTiles into  OSM Scout Server sub-packages')

parser.add_argument('--tiles', default="mapbox/tiles", help='Folder with MBTiles split earlier into tiles')
parser.add_argument('--packages', default="mapbox/packages", help='Folder to export the packages')
parser.add_argument('--meta', default="mapbox/packages_meta", help='Folder to save packages metadata')

args = parser.parse_args()

planet_tiles = args.tiles
packages_dir = args.packages
packages_meta = args.meta
packages_timestamp = packages_dir + "/timestamp"
version = "3"

os.system("rm -rf %s %s" % (packages_dir, packages_meta))
for d in [packages_dir, packages_meta]:
    os.makedirs(d, exist_ok=True)

os.system("date +'%%Y-%%m-%%d_%%H:%%M' > %s" % packages_timestamp)

for tile in glob.glob(planet_tiles + "/*sqlite"):
    dname, bname = os.path.split(tile)
    bbox = os.path.join(dname, bname + ".bbox")
    shutil.copy(tile, packages_dir)
    if os.path.exists(bbox): shutil.copy(bbox, packages_meta)
    cmd = "./pack.sh " + os.path.join(packages_dir, bname) + " " + version
    print(cmd)
    os.system(cmd)

print("Made Mapbox GL packages")
