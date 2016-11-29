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
     pkg-config \
     libxml2-dev \
     libprotobuf-dev protobuf-compiler \
     libagg-dev libfreetype6-dev \
     libcairo2-dev libpangocairo-1.0-0 libpango1.0-dev \
     qt5-default qtdeclarative5-dev libqt5svg5-dev qtlocation5-dev \
     freeglut3 freeglut3-dev \
     libmarisa-dev

git clone https://github.com/rinigus/libosmscout.git
cd libosmscout
mkdir build
cd build
cmake .. && make && make install

echo "Installation end time: `date`"
