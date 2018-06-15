#!/usr/bin/env python3

# Run this script from project root directory

import json

tag_alias_json = "data/geocoder-npl-tag-aliases.json"
geocoder_style = "src/geocoder-nlp/importer/stylesheet/map.ost"

tags = json.load(open(tag_alias_json, 'r'))

alltags = set()
for lang, vals in tags['tag2alias'].items():
    for tag,alias in vals.items():
        alltags.add(tag)

gd = open(geocoder_style, 'r', encoding='latin-1').read()

missing = []
for t in alltags:
    if gd.find(t) < 0:
        missing.append(t)

missing.sort()
for m in missing:
    kv = m.split('_',1)

    print('\nMissing:', m, 'https://taginfo.openstreetmap.org/tags/%s=%s' % (kv[0],kv[1]))

    print('''
  TYPE %s
    = NODE AREA ("%s"=="%s")
      {Name, NameAlt}
      ADDRESS POI
      GROUP %s\n
    ''' % (m, kv[0], kv[1], kv[0]))
