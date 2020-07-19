#!/bin/bash

set -e

export PATH=`pwd`/install/bin:$PATH
export LD_LIBRARY_PATH=`pwd`/install/lib:$LD_LIBRARY_PATH

rm -rf install-dist
mkdir install-dist
cd install-dist

for i in {valhalla_build_admins,valhalla_build_timezones,valhalla_build_tiles,valhalla_run_route,valhalla_build_elevation,valhalla_build_transit,valhalla_fetch_transit} ; do
    cp `which $i` .
    for j in `ldd $i  | grep /usr/lib | awk '{ print $3; }'`; do
        cp $j .
    done
    for j in `ldd $i  | grep /lib | awk '{ print $3; }'`; do
        cp $j .
    done
done

for i in /usr/lib/x86_64-linux-gnu/mod_spatialite.so ; do
    cp $i .
    for j in `ldd $i  | grep /usr/lib | awk '{ print $3; }'`; do
        cp $j .
    done
    for j in `ldd $i  | grep /lib | awk '{ print $3; }'`; do
        cp $j .
    done
done

ln -s mod_spatialite.so mod_spatialite
rm libc.so.6 libpthread.so.0 libm.so.6 libdl.so.2

