#!/usr/bin/env python3

import argparse
import json
import math
import mercantile
import numpy as np
import os
import sqlite3
import sys
import psutil

from collections import namedtuple, defaultdict

from hierarchy import Hierarchy
from shapely.geometry import Polygon
from shapely.io import from_geojson


# Note that files are formed according to XYZ notation while tiles
# inside MBTiles are stored in TMS

TileInfo = namedtuple('TileInfo', 'zoom, column, row, data_id')

class TileInfoArray:
    minZ = 0
    maxZ = 14

    def __init__(self):
        ntiles = 0
        n = 1
        for z in range(TileInfoArray.minZ, TileInfoArray.maxZ+1):
            ntiles += n
            n *= 4
        self._data = np.empty((ntiles, 4), dtype=int)
        self._n = 0

    def __iter__(self):
        self._iter_n = 0
        return self

    def __next__(self):
        if self._iter_n >= self._n:
            raise StopIteration
        ti = TileInfo(*[int(i) for i in self._data[self._iter_n,:]])
        self._iter_n += 1
        return ti

    def append(self, row):
        self._data[self._n, :] = row #'zoom, column, row, data_id'
        self._n += 1

    def data_ids(self):
        return self._data[:self._n, 3]

    def zoom_range(self):
        return int(min(self._data[:, 0])), int(max(self._data[:, 0]))


def ti2mt(ti):
    y = (2 ** ti.zoom) - ti.row - 1
    return mercantile.Tile(x=ti.column, y=y, z=ti.zoom)

def tri2mt(tri):
    column, row, zoom = tri[0], tri[1], tri[2]
    y = (2 ** zoom) - row - 1
    return mercantile.Tile(x=column, y=y, z=zoom)

def mt2bnd(mt):
    b = mercantile.bounds(mt)
    coors = [b.west, b.south, b.east, b.north]
    poly = Polygon( ( (coors[0], coors[1]), (coors[0], coors[3]),
                      (coors[2], coors[3]), (coors[2], coors[1]) ) )
    return poly, [b.west, b.south, b.east, b.north]

def splitter(ti):
    # determine into which file will the tile will go
    if ti.zoom < SplitLevel:
        return (0, 0, 0) #mercantile.Tile(0, 0, 0)
    f = 2**(ti.zoom - SplitLevel)
    return (ti.column//f, ti.row//f, SplitLevel)

def fname(mt):
    if mt.z==0:
        f = 'tiles-world'
    else:
        f = f'tiles-section-{mt.z}-{mt.x}-{mt.y}'
    return os.path.join(OutputDir, f + '.sqlite')

def process_memory():
    process = psutil.Process(os.getpid())
    mem_info = process.memory_info()
    return mem_info.rss

def mem_report(tag, mem_after, mem_before = 0):
    delta = (mem_after - mem_before)/1024/1024
    print(f"Consumed memory {tag}: {delta:.1f}MB")

def intersects(polys, b):
    for p in polys:
        if p.intersects(b):
            return True
    return False

## MAIN ##
parser = argparse.ArgumentParser(description='Split MBTiles into smaller sub-packages.')

parser.add_argument('mbtiles', help='Planet MBTiles')
parser.add_argument('hierarchy', help='Hierarchy with POLY files')
parser.add_argument('output', help='Output directory')
parser.add_argument('--split-level', type=int, default=7, help='Zoom level at which to split')

args = parser.parse_args()

SplitLevel = args.split_level
OutputDir = args.output

tofile = defaultdict(TileInfoArray)

# Load countries of interest
polys = []
for root, folders, files in os.walk(args.hierarchy):
    if "name" in files and not Hierarchy.ignore(root):
        name = Hierarchy.get_full_name(root)
        poly = root + "/poly.json"
        #print(Hierarchy.get_id(root), name)
        cp = from_geojson(open(poly, 'r').read())
        polys.append(cp)
print('Loaded hierarchy with {n} POLY.JSON files'.format(n=len(polys)))

# access to db
print('Opening', args.mbtiles)
conn = sqlite3.connect(args.mbtiles)
cur = conn.cursor()

# Load schema
schema = []
for row in cur.execute('select sql from sqlite_master'):
    schema.append(row[0])

r = cur.execute('select min(zoom_level) as minz, max(zoom_level) as maxz from tiles_shallow').fetchone()
TileInfoArray.minZ, TileInfoArray.maxZ = SplitLevel, r[1]
print(f'Tiles zoom range: {r[0]} - {r[1]}')

# Load tiles indexes: holder for metadata
mem_before = process_memory()
t = TileInfoArray()
mem_after = process_memory()
mem_report("One array:", mem_after, mem_before)
for row in cur.execute('select zoom_level, tile_column, tile_row, tile_data_id from tiles_shallow '+
                       'order by zoom_level,tile_column,tile_row'):
    ti = TileInfo._make(row)
    tofile[splitter(ti)].append(ti)
mem_after = process_memory()
mem_report("during loading stage:", mem_after, mem_before)

filtered = dict()
for tri,v in tofile.items():
    k = tri2mt(tri)
    b, _ = mt2bnd(k)
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
    _, bnd = mt2bnd(k)
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
        min_z, max_z = tiles.zoom_range()
        cur_out.execute('INSERT INTO metadata(name, value) VALUES(?, ?)',
                        ('minzoom', min_z))
        cur_out.execute('INSERT INTO metadata(name, value) VALUES(?, ?)',
                        ('maxzoom', max_z))
        for r in cur.execute('SELECT name, value FROM metadata'):
            if r[0] not in ['bounds', 'center', 'minzoom', 'maxzoom']:
                cur_out.execute('INSERT INTO metadata(name, value) VALUES(?, ?)',
                                (r[0], r[1]))

        # copy tile data
        data_ids = list(set(tiles.data_ids()))
        data_ids.sort()
        for i in data_ids:
            r = cur.execute('SELECT tile_data_id, tile_data FROM tiles_data WHERE tile_data_id=:id',
                            dict(id=int(i))).fetchone()
            cur_out.execute('INSERT INTO tiles_data(tile_data_id, tile_data) VALUES(?, ?)',
                            (r[0], r[1]))

        # copy tile indexes
        for t in tiles:
            cur_out.execute('INSERT INTO tiles_shallow(zoom_level, tile_column, tile_row, tile_data_id) ' +
                            'VALUES(?, ?, ?, ?)', t)

    # record bbox
    if f.find('world') < 0:
        with open(f + ".bbox", "w") as fb:
            fb.write(' '.join([str(b) for b in bnd]))
