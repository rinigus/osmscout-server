# OSM Scout Server

[![Build Status](https://travis-ci.org/rinigus/osmscout-server.svg?branch=master)](https://travis-ci.org/rinigus/osmscout-server)
[![Donate](https://img.shields.io/badge/donate-liberapay-yellow.svg)](https://liberapay.com/rinigus)

OSM Scout server can be used as a drop-in replacement for online map
services providing map tiles, search, and routing. As a result, an
offline operation is possible if the device has a server and map
client programs installed and running.

At present, the server can be used to provide:
* vector or raster tiles for other applications;
* search for locations and free text search;
* search for POIs next to a reference area;
* calculating routes between given sequence of points.

User's guide is available at https://rinigus.github.io/osmscout-server

The server supports:
* map rendering via Mapnik (https://github.com/mapnik/mapnik);
* hosting of Mapbox GL vector tiles (https://github.com/mapbox/awesome-vector-tiles);
* search via Geocoder-NLP (https://github.com/rinigus/geocoder-nlp) which is based on libpostal (https://github.com/openvenues/libpostal);
* routing instructions via Valhalla (https://github.com/valhalla/valhalla);
* map rendering, search, and routing via libosmscout (http://libosmscout.sourceforge.net/).

To use the server, you have to start it and configure the client to
access it. An example configurations for Poor Maps, modRana, and
JavaScript-based clients are provided under "example" folder. At
present, Poor Maps and modRana include plugins already in the upstream
and no additional configuration is needed.

The server is written using Qt. The server can be used as a console, a
Sailfish, or a QtQuick application. For console and QtQuick versions,
use the corresponding project file, available in pro subfolder. For
Sailfish, use osmscout-server_silica.pro.


## Maps

Starting from version 0.7, the server uses maps distributed via online
distribution network. It is expected that the users would download,
update, and, when needed, remove maps via server's GUI/CLI. This
distribution model allows users to specify which components are
required (libosmscout, geocoder-nlp, for example) and download only
the required components.

If tinkering is required, it is still possible to import the maps
manually. However, in this case, the user is expected to incorporate
the manually imported maps into configuration JSON file describing
that map and use specific developer options.


### Maps distribution and data

Maps are hosted by Natural Language Processing Centre
(https://nlp.fi.muni.cz/en/ , Faculty of Informatics, Masaryk
University, Brno, Czech Republic) through modRana (http://modrana.org)
data repository.

Map data from OpenStreetMap, Open Database License 1.0. Maps are
converted to a suitable format from downloaded extracts and/or using
polygons as provided by Geofabrik GmbH.


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
of CPUs. Depending on the used backend, one operation can block other
operations due to the blocking of the corresponding
database. Exceeding the number of supported connections would lead to
dropping the connections exceeding the limit.


## General notes for developers

There are several aspects that became evident while developing and
using OSM Scout Server for providing offline solution on a mobile
platform, such as Sailfish OS.

### Request large tiles

When requesting rendered map tiles, its advantageous to request larger
tiles. Each tile is rendered with an overhead that is induced by
labels located close to the borders. For example, current default
settings in Mapnik backend, add 128 pixels on each side of the
tile. This buffer is internally multiplied by the scale of Mapnik map,
commonly set to 3. So, we have 384 pixels added on each side. If the
common size of the tiles is requested (256x256), each tile triggers
rendering of 1024x1024 out of which only 256x256 section is used. In
other words, we use only 6.25% of the rendered data. By requesting
tiles with the size 1024x1024, we bring up efficiency to 32.7%, a
significant increase.

### Do not use timeouts

When using the server on mobile, one has to remember that the server
is not rendering tiles nor performing any other calculations while the
device is asleep. The server is not keeping device awake, its a job of
a client that interacts with the user and knows whether current
operation is time-critical, as during navigation, or can be postponed,
as when user is just browsing a map and switched off the phone. As a
result, using timeouts while interacting with the server is strongly
discouraged. In particular, when using timeouts and the device was put
to sleep, this can cause large number of re-submissions by a client
leading to failure of the server to process a long accumulated queue
of requests in reasonable time. So, its recommended not to use any
timeouts when using server on the device that can be put to sleep or,
if the timeouts are needed, not to submit new requests after
that. Since, when client and the server are operating on the same
device, network failure is not expected, such timeouts are, in
general, not needed.


## Default port

Default port is 8553 TCP and the server binds to 127.0.0.1 providing
services to local apps only.


## URL scheme

Access to functionality is provided via path and query parts of
URL. The path determines the module that is accessed with the query
specifying arguments. Here, order of query elements is not important.


## Examples

See `examples` folder for results of the example queries.


## Raster tiles

The server component for providing raster tiles operates using OSM convention
with small extensions. URL is

`http://localhost:8553/v1/tile?style={style}&daylight={dlight}&shift={shift}&scale={scale}&z={z}&x={x}&y={y}`

where

`{style}` - style of the map, set to `default` if not specified

`{dlight}` - either 0 for night or 1 for day

`{shift}` - allows to change used {z} by increasing it to {z}+{shift}

`{scale}` - size of a tile in pixels is {scale}*256

`{z}`, `{x}`, and `{y}` are as in http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames .

Addition of `{scale}` and `{shift}` allows to experiment with
different tile sizes to optimize for performance and human-map
interaction. Note that `shift` is ignored in Mapnik backend. See Poor
Maps settings for example.

At present, only Mapnik backend supports different styles. When using
libosmscout backend, `styles` parameter is ignored.


## Mapbox GL vector tiles

The vector tiles and associated styles, fonts and icons are provided
via server.

### Tiles

For requesting tiles, use

`http://localhost:8553/v1/mbgl/tile?z={z}&x={x}&y={y}`

where

`{z}`, `{x}`, and `{y}` are as in
http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames .


### Styles, sprite

For requesting styles, use

`http://localhost:8553/v1/mbgl/style?style={style}`

where `{style}` is a style name.

Fonts (glyphs) are provided via

`http://localhost:8553/v1/mbgl/glyphs?stack={fontstack}&range={range}`

where

`{fontstack}` - requested font stack, for example `Noto Sans`

`{range}` - requested range, for example `0-255`.

In the styles, corresponding setting for glyphs is
`http://localhost:8553/v1/mbgl/glyphs?stack={fontstack}&range={range}`.

The styles can use provided sprite with icons by specifying
`http://localhost:8553/v1/mbgl/sprite` as a corresponding URL in style
definition.



## Location search

There are two versions of the location search query results. The only
difference is in returned JSON format with the second version, in
addition to returning the results, giving feedback on query parsing to
the user.

### Location search: version 1

The location search is accessed by the following URL:

`http://localhost:8553/v1/search?limit={limit}&search={query}`

where

`{limit}` - maximal number of search results

`{query}` - location and free text search

Results are returned in JSON format. Example query:
`http://localhost:8553/v1/search?limit=10&search=tartu mnt 1, tallinn`

```
[
{
"admin_region": "Tallinn, Kesklinna linnaosa, Tallinna linn, Harju maakond, Eesti",
"lat": 59.434895,
"lng": 24.758684,
"object_id": "Node 631817",
"title": "Tartu mnt 1, Tallinn",
"type": "address"
},
{
"admin_region": "Tallinn, Kesklinna linnaosa, Tallinna linn, Harju maakond, Eesti",
"lat": 59.404687,
"lng": 24.810360,
"object_id": "Way 9670330",
"title": "Tartu mnt, Tallinn",
"type": "highway_trunk"
},
...
]
```

### Location search: version 2

The location search is accessed by the following URL:

`http://localhost:8553/v2/search?limit={limit}&search={query}`

where meaning of the query parameters is the same as for the version
one. However, the result includes parsing feedback when geocoder-nlp
is used. For example,
`http://localhost:8553/v2/search?limit=3&search=tartu mnt 1, tallinn`:

```
{
    "parsed": {
        "city": "tallinn",
        "house_number": "1",
        "road": "tartu mnt"
    },
    "parsed_normalized": [
        {
            "city": "tallinn",
            "house_number": "1",
            "road": "tartu maantee"
        },
        {
            "h-0": "tallinn",
            "h-1": "tartu maantee 1"
        }
    ],
    "query": "tartu mnt 1, tallinn",
    "result": [
        {
            "admin_region": "1, Tartu mnt, Kesklinna linnaosa, Tallinna linn, Harju maakond, Eesti",
            "lat": 59.434894989690889,
            "levels_resolved": 3,
            "lng": 24.758684372594075,
            "object_id": 31299,
            "title": "1, Tartu mnt",
            "type": ""
        },
        {
            "admin_region": "13, Tartu mnt, Kesklinna linnaosa, Tallinna linn, Harju maakond, Eesti",
            "lat": 59.434417556482686,
            "levels_resolved": 3,
            "lng": 24.761235153386252,
            "object_id": 31312,
            "title": "13, Tartu mnt",
            "type": ""
        },
        {
            "admin_region": "14, Tartu mnt, Kesklinna linnaosa, Tallinna linn, Harju maakond, Eesti",
            "lat": 59.433896537377663,
            "levels_resolved": 3,
            "lng": 24.761358535001861,
            "object_id": 31314,
            "title": "14, Tartu mnt",
            "type": ""
        }
    ]
}
```


## List of available POI types

List of available POI types is available via

`http://localhost:8553/v1/poi_types`


## POI search near a reference position

To find POIs within a given radius from a specified reference
position, server can be accessed via `/v1/guide` path:

`http://localhost:8553/v1/guide?radius={radius}&blimit={limit}&query={query}&search={search}&lng={lng}&lat={lat}`

where

`{limit}` - maximal number of search results

`{radius}` - distance from the reference in meters

`{query}` - POI type name substring (checked against POI type name in case-insensitive manner) or name of the POI

`{search}` - a query that is run to find a reference point, the first result is used

`{lng}`, `{lat}` - longitude and latidude, respectively.

Query is considered as a substring that is looked for in all available
POI types without taking into account the case of letters. For
example, "Cafe" would match amenity_cafe_building and
amenity_cafe.

In addition, POI can be searched by its name. For example, you could
search for the restaurant by its name. This is only supported by
Geocoder-NLP backend. In Geocoder-NLP, all query requests are
normalized using libpostal and, in case of the names, compared with
the normalized names of POIs.

For backward compatibility, `query` keyword can be replaced with
`poitype`.

The reference point can be given either as a query ("Paris") or as a
location coordinates. If the both forms are given in URL, location
coordinates are preferred.

The result is given in JSON format. It returns a JSON object with
keys: `"origin"` (coordinates of the reference point used in the
search) and `"results"` (array with the POIs). See `examples` folder
and Poor Maps implementation on how to process the results.


## Routing

There are two versions of routing protocol that have to be used in
accordance with the used backend. Version 1 (`v1/route`) is used by
libosmscout and is described below. Version 2 (`v2/route`) is used by
Valhalla and uses Valhalla's API. Version 2 is supported, in part, by
libosmscout as well.

### Version 1: libosmscout

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


### Version 2: Valhalla

This is the version that would be mainly supported in future. It uses
Valhalla's API, as described in
https://github.com/valhalla/valhalla-docs/blob/master/turn-by-turn/api-reference.md
. Please note that there is no API key in the Valhalla's component
used by OSM Scout Server.

At present, all calls via `v2/route`, as
`http://localhost:8553/v2/route?...` would be forwarded to Valhalla
via `/route?...`.


### Version 2: libosmscout

When using libosmscout as a backend, version 2 can be used to request
the routes. In this case, the server would consider limited subset of
Valhalla's API. In particular, `costing` option would be used to
select the transportation mode (`auto`, `bicycle`, or `pedestrian`)
with the order of points found from `location`. The reply of the
server will follow Version 1 protocol with an additional flag `API
version` set to `libosmscout V1` in the response of the server. Using
this flag, the client application can determine whether version 1
protocol response has been used.


## Translations

The translations were contributed by

- Carmen F. B. @carmenfdezb: Spanish
- Lukáš Karas @Karry: Czech
- Åke Engelbrektson @eson57: Swedish
- Ricardo Breitkopf @monkeyisland: German
- Nathan Follens @pljmn: Dutch
- @Sagittarii: French
- Oleg Artobolevsky @XOleg: Russian
- A @atlochowski: Polish
- Peer-Atle Motland @Pam: Norwegian Bokmål
- Matti Lehtimäki @mal: Finnish


For translations, please see https://github.com/rinigus/osmscout-server/blob/master/translations/README


## Libpostal

Please note that libpostal is developed to be used with the fine tuned
model covering the World. The country-based models were developed to
use libpostal in mobile devices and have not been specifically
tuned. Please submit the issues with libpostal performance on
country-based models to OSM Scout Server or geocoder-nlp github
projects.


## Acknowledgments

libosmscout: http://libosmscout.sourceforge.net

libpostal: https://github.com/openvenues/libpostal

mapnik: https://github.com/mapnik/mapnik

valhalla: https://github.com/valhalla/valhalla

geocoder-nlp: https://github.com/rinigus/geocoder-nlp

osmscout-sailfish: https://github.com/Karry/osmscout-sailfish

GNU Libmicrohttpd: https://www.gnu.org/software/libmicrohttpd

langcodes: https://github.com/LuminosoInsight/langcodes

Mapbox GL import scripts: https://github.com/rinigus/mapbox-gl-importer

Hosting of maps: Natural Language Processing Centre
(https://nlp.fi.muni.cz/en/ , Faculty of Informatics, Masaryk
University, Brno, Czech Republic) through modRana
(http://modrana.org).

