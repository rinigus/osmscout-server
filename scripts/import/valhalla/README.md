# Import by Valhalla

Valhalla has issues with the import which may lead to crashes. This could be caused by the bugs in the code,
incompatibility with Lua installed in the system or something else. It seems that the best strategy
right now is to compile Valhalla on supported platform (Ubuntu 18.04 seems fine), make a folder with all
required binaries and libs, move to the system where you import and import using those binaries.

Working version: 71cf5b8815a0f4c8f90b4eb45d3636bf047d38b2

Corresponding scripts:

- make-dist.sh - run this after compilation and install into `install` folder. It will create 
`install-dist` with executables and libs

- import_planet_install_dist.sh - use it for import instead of regular import_planet.sh
