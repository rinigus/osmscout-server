# Examples 

Here examples of queries and integration of server with the different
platforms are shown.

Example queries were run using a map of Estonia. Below, the URLs used to get
example results and the links to the response are listed.

*Search V1*

Query: `http://localhost:8553/v1/search?limit=10&search=Tallinn` ; Result: [search_v1](search_v1.json)

*Search V2*

Query: `http://localhost:8553/v2/search?limit=3&search=tartu mnt 1, tallinn` ; Result: [search_v2](search_v2.json)

*List of available POI types*

Query: `http://localhost:8553/v1/poi_types` ; Result: [poi_types](poi_types.json)

*Guide*

Query: `http://localhost:8553/v1/guide?radius=1000&limit=3&poitype=cafe&name=kohvik&search=pühavaimu, tallinn` ; Result: [guide](guide.json)


*Route V1*

Query: `http://localhost:8553/v1/route?p[0][search]=Tallinn&p[1][search]=Tartu` ; Result: [route_v1](route_v1.json)
