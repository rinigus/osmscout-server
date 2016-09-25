# osmscout-server

Server based on libosmscout. This server is envisioned to be a drop-in
replacement for online map services providing map tiles, search, and
routing. As a result, an offline operation is possible if the device
has a server and map client programs installed and running. At
present, the server can be used to provide map tiles for other
applications.

To use the server, you have to start it and configure the client to
access it. An example configurations for Poor Maps and modRana are
provided under "thirdparty" folder.

The server is written using Qt. The server can be used as a console or
Sailfish application. For console version, use
osmscout-server_console.pro as a project. For Sailfish, use
osmscout-server_silica.pro.

## Maps

Maps provided by OpenSteetsMaps have to be converted to the format used by libosmscout library. See http://libosmscout.sourceforge.net/tutorials/Importing/ for instructions. At this stage, you would have to compile the library on your PC and run the import program, as explained in the tutorial.

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


## Tile server

The server component for providing tiles operates using OSM convention
with small extensions. URL is

http://localhost:8553/v1/tile/{dlight}/{shift}/{scale}/{z}/{x}/{y}.png

where

{dlight} - either 0 for night or 1 for day

{shift} - allows to change used {z} by increasing it to {z}+{shift}

{scale} - size of a tile is {scale}*256

{z}, {x}, and {y} are as in http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames .

Addition of {scale} and {shift} allows to experiment with different
tile sizes to optimize for performance and human-map interaction. See
Poor Maps settings for example.


## Acknowledgments

libosmscout: http://libosmscout.sourceforge.net

osmscout-sailfish: https://github.com/Karry/osmscout-sailfish

QtWebApp HTTP Webserver: http://stefanfrings.de/qtwebapp/index-en.html
