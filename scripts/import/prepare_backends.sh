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

# libpostal: version specified by commit id
# git clone https://github.com/openvenues/libpostal $BNDSRC/libpostal
# ( cd $BNDSRC/libpostal && git checkout 7f7aada32ab1a65b94f880a45f9755bbd941eedc )
git clone https://github.com/rinigus/libpostal $BNDSRC/libpostal
( cd $BNDSRC/libpostal && git checkout 17eb4a63356aa1cc9aa073635f4952d6d4c2c54b )
( cd $BNDSRC/libpostal && ./bootstrap.sh && ./configure --datadir=$BNDINSTALL --prefix=$BNDINSTALL && make -j3 && make install )

# libosmscout
git clone --recursive https://github.com/rinigus/libosmscout.git $BNDSRC/libosmscout
mkdir -p $BNDSRC/libosmscout/build
( cd $BNDSRC/libosmscout/build && cmake -DCMAKE_INSTALL_PREFIX:PATH=$BNDINSTALL -DBUILD_WITH_OPENMP=OFF -DOSMSCOUT_BUILD_DOC_API=OFF -DOSMSCOUT_BUILD_BINDING_JAVA=OFF -DOSMSCOUT_BUILD_MAP_OPENGL=OFF  -DOSMSCOUT_BUILD_MAP_AGG=OFF -DOSMSCOUT_BUILD_MAP_CAIRO=OFF -DOSMSCOUT_BUILD_MAP_SVG=OFF -DOSMSCOUT_BUILD_MAP_QT=OFF -DOSMSCOUT_BUILD_MAP_IOSX=OFF -DOSMSCOUT_BUILD_TESTS=OFF -DOSMSCOUT_BUILD_DEMOS=OFF -DOSMSCOUT_BUILD_BINDING_CSHARP=OFF -DOSMSCOUT_BUILD_CLIENT_QT=OFF -DOSMSCOUT_BUILD_TOOL_OSMSCOUT2=OFF -DOSMSCOUT_BUILD_TOOL_STYLEEDITOR=OFF -DGPERFTOOLS_USAGE=OFF -DOSMSCOUT_BUILD_TOOL_IMPORT=ON .. && make -j8 && make install )

# geocoder-nlp
git clone --recursive https://github.com/rinigus/geocoder-nlp.git $BNDSRC/geocoder-nlp
(cd $BNDSRC/geocoder-nlp/importer && ./install_deps.sh && make LIBPOSTAL_INCLUDE=-I$BNDINSTALL/include LIBPOSTAL_LIB="-L$BNDINSTALL/lib -lpostal" && ln -s -f `pwd`/import_pbf.sh $BNDINSTALL/bin/geocoder-importer-pbf && ln -s -f `pwd`/importer $BNDINSTALL/bin/geocoder-importer )

# libosmscout version tool
(cd libosmscout-version-tool && make clean && make INCLUDE=-I$BNDINSTALL/include LIBRARIES=-I$BNDINSTALL/lib && ln -s `pwd`/liboscmscout-version $BNDINSTALL/bin)

# mapnik-styles-sqlite
git clone https://github.com/rinigus/mapnik-styles-sqlite.git $BNDSRC/mapnik-styles-sqlite
ln -s $BNDSRC/mapnik-styles-sqlite $BNDINSTALL
(cd $BNDSRC/mapnik-styles-sqlite/importer && ./install_twkb.sh)

# gdal - required for mapnik import (working version, 2.3.0, 2.3.1, 2.4.0 have led to missing buildings
(cd $BNDSRC && rm gdal-*.tar.gz || echo No old gdal file)
(cd $BNDSRC && wget http://download.osgeo.org/gdal/2.2.4/gdal-2.2.4.tar.gz && tar xvf gdal-2.2.4.tar.gz )
(cd $BNDSRC/gdal-2.2.4 && ./configure --prefix=$BNDINSTALL --with-spatialite=yes && make -j16 && make install )

