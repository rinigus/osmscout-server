#!/bin/bash

set -e

echo "Install backends used by OSM Scout Server maps importer"

BNDSRC=`pwd`/backends-source
BNDINSTALL=`pwd`/backends-install

# sources are deleted, but not install. this is to prevent libpostal
# dataset downloads when its not needed
rm -rf $BNDSRC
mkdir -p $BNDSRC

# libpostal
git clone https://github.com/openvenues/libpostal $BNDSRC/libpostal
( cd $BNDSRC/libpostal && ./bootstrap.sh && ./configure --datadir=$BNDINSTALL --prefix=$BNDINSTALL && make -j3 && make install )

# libosmscout
git clone --recursive https://github.com/rinigus/libosmscout.git $BNDSRC/libosmscout
mkdir -p $BNDSRC/libosmscout/build
( cd $BNDSRC/libosmscout/build && cmake -DCMAKE_INSTALL_PREFIX:PATH=$BNDINSTALL -DBUILD_WITH_OPENMP=OFF -DOSMSCOUT_BUILD_DOC_API=OFF -DOSMSCOUT_BUILD_BINDING_JAVA=OFF -DOSMSCOUT_BUILD_MAP_OPENGL=OFF  -DOSMSCOUT_BUILD_MAP_AGG=OFF -DOSMSCOUT_BUILD_MAP_CAIRO=OFF -DOSMSCOUT_BUILD_MAP_SVG=OFF -DOSMSCOUT_BUILD_MAP_QT=OFF -DOSMSCOUT_BUILD_MAP_IOSX=OFF -DOSMSCOUT_BUILD_TESTS=OFF -DOSMSCOUT_BUILD_DEMOS=OFF -DOSMSCOUT_BUILD_BINDING_JAVA=OFF -DOSMSCOUT_BUILD_BINDING_CSHARP=OFF -DOSMSCOUT_BUILD_DOC_API=OFF -DOSMSCOUT_BUILD_CLIENT_QT=OFF -DOSMSCOUT_BUILD_TOOL_OSMSCOUT2=OFF -DOSMSCOUT_BUILD_TOOL_STYLEEDITOR=OFF -DGPERFTOOLS_USAGE=OFF -DOSMSCOUT_BUILD_TOOL_IMPORT=ON .. && make -j6 && make install )

# geocoder-nlp
git clone --recursive https://github.com/rinigus/geocoder-nlp.git $BNDSRC/geocoder-nlp
(cd $BNDSRC/geocoder-nlp/importer && make LIBPOSTAL_INCLUDE=-I$BNDINSTALL/include LIBPOSTAL_LIB="-L$BNDINSTALL/lib -lpostal" && ln -s `pwd`/importer $BNDINSTALL/bin/geocoder-importer )
