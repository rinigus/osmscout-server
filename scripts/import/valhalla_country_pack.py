import glob
from shapely.geometry import MultiPolygon, Polygon

# directories used for searching for packages
valhalla_meta_dir = 'valhalla/packages_meta'
valhalla_packages_dir = 'valhalla/packages'

# taken from http://wiki.openstreetmap.org/wiki/Osmosis/Polygon_Filter_File_Python_Parsing
def parse_poly(fname):
    """ Parse an Osmosis polygon filter file.

        Accept a sequence of lines from a polygon file, return a shapely.geometry.MultiPolygon object.

        http://wiki.openstreetmap.org/wiki/Osmosis/Polygon_Filter_File_Format
    """
    in_ring = False
    coords = []

    lines = open(fname, 'r')
    
    for (index, line) in enumerate(lines):
        if index == 0:
            # first line is junk.
            continue
        
        elif index == 1:
            # second line is the first polygon ring.
            coords.append([[], []])
            ring = coords[-1][0]
            in_ring = True
        
        elif in_ring and line.strip() == 'END':
            # we are at the end of a ring, perhaps with more to come.
            in_ring = False
    
        elif in_ring:
            # we are in a ring and picking up new coordinates.
            ring.append(map(float, line.split()))
    
        elif not in_ring and line.strip() == 'END':
            # we are at the end of the whole polygon.
            break
    
        elif not in_ring and line.startswith('!'):
            # we are at the start of a polygon part hole.
            coords[-1][1].append([])
            ring = coords[-1][1][-1]
            in_ring = True
    
        elif not in_ring:
            # we are at the start of a polygon part.
            coords.append([[], []])
            ring = coords[-1][0]
            in_ring = True
    
    return MultiPolygon(coords)

def getsize(sname):
    f = open(sname, 'r')
    return int(f.read().split()[0])

def gettimestamp(sname):
    f = open(sname + ".timestamp", 'r')
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
             "size": str(size),
             "size-compressed": str(size_compressed) }

if __name__ == '__main__':
    print country_pack('hierarchy/europe/estonia/poly')
