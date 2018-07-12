## Description of older APIs

While APIs described here are supported, the developers are encouraged
to use APIs described in the main [README](README.md).

### Routing, Version 1: libosmscout

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
