#!/bin/bash

set -e

VERSION=1
PG=distribution/postal/global
PC=distribution/postal/countries
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
