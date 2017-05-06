#!/usr/bin/env python

# This script prepares files before uploading them for distribution
# This has to be run after all imports are finished

import json, pickle, os, stat

root_dir = "distribution"
bucket = open("bucket_name", "r").read().strip()
url_base = "http://data.modrana.org/osm_scout_server"
#url_base = "https://kuqrhldx.e24files.com"

url_specs = {
    "base": url_base,
    "type": "url",
    "osmscout": "osmscout-5",
    "geocoder_nlp": "geocoder-nlp-5",
    "postal_global": "postal-global-1",
    "postal_country": "postal-country-1",
    "mapnik_global": "mapnik-global-1",
    "mapnik_country": "mapnik-country-3",
}

dist = json.loads( open("countries.json", "r").read() )

dist["postal/global"] = {
    "id": "postal/global",
    "type": "postal/global",
    "postal_global": { "path": "postal/global" }
    }

dist["mapnik/global"] = {
    "id": "mapnik/global",
    "type": "mapnik/global",
    "mapnik_global": { "path": "mapnik/global" }
    }

dist["url"] = url_specs

# could make it smarter in future to check whether the files have
# changed since the last upload
toupload = []
upload_commands = "#!/bin/bash\nset -e\nrm -f digest.md5\n"
def uploader(dirname, targetname, extra="/"):
    global toupload, upload_commands
    toupload.append([dirname, targetname])
    upload_commands += "echo\necho " + dirname + "\n"
    sd = dirname.replace("/", "\/")
    st = targetname.replace("/", "\/")
    upload_commands += "md5deep -t -l -r " + dirname + " | sed 's/%s/%s/g' >> digest.md5\n" % (sd,st)
    upload_commands += "s3cmd --config=.s3cfg sync " + dirname + extra + " s3://" + bucket + "/" + targetname + extra + " --acl-public --signature-v2 " + "\n"

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
fjson.close()

uploader("provided/countries_provided.json", "countries_provided.json", extra = "")

upload_commands += "bzip2 -f digest.md5\n"
uploader("digest.md5.bz2", "digest.md5.bz2", extra = "")
upload_commands += "echo\necho 'Set S3 permissions'\n"
upload_commands += "s3cmd --config=.s3cfg setacl s3://" + bucket + "/ --acl-public --recursive\n"
upload_commands += "mv digest.md5 digest.md5.bz2.md5\n"
uploader("digest.md5.bz2.md5", "digest.md5.bz2.md5", extra = "")

# save uploader script
fscript = open("uploader.sh", "w")
fscript.write( upload_commands )

fscript.write( "echo\necho 'Set S3 permissions'\n" )
fscript.write( "s3cmd --config=.s3cfg setacl s3://" + bucket + "/ --acl-public --recursive\n" )
fscript.write( "s3cmd --config=.s3cfg setacl s3://" + bucket + "/ --acl-private\n" )

fscript.close()

st = os.stat('uploader.sh')
os.chmod('uploader.sh', st.st_mode | stat.S_IEXEC)

print "Check uploader script and run it"

