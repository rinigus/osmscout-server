import glob
from poly import parse_poly
from shapely.geometry import Polygon

# directories used for searching for packages
valhalla_meta_dir = 'valhalla/packages_meta'
valhalla_packages_dir = 'valhalla/packages'
valhalla_tiles_timestamp = "valhalla/tiles/timestamp"
version = "1"

def getsize(sname):
    f = open(sname, 'r')
    return int(f.read().split()[0])

def gettimestamp(sname):
    f = open(valhalla_tiles_timestamp, 'r')
    return f.read().split()[0]

# call with the name of POLY filename
def country_pack(country_poly_fname):
    country = parse_poly(country_poly_fname)
    packs = []
    size_compressed = 0
    size = 0
    ts = None
    for bbox in glob.glob(valhalla_meta_dir + "/*.bbox"):
        coors = []
        for i in open(bbox, 'r'):
            for k in i.split():
                coors.append(float(k))

        poly = Polygon( ( (coors[0], coors[1]), (coors[0], coors[3]),
                          (coors[2], coors[3]), (coors[2], coors[1]) ) )

        if country.intersects(poly):
            pname = bbox[len(valhalla_meta_dir)+1:-len(".bbox")]
            packs.append(pname)
            pdata = valhalla_packages_dir + "/" + bbox[len(valhalla_meta_dir)+1:-len(".bbox")] + ".tar"
            size_compressed += getsize(pdata + '.size-compressed')
            size += getsize(pdata + '.size')
            ts = gettimestamp(pdata)

    return { "packages": packs,
             "timestamp": ts,
             "version": version,
             "size": str(size),
             "size-compressed": str(size_compressed) }

if __name__ == '__main__':
    print country_pack('hierarchy/europe/estonia/poly')
