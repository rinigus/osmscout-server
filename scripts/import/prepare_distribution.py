#!/usr/bin/env python

# This script prepares files before uploading them for distribution
# This has to be run after all imports are finished

import json, pickle, os, stat

root_dir = "distribution"
bucket = "kuqrhldx"
url_base = "https://" + bucket + ".e24files.com"

url_specs = {
    "base": url_base,
    "type": "url",
    "osmscout": "osmscout-1",
    "geocoder_nlp": "geocoder-nlp-1",
    "postal_global": "postal-global-1",
    "postal_country": "postal-country-1",
}

dist = json.loads( open("countries.json", "r").read() )

dist["postal/global"] = {
    "id": "postal/global",
    "type": "postal/global",
    "postal_global": { "path": "postal/global" }
    }

dist["url"] = url_specs

# could make it smarter in future to check whether the files have
# changed since the last upload
toupload = []
upload_commands = "#!/bin/bash\n"
def uploader(dirname, targetname):
    global toupload, upload_commands
    toupload.append([dirname, targetname])
    upload_commands += "echo\necho " + dirname + "\n"
    upload_commands += "s3cmd --config=.s3cfg sync " + dirname + "/ s3://" + bucket + "/" + targetname + "/ --acl-public " + "\n"

def getprop(dirname):
    props = {}
    for p in ["size", "size-compressed", "timestamp", "version"]:
        v = open(dirname + "." + p, "r").read().split()[0]
        props[p] = v
    return props

# fill database details
for d in dist:
    for sub in dist[d]:
        try:
            rpath = dist[d][sub]["path"]
            print rpath
        except:
            continue

        locdir = root_dir + "/" + rpath
        remotedir = url_specs[sub] + "/" + rpath

        dist[d][sub].update( getprop(locdir) )
        uploader(locdir, remotedir)


# save provided countries
fjson = open("provided/countries_provided.json", "w")
fjson.write( json.dumps( dist, sort_keys=True, indent=4, separators=(',', ': ')) )

# save uploader script
fscript = open("uploader.sh", "w")
fscript.write( upload_commands )
fscript.write( "s3cmd --config=.s3cfg setacl s3://" + bucket + "/ --acl-public --recursive\n" )
fscript.write( "s3cmd --config=.s3cfg setacl s3://" + bucket + "/ --acl-private\n" )
fscript.close()

st = os.stat('uploader.sh')
os.chmod('uploader.sh', st.st_mode | stat.S_IEXEC)

print "Check uploader script and run it"

