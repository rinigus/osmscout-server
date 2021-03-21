# OSM Scout Server

[![Matrix](https://img.shields.io/badge/matrix.org-%23pure--maps-blue)](https://matrix.to/#/#pure-maps:matrix.org)
[![TMO](https://img.shields.io/badge/forum-T.M.O-FB9200)](https://talk.maemo.org/showthread.php?t=100442)

OSM Scout server can be used as a drop-in replacement for online map
services providing map tiles, search, and routing. As a result, an
offline operation is possible if the device has a server and map
client programs installed and running.

Among other services, the server can be used to provide:
* vector or raster tiles for other applications;
* search for locations and free text search;
* search for POIs next to a reference point or route;
* calculating routes between given sequence of points;

User's guide is available at https://rinigus.github.io/osmscout-server .

The server supports:
* map rendering via Mapnik (https://github.com/mapnik/mapnik);
* hosting of Mapbox GL vector tiles (https://github.com/mapbox/awesome-vector-tiles);
* search via Geocoder-NLP (https://github.com/rinigus/geocoder-nlp) which is based on libpostal (https://github.com/openvenues/libpostal);
* routing instructions via Valhalla (https://github.com/valhalla/valhalla);
* map rendering, search, and routing via libosmscout (http://libosmscout.sourceforge.net/).

To use the server, you have to start it and configure the client to
access it. An example configurations for JavaScript-based clients are
provided under "example" folder. At present, Pure Maps, uNav, and
modRana include plugins already in the upstream and no additional
configuration is needed.

The server is written using Qt. The server consists of a daemon and
separate GUI application for Sailfish, Kirigami, Ubuntu Touch, or a
QtQuick application. Installation instructions are given in
[INSTALL](INSTALL.md).


## User feedback

There are three main communication channels with the users: GitHub,
Matrix channel
[#pure-maps:matrix.org](https://matrix.to/#/#pure-maps:matrix.org) and
a thread at
[TMO](http://talk.maemo.org/showthread.php?t=97823). Matrix channel is
shared with Pure Maps.

Please use Github issues to address specific problems and development
requests. General discussion is expected either through corresponding
issues, Matrix channel, or TMO thread. Please note that users from all
platforms are welcome at TMO, not only current Sailfish OS users.


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

UK postal codes downloaded from https://www.freemaptools.com. These
data contain: Ordnance Survey data © Crown copyright and database
right 2017; Royal Mail data © Royal Mail copyright and database right
2017; National Statistics data © Crown copyright and database right
2017.


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


## API and Adding support for OSM Scout Server to map clients

Exported API and what to consider while adding support for the server
by a map client, see separate [README.api](README.api.md).


## Artwork

Custom icons used by the application were contributed by Fellfrosch
(from talk.maemo.org).


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
- @Watchmaker: Italian


For translations, please see https://github.com/rinigus/osmscout-server/blob/master/translations/README.md


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
