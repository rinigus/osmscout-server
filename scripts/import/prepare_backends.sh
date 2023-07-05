#!/bin/bash

set -e

echo "Install backends used by OSM Scout Server maps importer"

BNDSRC=`pwd`/backends-source
BNDINSTALL=`pwd`/backends-install

# sources are deleted, but not install. this is to prevent libpostal
# dataset downloads when its not needed
rm -rf $BNDSRC
mkdir -p $BNDSRC
rm $BNDINSTALL/bin/geocoder-importer || true
rm $BNDINSTALL/bin/liboscmscout-version || true
rm $BNDINSTALL/mapnik-styles-sqlite || true
rm $BNDINSTALL/bin/poly2geojson || true
rm -rf $BNDINSTALL/share/geocoder-nlp || true

# libpostal: version specified by commit id
# git clone https://github.com/openvenues/libpostal $BNDSRC/libpostal
# ( cd $BNDSRC/libpostal && git checkout 7f7aada32ab1a65b94f880a45f9755bbd941eedc )

git clone https://github.com/rinigus/libpostal $BNDSRC/libpostal
( cd $BNDSRC/libpostal && git checkout 17eb4a63356aa1cc9aa073635f4952d6d4c2c54b )
( cd $BNDSRC/libpostal && ./bootstrap.sh && ./configure --datadir=$BNDINSTALL --prefix=$BNDINSTALL && make -j3 && make install )

# geocoder-nlp
git clone --recursive https://github.com/rinigus/geocoder-nlp.git $BNDSRC/geocoder-nlp
(cd $BNDSRC/geocoder-nlp && mkdir build && cd build && PKG_CONFIG_PATH=$BNDINSTALL/lib/pkgconfig cmake -DCMAKE_INSTALL_PREFIX=$BNDINSTALL .. && make -j8 install)
(cd $BNDSRC/geocoder-nlp && mkdir -p $BNDINSTALL/share/geocoder-nlp && ln -s `pwd`/importer/data $BNDINSTALL/share/geocoder-nlp/)

# poly2geojson
wget -O $BNDINSTALL/bin/poly2geojson https://github.com/frafra/poly2geojson/releases/download/v0.1.2/poly2geojson-v0.1.2-linux-x64
chmod +x $BNDINSTALL/bin/poly2geojson

# libosmscout version tool
#(cd libosmscout-version-tool && make clean && make INCLUDE=-I$BNDINSTALL/include LIBRARIES=-I$BNDINSTALL/lib && ln -s `pwd`/liboscmscout-version $BNDINSTALL/bin)

## mapnik-styles-sqlite
#git clone https://github.com/rinigus/mapnik-styles-sqlite.git $BNDSRC/mapnik-styles-sqlite
#ln -s $BNDSRC/mapnik-styles-sqlite $BNDINSTALL
#(cd $BNDSRC/mapnik-styles-sqlite/importer && ./install_twkb.sh)

## gdal - required for mapnik import (working version, 2.3.0, 2.3.1, 2.4.0 have led to missing buildings
#(cd $BNDSRC && rm gdal-*.tar.gz || echo No old gdal file)
#(cd $BNDSRC && wget http://download.osgeo.org/gdal/2.2.4/gdal-2.2.4.tar.gz && tar xvf gdal-2.2.4.tar.gz )
#(cd $BNDSRC/gdal-2.2.4 && ./configure --prefix=$BNDINSTALL --with-spatialite=yes && make -j16 && make install )

