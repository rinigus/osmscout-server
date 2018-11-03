# Installation instructions

## Compilation from source

### Ubuntu 18.04

To compile and install under /usr/local, proceed as follows.

Install packages:

```
sudo apt-get install git libsystemd-dev libmarisa-dev libmicrohttpd-dev libkyotocabinet-dev libprotobuf-dev libboost-dev liblz4-dev libsqlite3-dev libcurl4-openssl-dev zlib1g-dev libicu-dev libharfbuzz-dev libboost-system-dev libboost-filesystem-dev libboost-regex-dev libboost-program-options-dev libjpeg-dev libproj-dev libfreetype6-dev libpng-dev libwebp-dev libtiff5-dev cmake qt5-default libxml2-dev qtdeclarative5-dev make libtool pkg-config g++ gcc jq lcov protobuf-compiler vim-common libboost-date-time-dev libboost-iostreams-dev libboost-thread-dev liblua5.2-dev qtpositioning5-dev texinfo qml-module-qtquick2  qml-module-qtquick-controls2 qml-module-qtquick-controls qml-module-qtquick-layouts qml-module-qtquick-dialogs qml-module-qt-labs-settings qml-module-qt-labs-folderlistmodel
```

Build the following libraries

Mapnik:

```
git clone --recursive https://github.com/rinigus/pkg-mapnik
cd pkg-mapnik/mapnik
patch -p1 < ../rpm/mapnik.twkb.patch
patch -p1 < ../rpm/mapnik.issue3384.patch
make reset
./configure INPUT_PLUGINS="sqlite,shape" PREFIX="/usr/local" LINKING=shared OPTIMIZATION=2 CPP_TESTS=no CAIRO=no PLUGIN_LINKING=static MEMORY_MAPPED_FILE=no DEMO=no MAPNIK_INDEX=no MAPNIK_RENDER=no
make
sudo make install
sudo cp -r deps/mapbox/variant/include/mapbox /usr/local/include
cd ../../
```

Compile libpostal:

```
git clone --recursive https://github.com/rinigus/pkg-libpostal.git
cd pkg-libpostal/libpostal
./bootstrap.sh
./configure --datadir=/usr/local/libpostal/data --disable-data-download
make -j$(nproc)
sudo make install
cd ../../
```

libosmscout:
```
git clone --recursive https://github.com/rinigus/libosmscout.git
cd libosmscout
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local -DBUILD_WITH_OPENMP=OFF -DOSMSCOUT_BUILD_MAP_OPENGL=OFF -DOSMSCOUT_BUILD_IMPORT=OFF -DOSMSCOUT_BUILD_MAP_AGG=OFF -DOSMSCOUT_BUILD_MAP_CAIRO=OFF -DOSMSCOUT_BUILD_MAP_SVG=OFF -DOSMSCOUT_BUILD_MAP_IOSX=OFF -DOSMSCOUT_BUILD_TESTS=OFF -DOSMSCOUT_BUILD_DEMOS=OFF -DOSMSCOUT_BUILD_BINDING_JAVA=OFF -DOSMSCOUT_BUILD_BINDING_CSHARP=OFF -DOSMSCOUT_BUILD_DOC_API=OFF -DOSMSCOUT_BUILD_CLIENT_QT=ON -DOSMSCOUT_BUILD_TOOL_OSMSCOUT2=OFF -DOSMSCOUT_BUILD_TOOL_STYLEEDITOR=OFF -DGPERFTOOLS_USAGE=OFF -DOSMSCOUT_BUILD_TOOL_IMPORT=OFF -DOSMSCOUT_BUILD_TOOL_DUMPDATA=OFF ..
make -j$(nproc)
sudo make install
cd ../..
```

Light version of Valhalla:
```
git clone --recursive https://github.com/rinigus/pkg-valhalla-lite.git
cd pkg-valhalla-lite/valhalla
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/usr/local -DENABLE_DATA_TOOLS=OFF -DENABLE_PYTHON_BINDINGS=OFF -DENABLE_SERVICES=OFF
make
sudo make install
cd ../../../
```

OSM Scout Server:
```
git clone --recursive https://github.com/rinigus/osmscout-server.git
cd osmscout-server
ln -s pro/osmscout-server_qtcontrols.pro .
mkdir build && cd build
qmake PREFIX=/usr/local ../osmscout-server_qtcontrols.pro
make -j$(nproc)
sudo make install
```

If planning to use with Mapnik, get the fonts:
```
sudo git clone --recursive https://github.com/rinigus/osmscout-server-fonts.git /usr/local/share/osmscout-server-fonts
```


