# OSM Scout Server

Server based on libosmscout. This server is envisioned to be a drop-in
replacement for online map services providing map tiles, search, and
routing. As a result, an offline operation is possible if the device
has a server and map client programs installed and running.

At present, the server can be used to provide:
* map tiles for other applications;
* search for locations and free text search;
* search for POIs next to a reference area.

To use the server, you have to start it and configure the client to
access it. An example configurations for Poor Maps and modRana are
provided under "thirdparty" folder. At present, Poor Maps plugins
cover functionality of the server fully. modRana can use the server to
show the map with the other functionality expected in near future.

The server is written using Qt. The server can be used as a console or
Sailfish application. For console version, use
osmscout-server_console.pro as a project. For Sailfish, use
osmscout-server_silica.pro. For drawing, its possible to use Qt or
Cairo backends, determined during compilation. While default
configuration uses Qt for drawing, it maybe advantageous to use Cairo
in server environment when compiled as a console application.


## Maps

Maps provided by OpenStreetMaps have to be converted to the format
used by libosmscout library. See
http://libosmscout.sourceforge.net/tutorials/Importing/ for
instructions. At this stage, you would have to compile the library on
your PC and run the import program, as explained in the linked
tutorial. Please install MARISA as a dependency to be able to generate
index files for free-text search of the map data.

Note that since it is planned to extend the server to provide routing
as well, the server can at any given time provide data from a single
map. If you need to cover an area from multiple countries, I suggest
to make a joined map using osmconvert
(https://wiki.openstreetmap.org/wiki/Osmconvert) as described in
https://wiki.openstreetmap.org/wiki/Osmconvert#Parallel_Processing

Finally, regarding the maps: present Import program keeps several
debug and temporary files. On device, these files are not needed and
you need only the resulting files listed in the end of the
import. (see issue https://github.com/Framstag/libosmscout/issues/133
for information regarding it). Expect that the size of map would be
similar to the map file in PBF format.


## Settings

Configuration settings are in
~/.config/osmscout-server/osmscout-server.conf and this file is
initialized on the first start. For Sailfish version, add prefix
harbour- to the file and directory name.

In configuration file you can specify server HTTP port and the
interface. In addition, map directory with several rendering options
are configured. In Sailfish, map rendering options are possible to
specify through GUI.

Server supports limited number of connections (usually number of CPU
cores plus 2). It is able to render several tiles in parallel, if the
client application supports it. Exceeding the number of supported
connections would lead to dropping the connections exceeding the
limit.


## Default port

Default port is 8553 TCP and the server binds to 127.0.0.1 providing
services to local apps only.


## URL scheme

Access to functionality is provided via path and query parts of
URL. The path determines the module that is accessed with the query
specifying arguments. Here, order of query elements is not important.


## Tile server

The server component for providing tiles operates using OSM convention
with small extensions. URL is

http://localhost:8553/v1/tile?daylight={dlight}&shift={shift}&scale={scale}&z={z}&x={x}&y={y}

where

{dlight} - either 0 for night or 1 for day

{shift} - allows to change used {z} by increasing it to {z}+{shift}

{scale} - size of a tile is {scale}*256

{z}, {x}, and {y} are as in http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames .

Addition of {scale} and {shift} allows to experiment with different
tile sizes to optimize for performance and human-map interaction. See
Poor Maps settings for example.


## Location search

The location search is accessed by the following URL:

http://localhost:8553/v1/search?limit={limit}?search={query}

where

{limit} - maximal number of search results

{query} - location and free text search

Results are returned in JSON format. See Poor Maps geocoder
implementation for details of the format.


## List of available POI types

List of available POI types is available via 

http://localhost:8553/v1/poi_types


## POI search near a reference position

To find POIs within a given radius from a specified reference
position, server can be accessed via `/v1/guide` path:

http://localhost:8553/v1/guide?radius={radius}&limit={limit}&poitype={poitype}&search={search}&lng={lng}&lat={lat}

where

{limit} - maximal number of search results

{radius} - distance from the reference in meters

{poitype} - POI type name substring (checked against POI type name in case-insensitive manner)

{search} - a query that is run to find a reference point, the first result is used

{lng}, {lat} - longitude and latidude, respectively.

As mentioned above, given POI type is considered as a substring that
is looked for in all available POI types without taking into account
the case of letters. For example, "Cafe" would match
amenity_cafe_building and amenity_cafe. However, "Café" would miss
them.

The reference point can be given either as a query ("Paris") or as a
location coordinates. If the both forms are given in URL, location
coordinates are preferred.

The result is given in JSON format. It returns a JSON object with two
keys: "origin" (coordinates of the reference point used in the search)
and "results" (array with the POIs). See Poor Maps implementation on
how to process the results.


## Acknowledgments

libosmscout: http://libosmscout.sourceforge.net

osmscout-sailfish: https://github.com/Karry/osmscout-sailfish

QtWebApp HTTP Webserver: http://stefanfrings.de/qtwebapp/index-en.html
