
set -e

LIBOSMIUM_VERSION=2.11.1
OSMIUM_VERSION=1.5.1
BNDINSTALL=`pwd`/osmium/install

mkdir -p osmium

rm -f osmium/libosmium.tar.gz
wget -O osmium/libosmium.tar.gz https://github.com/osmcode/libosmium/archive/v$LIBOSMIUM_VERSION.tar.gz 
(cd osmium && tar xvf libosmium.tar.gz)
(cd osmium/libosmium-$LIBOSMIUM_VERSION && rm -rf build && mkdir build && cd build && cmake -DINSTALL_PROTOZERO=ON -DINSTALL_UTFCPP=ON -DCMAKE_INSTALL_PREFIX:PATH=$BNDINSTALL .. && make -j4 && make install)

rm -f osmium/osmium-tool.tar.gz
wget -O osmium/osmium-tool.tar.gz https://github.com/osmcode/osmium-tool/archive/v$OSMIUM_VERSION.tar.gz
(cd osmium && tar xvf osmium-tool.tar.gz)
(cd osmium/osmium-tool-$OSMIUM_VERSION && rm -rf build && mkdir build && cd build && cmake -DOSMIUM_INCLUDE_DIRS=$BNDINSTALL/include -DCMAKE_INSTALL_PREFIX:PATH=$BNDINSTALL .. && make -j4 && make install)
