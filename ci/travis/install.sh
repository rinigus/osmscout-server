#!/bin/sh

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
     libmarisa-dev libmicrohttpd-dev libsnappy-dev libsqlite3-dev libkyotocabinet-dev libmapnik-dev

POSTAL_VERSION=0.3.4
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

git clone https://github.com/rinigus/libosmscout.git
cd libosmscout
mkdir build
cd build
cmake -DBUILD_WITH_OPENMP=OFF -DOSMSCOUT_BUILD_MAP_OPENGL=OFF -DOSMSCOUT_BUILD_IMPORT=OFF -DOSMSCOUT_BUILD_MAP_AGG=OFF -DOSMSCOUT_BUILD_MAP_CAIRO=OFF -DOSMSCOUT_BUILD_MAP_SVG=OFF -DOSMSCOUT_BUILD_MAP_IOSX=OFF -DOSMSCOUT_BUILD_TESTS=OFF -DOSMSCOUT_BUILD_DEMOS=OFF -DOSMSCOUT_BUILD_BINDING_JAVA=OFF -DOSMSCOUT_BUILD_BINDING_CSHARP=OFF -DOSMSCOUT_BUILD_DOC_API=OFF -DOSMSCOUT_BUILD_CLIENT_QT=ON -DOSMSCOUT_BUILD_TOOL_OSMSCOUT2=OFF -DOSMSCOUT_BUILD_TOOL_STYLEEDITOR=OFF -DGPERFTOOLS_USAGE=OFF -DOSMSCOUT_BUILD_TOOL_IMPORT=OFF -DOSMSCOUT_BUILD_TOOL_DUMPDATA=OFF ..

make
sudo make install

echo "Installation end time: `date`"
