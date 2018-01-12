import glob
from poly import parse_poly
from shapely.geometry import Polygon

# directories used for searching for packages
mapbox_meta_dir = 'mapbox/packages_meta'
mapbox_packages_dir = 'mapbox/packages'
mapbox_tiles_timestamp = "mapbox/packages/timestamp"
#version = "1"

def getsize(sname):
    f = open(sname, 'r')
    return int(f.read().split()[0])

def getversion(pname):
    f = open(pname + ".version", 'r')
    return f.read().split()[0]

def gettimestamp(sname):
    f = open(mapbox_tiles_timestamp, 'r')
    return f.read().split()[0]

def getpackname(sname):
    return sname[len("tiles-section-"):-len(".sqlite")]

# call with the name of POLY filename
def country_pack(country_poly_fname):
    country = parse_poly(country_poly_fname)
    packs = []
    size_compressed = 0
    size = 0
    ts = None
    for bbox in glob.glob(mapbox_meta_dir + "/*.bbox"):
        coors = []
        for i in open(bbox, 'r'):
            for k in i.split():
                coors.append(float(k))

        poly = Polygon( ( (coors[0], coors[1]), (coors[0], coors[3]),
                          (coors[2], coors[3]), (coors[2], coors[1]) ) )

        if country.intersects(poly):
            pname = bbox[len(mapbox_meta_dir)+1:-len(".bbox")]
            packs.append(getpackname(pname))
            pdata = mapbox_packages_dir + "/" + pname
            size_compressed += getsize(pdata + '.size-compressed')
            size += getsize(pdata + '.size')
            ts = gettimestamp(pdata)
            version = getversion(pdata)
            
    return { "path": "mapboxgl/packages", # generic path
             "packages": packs,
             "timestamp": ts,
             "version": version,
             "size": str(size),
             "size-compressed": str(size_compressed) }

def world_pack():
    pname = "tiles-world.sqlite"
    pdata = mapbox_packages_dir + "/" + pname
    return { "path": "mapboxgl/packages", # generic path, tiles-world.sqlite is specified as requested files in the mapmanagerfeature
             "timestamp": gettimestamp(pdata),
             "version": getversion(pdata),
             "size": str(getsize(pdata + '.size')),
             "size-compressed": str(getsize(pdata + '.size-compressed')) }
    

if __name__ == '__main__':
    print country_pack('hierarchy/europe/poly')
    print
    print world_pack()
