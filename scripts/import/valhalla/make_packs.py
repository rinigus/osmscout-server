#!/usr/bin/env python3

import os, json

planet_tiles = "valhalla/tiles"
tiles_timestamp = planet_tiles + "/timestamp"
packages_dir = "valhalla/packages"
packages_meta = "valhalla/packages_meta"
tile_packages_conf = "valhalla/tiles_in_packages.json"

tiles_timestamp = planet_tiles + "/timestamp"
version = "2"
acceptable_package_size = 30*1024*1024 # in bytes

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

os.system("date +'%%Y-%%m-%%d_%%H:%%M' > %s" % tiles_timestamp)

packages = json.load(open(tile_packages_conf, 'r'))
tiles = {}

def walkfiles(o, t):
    for curr in o:
        for f in curr['files']:
            if f not in t:
                fname = os.path.join( planet_tiles, f )
                d = None
                fullname = None
                if os.path.exists(fname): fullname = fname
                elif os.path.exists(fname + ".gz"): fullname = fname + ".gz"

                if fullname is not None:
                    d = {}
                    d['file'] = fullname
                    d['size'] = os.path.getsize(fullname)
                t[f] = d
        if "children" in curr:
            walkfiles(curr["children"], t)

walkfiles(packages, tiles)

counter = 0
sizes = []
def getsize(o):
    sz = 0
    for i in o['files']:
        if tiles[i] is not None:
            sz += tiles[i]['size']
    return sz

def makepacks(o):
    global counter
    for curr in o:
        sz = getsize(curr)
        if sz == 0:
            continue

        if 'children' in curr and sz > acceptable_package_size:
            # packaging via children
            makepacks(curr['children'])
        else:
            print(counter, sz/1024./1024.)
            sizes.append(sz/1024./1024.)

            totar = []
            for i in curr['files']:
                if tiles[i] is not None:
                    totar.append(tiles[i]['file'])

            tarname = os.path.join(packages_dir, "%d.tar" % counter)
            polyname = os.path.join(packages_meta, "%d.bbox" % counter)
            cmd = "tar cf " + tarname
            flist_name = tarname + ".list"
            flist = open(flist_name, "w")
            for i in totar:
                #cmd += " " + i
                flist.write(i + "\n")
            flist.close()
            cmd += " -T " + flist_name + " " + flist_name + " " + tiles_timestamp
            cmd += " && ./pack.sh " + tarname + " " + version
            print(cmd)
            os.system(cmd)

            fpoly = open(polyname, "w")
            for i in curr['bbox']:
                fpoly.write(str(i))
                fpoly.write(" ")
            fpoly.write("\n")
            fpoly.close()

            counter += 1

makepacks(packages)

sizes.sort()
print('Size of all packages:', sum(sizes), 'MB. Number of packages:', len(sizes))
os.system("rm  %s/*.list" % packages_dir)
