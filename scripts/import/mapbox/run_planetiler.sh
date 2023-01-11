#!/usr/bin/env bash

set -e

cd "$(dirname "$(readlink -f "$0")")"
cd planetiler

# from https://github.com/onthegomap/planetiler/blob/main/PLANET.md

echo Remove old data
rm output.mbtiles || echo No output.mbtiles found
rm -rf data && mkdir data

java -Xmx110g \
  `# return unused heap memory to the OS` \
  -XX:MaxHeapFreeRatio=40 \
  -jar planetiler.jar \
  `# Download the latest planet.osm.pbf from s3://osm-pds bucket` \
  --area=planet --bounds=planet --download \
  `# Accelerate the download by fetching the 10 1GB chunks at a time in parallel` \
  --download-threads=10 --download-chunk-size-mb=1000 \
  `# Also download name translations from wikidata` \
  --fetch-wikidata \
  --mbtiles=output.mbtiles \
  `# Store temporary node locations in memory` \
  --nodemap-type=array --storage=ram
