#!/bin/bash

set -e

echo "Target:     " $TARGET
echo "OS:         " $TRAVIS_OS_NAME
echo "Build tool: " $BUILDTOOL
echo "Compiler:   " $CXX

echo "Installation start time: `date`"

export DEBIAN_FRONTEND=noninteractive

sudo apt-get -qq update
sudo apt-get install -y cmake

sudo apt-get install -y \
     pkg-config autoconf automake libtool wget \
     libxml2-dev \
     libprotobuf-dev protobuf-compiler \
     libagg-dev libfreetype6-dev \
     libcairo2-dev libpangocairo-1.0-0 libpango1.0-dev \
     qt5-default qtdeclarative5-dev libqt5svg5-dev qtlocation5-dev qtbase5-dev \
     freeglut3 freeglut3-dev \
     libmarisa-dev libmicrohttpd-dev libsnappy-dev libsqlite3-dev libkyotocabinet-dev libmapnik-dev \
     libsystemd-daemon-dev

# libosmscout
if [ -z ${OSMSCOUT+x} ];
then
    echo libosmscout disabled
else
    git clone https://github.com/rinigus/libosmscout.git
    cd libosmscout
    mkdir build
    cd build
    cmake -DBUILD_WITH_OPENMP=OFF -DOSMSCOUT_BUILD_MAP_OPENGL=OFF -DOSMSCOUT_BUILD_IMPORT=OFF -DOSMSCOUT_BUILD_MAP_AGG=OFF -DOSMSCOUT_BUILD_MAP_CAIRO=OFF -DOSMSCOUT_BUILD_MAP_SVG=OFF -DOSMSCOUT_BUILD_MAP_IOSX=OFF -DOSMSCOUT_BUILD_TESTS=OFF -DOSMSCOUT_BUILD_DEMOS=OFF -DOSMSCOUT_BUILD_BINDING_JAVA=OFF -DOSMSCOUT_BUILD_BINDING_CSHARP=OFF -DOSMSCOUT_BUILD_DOC_API=OFF -DOSMSCOUT_BUILD_CLIENT_QT=ON -DOSMSCOUT_BUILD_TOOL_OSMSCOUT2=OFF -DOSMSCOUT_BUILD_TOOL_STYLEEDITOR=OFF -DGPERFTOOLS_USAGE=OFF -DOSMSCOUT_BUILD_TOOL_IMPORT=OFF -DOSMSCOUT_BUILD_TOOL_DUMPDATA=OFF ..
    make
    sudo make install
    cd ../..
fi

# valhalla
if [ -z ${VALHALLA+x} ];
then
    echo Valhalla disabled
else
    VALHALLA_VERSION=2.6.2
    sudo add-apt-repository -y ppa:valhalla-core/valhalla
    sudo apt-get update
    sudo apt-get install -y cmake make libtool pkg-config g++ gcc jq lcov protobuf-compiler vim-common libboost-all-dev libboost-all-dev libcurl4-openssl-dev zlib1g-dev liblz4-dev libprotobuf-dev
    sudo apt-get install -y libgeos-dev libgeos++-dev liblua5.2-dev libspatialite-dev libsqlite3-dev lua5.2 wget
    if [[ $(grep -cF xenial /etc/lsb-release) > 0 ]]; then sudo apt-get install -y libsqlite3-mod-spatialite; fi
    wget -O valhalla.tar.gz https://github.com/valhalla/valhalla/archive/$VALHALLA_VERSION.tar.gz
    tar zxvf valhalla.tar.gz
    cd valhalla-$VALHALLA_VERSION
    mkdir build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/usr -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DENABLE_DATA_TOOLS=OFF -DENABLE_PYTHON_BINDINGS=OFF -DENABLE_SERVICES=OFF
    make
    sudo make install
    cd ../..
fi


# libpostal - 
POSTAL_VERSION=1.0.0
wget -O libpostal.tar.gz https://github.com/openvenues/libpostal/archive/v$POSTAL_VERSION.tar.gz
tar zxvf libpostal.tar.gz
cd libpostal-$POSTAL_VERSION
#git clone https://github.com/openvenues/libpostal.git
#cd libpostal
./bootstrap.sh
./configure --datadir=/usr/local/libpostal/data --disable-data-download
make
sudo make install
cd ..

echo "Installation end time: `date`"
