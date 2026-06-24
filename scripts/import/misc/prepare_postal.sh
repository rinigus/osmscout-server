#!/bin/bash

set -e

VERSION=2
PG=distribution/postal/global-v1
PC=distribution/postal/countries-v1
BNDINSTALL=`pwd`/backends-install
POSCOUNTRY=postal-country

# global
mkdir -p $PG
for i in "address_expansions" "language_classifier" "numex" "transliteration"; do
    rsync -av $BNDINSTALL/libpostal/$i $PG/
done
./pack.sh $PG $VERSION

# country
rsync -av $POSCOUNTRY/ $PC
for i in $PC/*; do
    ./pack.sh $i $VERSION
done
