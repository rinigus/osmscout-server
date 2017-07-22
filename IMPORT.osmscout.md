### Maps import

Maps provided by OpenStreetMaps have to be converted to the format
used by libosmscout library. The importing procedure below concerns
only libosmscout backend. When using other backends, separate
databases are required. For geocoder-nlp, see
https://github.com/rinigus/geocoder-nlp/blob/master/README.md.

The maps are imported from PBF or OSM file formats, as provided by
OpenStreetMap download servers.  While smaller maps lead to faster
rendering, if you need to use the server to calculate the routes
between countries, it maybe advantageous to generate a map covering
multiple countries. I suggest to make a joined map using osmconvert
(https://wiki.openstreetmap.org/wiki/Osmconvert) as described in
https://wiki.openstreetmap.org/wiki/Osmconvert#Parallel_Processing

For importing, you could either use pre-compiled released import tool
or compile the import tool from source.


### Using compiled Import tool 

Get the Import tool corresponding to the release of libosmscout
library that is used in your server build. For Sailfish OSM Scout
Server releases, the following import tools are available:

OSM Scout Server | libosmscout Sailfish
--- | ---
0.7.x and later | please use distributed maps
0.6.x | https://github.com/rinigus/libosmscout/releases/tag/0.0.git.20170126
0.5.x | https://github.com/rinigus/libosmscout/releases/tag/0.0.git.20161207
0.4.x | https://github.com/rinigus/libosmscout/releases/tag/0.0.git.20161128.2
0.3.0 | https://github.com/rinigus/libosmscout/releases/tag/0.0.git.20161118.1

Note that the maps format is not changing between all the versions. It
will be specified in OSM Scout Server and libosmscout release
changelog if the change in the format or significant import bug has
been fixed. For example, you could use the server 0.5.x releases with
the maps imported by the importer corresponding to 0.3.0.

### Compiling Import tool

See http://libosmscout.sourceforge.net/tutorials/Importing/ for
instructions. You would have to compile the library on your PC and run
the import program, as explained in the linked tutorial. Please
install MARISA as a dependency to be able to generate index files for
free-text search of the map data.

To keep minimal number of required map database files, use
the same options as in following import command example:

```
Import --delete-temporary-files true --delete-debugging-files true --delete-analysis-files true --delete-report-files true --typefile libosmscout/stylesheets/map.ost --destinationDirectory mymap mymap.osm.pbf
```

This would keep the number of produced files to minimum needed on the
device. Expect that the size of map would be similar to the map file in
PBF format.

