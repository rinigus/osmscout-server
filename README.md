# OSM Scout Server

[![Build Status](https://travis-ci.org/rinigus/osmscout-server.svg?branch=master)](https://travis-ci.org/rinigus/osmscout-server)
[![Donate](https://img.shields.io/badge/donate-liberapay-yellow.svg)](https://liberapay.com/rinigus)
[![Donate](https://img.shields.io/badge/donate-bitcoin-yellowgreen.svg)](http://rinigus.github.io/donate-bitcoin)

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

See `examples` folder for the example queries and their
results. Below, just the results are referenced. For corresponding
queries, see accompanying [README](examples/README.md). 


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

The vector tiles, associated styles, fonts, and icons are provided
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

Results are returned in JSON format ([example](examples/search_v1.json)).

### Location search: version 2

The location search is accessed by the following URL:

`http://localhost:8553/v2/search?limit={limit}&search={query}`

where meaning of the query parameters is the same as for the version
one. However, the result includes parsing feedback when geocoder-nlp
is used, see [example](examples/search_v2.json).


## List of available POI types

List of available POI types is available via

`http://localhost:8553/v1/poi_types`

The list is given as JSON array. When using geocoder-nlp as a search
backend, the list represents currently used aliases for the used tags.
Shortened version of the response is given as an
[example](examples/poi_types.json).


## POI search near a reference position

To find POIs within a given radius from a specified reference
position, server can be accessed via `/v1/guide` path:

`http://localhost:8553/v1/guide?radius={radius}&limit={limit}&poitype={poitype}&name={name}&search={search}&lng={lng}&lat={lat}`

where

`{limit}` - maximal number of search results

`{radius}` - distance from the reference in meters

`{poitype}` - POI type name

`{name}` - Name of POI to search

`{search}` - a query that is run to find a reference point, the first result is used

`{lng}`, `{lat}` - longitude and latidude, respectively.

Given POI type is considered either as an alias or imported POI
type. Type comparison is done in a case-insensitive manner. POI types
are formed from OSM tags in the form `tag_value`.

List of the current tags and aliases, as used by geocoder-nlp, is given at
[https://rinigus.github.io/osmscout-server/tags](https://rinigus.github.io/osmscout-server/tags).

In addition, POI can be searched by its name. For example, you could
search for the restaurant by its name. This parameter is only
supported by Geocoder-NLP backend.

It is required, that either POI type, POI name, or the type and name
are specified in the query.

The reference point can be given either as a query ("Paris") or as a
location coordinates. If the both forms are given in URL, location
coordinates are preferred.

The result is given in JSON format. It returns a JSON object with two
keys: `"origin"` (coordinates of the reference point used in the
search) and `"results"` (array with the POIs). See
[example](examples/guide.json) for details.


## Routing

There are two versions of routing protocol that have to be used in
accordance with the used backend. Version 2 (`v2/route`) is used by
Valhalla and uses Valhalla's API. Version 2 is supported, in part, by
libosmscout as well. Older Version 1 (`v1/route`) is used by
libosmscout and is described in [OLD_API](README.older_api.md). 


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
server will follow Version 1 protocol with an additional flag `API version` 
set to `libosmscout V1` in the response of the server. Using
this flag, the client application can determine whether version 1
protocol response has been used.



## Translations

The translations were contributed by

- Carmen F. B. @carmenfdezb: Spanish
- Lukáš Karas @Karry: Czech
- Åke Engelbrektson @eson57: Swedish
- Ricardo Breitkopf @monkeyisland: German
- Nathan Follens @pljmn: Dutch (NL and BE)
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

