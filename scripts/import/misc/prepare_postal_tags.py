#!/usr/bin/env python

import os, pycountry, codecs, sys

base_dir = "hierarchy"

##########################################################################
# comments:

# Georgia (Europe) has to be inserted manually. Otherwise there is a
# conflict with US state

##########################################################################

Name2Code = {
    "ireland-and-northern-ireland": "GB-IE",
    "haiti-and-domrep": "HT-DO",
    "senegal-and-gambia": "SN-GM",
    "gcc-states": "BH-KW-OM-QA-SA-AE",
    "israel-and-palestine": "IL-PS",
    "malaysia-singapore-brunei": "MY-SG-BN",

    "azores": "PT",
    "bosnia-herzegovina": "BA",
    "great-britain": "GB",
    "kosovo": "RS", # in libpostal dataset
    "lesotho": "ZA", # in libpostal dataset
    "macedonia": "MK",
    "russia": "RU",
    "canary-islands": "ES",
    "cape-verde": "CV",
    "comores": "KM",
    "congo-brazzaville": "CG",
    "congo-democratic-republic": "CD",
    "guinea-bissau": "GW",
    "ivory-coast": "CI",
    "saint-helena-ascension-and-tristan-da-cunha": "SH",
    "iran": "IR",
    "north-korea": "KP",
    "south-korea": "KR",
    "syria": "SY",
    "vietnam": "VN",
}

#####################################################

def ignore(d):
    return os.path.exists(d + "/ignore")

def get_from_parent(d, fname):
    dirs = d.split("/")
    base = base_dir.split("/")
    i = len(dirs)-1
    while i > len(base):
        k = ""
        for j in range(i): k = os.path.join(k, dirs[j])
        k = os.path.join(k, fname)
        if os.path.exists(k):
            return k
        i -= 1
    return None

def has_postal(d):
    if os.path.exists(os.path.join(d, "postal_country")):
        return "MINE"
    if get_from_parent(d, "postal_country") is not None:
        return "PARENT"
    return None

#####################################################
## MAIN

for root, folders, files in os.walk(base_dir):
    if "name" in files and not ignore(root) and has_postal(root) != "MINE":
        name = codecs.open(os.path.join(root, "name"), 'r', "utf-8").read().strip()
        basename = root.split("/")[-1]
        if basename in Name2Code:
            code = Name2Code[basename]
        else:
            try:
                code = pycountry.countries.lookup(name).alpha_2
            except:
                code = None

        print code, name
        if code is not None:
            f = open(os.path.join(root,"postal_country"), "w")
            f.write(code)
            f.close()
        else:
            if has_postal(root) is None:
                print "Cannot tag", root, name
                sys.exit(-1)
