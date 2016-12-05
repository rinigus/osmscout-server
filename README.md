# OSM Scout Server

Server based on libosmscout. This server is envisioned to be a drop-in
replacement for online map services providing map tiles, search, and
routing. As a result, an offline operation is possible if the device
has a server and map client programs installed and running.

At present, the server can be used to provide:
* map tiles for other applications;
* search for locations and free text search;
* search for POIs next to a reference area;
* calculating routes between given sequence of points.

To use the server, you have to start it and configure the client to
access it. An example configurations for Poor Maps and modRana are
provided under "thirdparty" folder. At present, Poor Maps includes
plugins already in the upstream and no additional configuration is
needed. Poor Maps plugins cover functionality of the server
fully. modRana can use the server to show the map with the other
functionality expected in near future.

The server is written using Qt. The server can be used as a console or
a Sailfish application. For console version, use
osmscout-server_console.pro as a project. For Sailfish, use
osmscout-server_silica.pro. For drawing, its possible to use Qt or
Cairo backends, determined during compilation. While default
configuration uses Qt for drawing, it maybe advantageous to use Cairo
in server environment when compiled as a console application.


## Maps

Maps provided by OpenStreetMaps have to be converted to the format
used by libosmscout library. 

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
0.4.x | https://github.com/rinigus/libosmscout/releases/tag/0.0.git.20161128.2
0.3.0 | https://github.com/rinigus/libosmscout/releases/tag/0.0.git.20161118.1


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


## Settings

Configuration settings are in
~/.config/osmscout-server/osmscout-server.conf and this file is
initialized on the first start. For Sailfish version, add prefix
harbour- to the file and directory name.

In configuration file you can specify server HTTP port and the
interface. In addition, map directory with several rendering options
are configured. In Sailfish, map rendering options are possible to
specify through GUI.

Starting from version 0.3.0, server supports up to 100
connections. The requests are processed in parallel, as much as
possible, with the number of parallel threads the same as the number
of CPUs. In practice, while tiles are rendered in parallel, a long
routing calculation would block other operations until its finished
due to the locking of a database.  Exceeding the number of supported
connections would lead to dropping the connections exceeding the
limit.


## Default port

Default port is 8553 TCP and the server binds to 127.0.0.1 providing
services to local apps only.


## URL scheme

Access to functionality is provided via path and query parts of
URL. The path determines the module that is accessed with the query
specifying arguments. Here, order of query elements is not important.


## Examples

See `examples` folder for results of the example queries.


## Tiles

The server component for providing tiles operates using OSM convention
with small extensions. URL is

`http://localhost:8553/v1/tile?daylight={dlight}&shift={shift}&scale={scale}&z={z}&x={x}&y={y}`

where

`{dlight}` - either 0 for night or 1 for day

`{shift}` - allows to change used {z} by increasing it to {z}+{shift}

`{scale}` - size of a tile is {scale}*256

`{z}`, `{x}`, and `{y}` are as in http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames .

Addition of `{scale}` and `{shift}` allows to experiment with different
tile sizes to optimize for performance and human-map interaction. See
Poor Maps settings for example.


## Location search

The location search is accessed by the following URL:

`http://localhost:8553/v1/search?limit={limit}&search={query}`

where

`{limit}` - maximal number of search results

`{query}` - location and free text search

Results are returned in JSON format. See Poor Maps geocoder
implementation for details of the format.


## List of available POI types

List of available POI types is available via 

`http://localhost:8553/v1/poi_types`


## POI search near a reference position

To find POIs within a given radius from a specified reference
position, server can be accessed via `/v1/guide` path:

`http://localhost:8553/v1/guide?radius={radius}&blimit={limit}&poitype={poitype}&search={search}&lng={lng}&lat={lat}`

where

`{limit}` - maximal number of search results

`{radius}` - distance from the reference in meters

`{poitype}` - POI type name substring (checked against POI type name in case-insensitive manner)

`{search}` - a query that is run to find a reference point, the first result is used

`{lng}`, `{lat}` - longitude and latidude, respectively.

As mentioned above, given POI type is considered as a substring that
is looked for in all available POI types without taking into account
the case of letters. For example, "Cafe" would match
amenity_cafe_building and amenity_cafe. However, "Café" would miss
them.

The reference point can be given either as a query ("Paris") or as a
location coordinates. If the both forms are given in URL, location
coordinates are preferred.

The result is given in JSON format. It returns a JSON object with two
keys: `"origin"` (coordinates of the reference point used in the search)
and `"results"` (array with the POIs). See Poor Maps implementation on
how to process the results.


## Routing

The routing component allows to calculate routes between given
points. Server can be accessed via `/v1/route` path:

`http://localhost:8553/v1/route?radius={radius}&type={type}&gpx={gpx}&p[0][search]={search}&p[0][lng]={lng}&p[0][lat]={lat}& ... &p[n-1][search]={search}&p[n-1][lng]={lng}&p[n-1][lat]={lat}`

where each point along the route can be given either by `{search}` or
longitude and latitude with precise coordinates preferred if the both
approaches are used. The number of points `n` should be at least two,
with the counting starting from 0. The server looks for points in the
query by starting from index 0 and incrementing it by one until the
points with consecutive indexes are all found. Note that if you skip
an index in the list (like having indexes 0, 1, 3, and 4), the points
after the increment larger than one will be ignored (in the example,
points 3 and 4). 

The query parameters are:

`{type}` - type of the vehicle with `car`, `bicycle`, and `foot`
supported (`car` is default);

`{radius}` - distance from the points in meters where closest routing
point is searched for (1000 meters by default);

`{gpx}` - when 1 or larger integer, GPX trace of the route will be
given in the response of the server instead of JSON reply;

`{search}` - a query that is run to find a reference point, the first
result is used;

`{lng}`, `{lat}` - longitude and latidude, respectively.


For example, the following request finds the route between two cities
given by names:

`http://localhost:8553/v1/route?p[0][search]=Tallinn&p[1][search]=Tartu`


The result is given in JSON format. It returns a JSON object with
several keys: 

`locations` - coordinates of the reference points used in the calculations;

`language` - language of the maneuvers instructions;

`lat` - array of latitudes with the calculated route;

`lng` - array of longitudes with the calculated route;

`maneuvers` - array of objects describing maneuvers;

`summary` - object specifying length and duration of the route;

`units_distance` - units of distances used in route description (kilometers for now);

`units_time` - units of time used in route description (seconds for now).


See included example under Examples and Poor Maps implementation on
how to process the results.

At present, the car speeds on different roads are inserted in the
code. This will improve in future.


## Translations

The translations were contributed by

Carmen F. B. @carmenfdezb: Spanish


## Acknowledgments

libosmscout: http://libosmscout.sourceforge.net

osmscout-sailfish: https://github.com/Karry/osmscout-sailfish

GNU Libmicrohttpd: https://www.gnu.org/software/libmicrohttpd


[![Build Status](https://travis-ci.org/rinigus/osmscout-server.svg?branch=master)](https://travis-ci.org/rinigus/osmscout-server)
