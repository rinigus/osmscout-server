#!/usr/bin/env python
import sys, json, tabulate

cutoff_less = 0.8
cutoff_more = 1.3

json_reference_fname = "provided/countries_provided.reference.json"
json_tocheck_fname = "provided/countries_provided.json"

alldata = json.load( open(json_tocheck_fname,"r") )
reference = json.load( open(json_reference_fname,"r") )
issues = ""
header = ["country", "backend", "reference", "tested", "ratio"]
missing = []
for k in alldata:
    if k != "url":
        obj = alldata[k]
        try:
            ref = reference[k]
        except:
            print "Missing reference for", k
            continue
        for j in obj:
            if 'size' in obj[j]:
                o = float(obj[j]['size'])
                r = float(ref[j]['size'])

                if o < cutoff_less*r or o > cutoff_more*r:
                    missing.append([k, j, r, o, o/r])

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
