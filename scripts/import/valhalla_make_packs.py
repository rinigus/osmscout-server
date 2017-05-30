import os

planet_tiles = "valhalla/tiles"
tiles_timestamp = planet_tiles + "/timestamp"
packages_dir = "valhalla/packages"
dir_meta = "valhalla/packages_meta"
tile_packages_conf = "valhalla/tiles_in_packages"
version = "1"

os.system("rm -rf %s %s" % (packages_dir, dir_meta))
for d in [packages_dir, dir_meta]:
    if not os.path.exists(d):
        os.makedirs(d)

os.system("date +'%%Y-%%m-%%d_%%H:%%M' > %s" % tiles_timestamp)
    
packages = []
for i in open(tile_packages_conf):
    packages.append([i.split(":")[0].split(), i.split(":")[1].split()])

counter = 0
for pack in packages:
    tiles = pack[1]
    totar = []
    for t in tiles:
        fname = os.path.join( planet_tiles, t )
        if os.path.exists(fname): totar.append(fname)

    if len(totar) > 0:
        tarname = os.path.join(packages_dir, "%d.tar" % counter)
        polyname = os.path.join(dir_meta, "%d.bbox" % counter)
        cmd = "tar cf " + tarname
        flist_name = tarname + ".list"
        flist = open(flist_name, "w")        
        for i in totar:
            cmd += " " + i
            flist.write(i + "\n")
        flist.close()
        cmd += " " + flist_name + " " + tiles_timestamp
        cmd += " && ./pack.sh " + tarname + " " + version
        print cmd
        os.system(cmd)

        fpoly = open(polyname, "w")
        for i in pack[0]:
            fpoly.write(i)
            fpoly.write(" ")
        fpoly.write("\n")
        fpoly.close()
        
        counter += 1

print "Made packages:", counter
os.system("rm  %s/*.list" % packages_dir)
