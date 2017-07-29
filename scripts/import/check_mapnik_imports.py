#!/usr/bin/env python
import sys, json, tabulate

cutoff = 0.35

whitelist = { "north-america/us/region/pacific": 0.07,
              "africa/saint-helena-ascension-and-tristan-da-cunha": 0.03,
              "russia/far-eastern-fed-district/chukotka": 0.03,
              "africa/djibouti": 0.1,
              "australia-oceania/fiji": 0.2,
              "north-america/greenland": 0.06,
              "north-america/us/alaska": 0.1,
              "russia/far-eastern-fed-district/magadan": 0.1 }

if len(sys.argv) != 2:
    print "Check for Mapnik imports by comparing them with the corresponding libosmscout import size"
    print "\nCall with JSON given as an argument\n"
    sys.exit(1)

json_fname = sys.argv[1]
print "Checking using", json_fname, "\n"

alldata = json.load( open(json_fname,"r") )
issues = ""
header = ["country", "ratio", "mapnik", "osmscout"]
missing = []
for k in alldata:
    obj = alldata[k]
    if k != "url" and "mapnik_country" in obj and "osmscout" in obj:
        try:
            szMapnik = float( obj["mapnik_country"]["size"] )
            szScout = float( obj["osmscout"]["size"] )
            r = szMapnik / szScout
            if (k in whitelist and r < whitelist[k]) or (k not in whitelist and r < cutoff):
                missing.append( [k, szMapnik/szScout, szMapnik / 1024./1024., szScout / 1024. / 1024.] )
                issues += obj["mapnik_country"]["path"] + "* "
                
        except:
            print "Failed to parse:", k
            print obj
            sys.exit(-2)

missing.sort()

if len(missing) > 0:
    print tabulate.tabulate(missing, headers=header, floatfmt=".2f")
            
    print "\nIssues found:"
    print issues
    print

    print "Countries and territories only:"
    for k in missing: print k[0]
    print

    sys.exit(-10)

else:
    print "Coast is clear - no issues found\n"
