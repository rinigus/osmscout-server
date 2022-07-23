#!/usr/bin/env python3

import argparse
import json
import math
import mercantile
import os
import sqlite3
import sys
#import shapely.geometry as sg

from collections import namedtuple, defaultdict

from hierarchy import Hierarchy
from poly import parse_poly, intersects


# Note that files are formed according to XYZ notation while tiles
# inside MBTiles are stored in TMS

def ti2mt(ti):
    y = (2 ** ti.zoom) - ti.row - 1
    return mercantile.Tile(x=ti.column, y=y, z=ti.zoom)

def mt2bnd(mt):
    b = mercantile.bounds(mt)
    return [b.west, b.south, b.east, b.north]

def splitter(ti) -> str:
    # determine into which file will the tile will go
    mt = ti2mt(ti)
    if ti.zoom < SplitLevel:
        return mercantile.Tile(0, 0, 0)
    elif ti.zoom == SplitLevel:
        return mt
    return mercantile.parent(mt, zoom=SplitLevel)

def fname(mt):
    if mt.z==0:
        f = 'tiles-world'
    else:
        f = f'tiles-section-{mt.z}-{mt.x}-{mt.y}'
    return os.path.join(OutputDir, f + '.sqlite')


## MAIN ##
parser = argparse.ArgumentParser(description='Split MBTiles into smaller sub-packages.')

parser.add_argument('mbtiles', help='Planet MBTiles')
parser.add_argument('hierarchy', help='Hierarchy with POLY files')
parser.add_argument('output', help='Output directory')
parser.add_argument('--split-level', type=int, default=7, help='Zoom level at which to split')

args = parser.parse_args()

SplitLevel = args.split_level
OutputDir = args.output

# Load countries of interest
polys = []
for root, folders, files in os.walk(args.hierarchy):
    if "name" in files and not Hierarchy.ignore(root):
        name = Hierarchy.get_full_name(root)
        poly = root + "/poly"
        #print(Hierarchy.get_id(root), name)
        polys.append(parse_poly(poly))
print('Loaded hierarchy with {n} POLY files'.format(n=len(polys)))

# extra types and main dict
TileInfo = namedtuple('TileInfo', 'zoom, column, row, data_id')
tofile = defaultdict(list)

# access to db
print('Opening', args.mbtiles)
conn = sqlite3.connect(args.mbtiles)
cur = conn.cursor()

# Load schema
schema = []
for row in cur.execute('select sql from sqlite_master'):
    schema.append(row[0])

# Load tiles indexes
for row in cur.execute('select zoom_level, tile_column, tile_row, tile_data_id from tiles_shallow '+
                       'order by zoom_level,tile_column,tile_row'):
    ti = TileInfo._make(row)
    tofile[splitter(ti)].append(ti)

filtered = dict()
for k,v in tofile.items():
    b = mt2bnd(k)
    if intersects(polys, b):
        f = fname(k)
        print('Keeping', k, f)
        if os.path.exists(f):
            print(f'Export file {f} exists already. Please clear output directory to avoid clashes')
            sys.exit(1)

        filtered[k] = v

# save files
os.makedirs(OutputDir, exist_ok=True)
counter = 0
for k, tiles in filtered.items():
    f = fname(k)
    bnd = mt2bnd(k)
    center = [(bnd[0]+bnd[2])/2, (bnd[1]+bnd[3])/2]
    ready = 100.0*counter/len(filtered)
    counter += 1
    print(f'Exporting to {f} (done {ready:.1f}%)')
    with sqlite3.connect(f) as conn_out:
        cur_out = conn_out.cursor()
        for s in schema:
            cur_out.execute(s)

        # fill metadata
        cur_out.execute('INSERT INTO metadata(name, value) VALUES(?, ?)',
                        ('bounds', ','.join([str(b) for b in bnd])))
        cur_out.execute('INSERT INTO metadata(name, value) VALUES(?, ?)',
                        ('center', ','.join([str(b) for b in center])))
        zlevels = [t.zoom for t in tiles]
        cur_out.execute('INSERT INTO metadata(name, value) VALUES(?, ?)',
                        ('minzoom', min(zlevels)))
        cur_out.execute('INSERT INTO metadata(name, value) VALUES(?, ?)',
                        ('maxzoom', max(zlevels)))
        for r in cur.execute('SELECT name, value FROM metadata'):
            if r[0] not in ['bounds', 'center', 'minzoom', 'maxzoom']:
                cur_out.execute('INSERT INTO metadata(name, value) VALUES(?, ?)',
                                (r[0], r[1]))

        # copy tile data
        data_ids = list(set([t.data_id for t in tiles]))
        data_ids.sort()
        for i in data_ids:
            r = cur.execute('SELECT tile_data_id, tile_data FROM tiles_data WHERE tile_data_id=:id',
                            dict(id=i)).fetchone()
            cur_out.execute('INSERT INTO tiles_data(tile_data_id, tile_data) VALUES(?, ?)',
                            (r[0], r[1]))

        # copy tile indexes
        for t in tiles:
            cur_out.execute('INSERT INTO tiles_shallow(zoom_level, tile_column, tile_row, tile_data_id) ' +
                            'VALUES(?, ?, ?, ?)', t)
