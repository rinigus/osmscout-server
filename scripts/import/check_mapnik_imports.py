#!/usr/bin/env python
import sys, json

cutoff = 0.5

if len(sys.argv) != 2:
    print "Check for Mapnik imports by comparing them with the corresponding libosmscout immport size"
    print "\nCall with JSON given as an argument\n"
    sys.exit(1)

json_fname = sys.argv[1]
print "Checking using", json_fname

alldata = json.load( open(json_fname,"r") )
issues = ""
for k in alldata:
    obj = alldata[k]
    if k != "url" and "mapnik_country" in obj and "osmscout" in obj:
        try:
            szMapnik = float( obj["mapnik_country"]["size"] )
            szScout = float( obj["osmscout"]["size"] )

            if szMapnik / szScout < cutoff:
                print "Check:", k, "Mapnik: %0.1fMB  /  Scout: %0.1fMB" % ( szMapnik / 1024./1024., szScout / 1024. / 1024.)
                issues += k + "* "
                
        except:
            print "Failed to parse:", k
            print obj
            sys.exit(-2)

print "\nIssues found:"
print issues
print
