#!/bin/bash

set -e

echo "Convert all POLY files in the hierarchy into GeoJSON format"

P2J=backends-install/bin/poly2geojson

for i in $(find hierarchy -name poly);
do
    echo Converting $i
    $P2J < $i > $i.json
done

    
