#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import os, json
from hierarchy import Hierarchy
from mapbox_country_pack import country_pack as mapbox_pack
from valhalla_country_pack import country_pack as valhalla_pack

parser = argparse.ArgumentParser(description='Collect data for finalizing')

parser.add_argument('--output', default="./", help='Folder for storing generated data')
args = parser.parse_args()

output_folder = args.output

def spath(name):
    return name.replace('/','-')

########### Main loop #############
provided_countries = {}

for root, folders, files in os.walk(Hierarchy.base_dir):
    if "name" in files and not Hierarchy.ignore(root):
        name = Hierarchy.get_full_name(root)
        polyjson = root + "/poly.json"
        print(Hierarchy.get_id(root), name, Hierarchy.get_postal(root), Hierarchy.get_postcodes(root))

        cid = Hierarchy.get_id(root)
        provided_countries[cid] = { "id": cid,
                                    "type": "territory",
                                    "name": Hierarchy.get_full_name(root),
                                    "postal_country": { "path": "postal/countries-v1/" + Hierarchy.get_postal(root) },
                                    # "osmscout": { "path": "osmscout/" + spath(cid) },
                                    "geocoder_nlp": { "path": "geocoder-nlp/" + spath(cid) },
                                    "mapnik_country": { "path": "mapnik/countries/" + spath(cid) },
                                    "mapboxgl_country": mapbox_pack(polyjson),
                                    "valhalla": valhalla_pack(polyjson),
        }

# save provided countries
fjson = open(os.path.join(output_folder, "countries.json"), "w")
fjson.write( json.dumps( provided_countries, sort_keys=True, indent=4, separators=(',', ': ')) )
