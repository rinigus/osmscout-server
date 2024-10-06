
# Importing maps and their distribution

Import is implemented using Docker containers. Containers are used to download 
PBF files, split them into regions and import into the formats recognized 
by OSM Scout Server.

## Hierarchy

Hierarchy of administrative regions is given in hierarchy
subfolder. Its based on the representation of the World by Geofabrik,
as of beginning of March 2017. 

Each region could have the following files:

* name
* poly - if PBF for the region has to be generated
* ignore - this region will not be imported to the server, used for
  organizing split of regions
* postal_country - specifies which region is used. NB! if missing, the
  parent region postal_country will be used
  
POLY files are imported from Geofabrik. As an example additional
region, Barcelona is demonstrated.

If some POLY file is missing, it can be generated using
http://polygons.openstreetmap.fr . Find corresponding OSM relationship
for the border of interest and use the relationship ID to generate
POLY file. It allows also to simply the polygon, use it to reduce
processing on import.


## Import

### Prepare containers

Configure the containers by making a file `.env` by using `.env.template` as a
template. Among other settings, set your User ID, directories where to
store the data, and whether to import full planet or a region.

Next, build the containers:
```
docker compose build
```

### Import

Import the maps by running:
```
docker compose up
```

This will start containers defined in the 
[docker compose file](docker-compose.yaml) and trigger the import into the
different formats used by the server. To reduce the stress on the server, the
imports are performed one after another. Expect that the process will take
few days.

If import is finished successfully, the last container will shutdown Nominatim
database used at one of the phases of the import.

Note that requiered storage during import is about:

- 1.5 TB for Nominatim
- 150 GB for MBTiles
- 80 GB for Planet.pbf
- 35 GB for Valhalla
- For distribution, about 110 GB

### Distribution

To distribute maps:

* Increase URL id for geocoder-nlp, and others in
  `prepare_distribution.py`
  
* Check that bucket_name is correct

* Run preparation scripts:
```
./prepare_distribution.py
./check_import.py
```

* Test the maps by serving them using Python:
```
(cd public_http && python -m http.server 8231)
```
and downloading them by OSM Scout Server (you would have to adjust server URL used by
application in file `url.json` of your maps directory and setting `development_disable_url_update=1` in the application configuration file). 

* If all is good, upload it using `./uploader.sh`
  
### Other scripts

* build_{geocoder,mapnik,osmscout}.sh - performs import for each region

* get_base_from_geofabrik.py - used to build initial hierarchy

* pack.sh - packs created libosmscout and geocoder-nlp datasets using
  bzip2 compression

* poly2json-hierarchy.sh - generates poly.json from poly in hierarchy
  
* prepare_countries.py, prepare_distribution.py, prepare_splitter.py - used by importer_master.sh

* prepare_postal.sh - rsyncs postal global and postal country datasets
  into distribution. Split into countries for geocoder-nlp is done in
  geocoder-nlp import scripts
  
* prepare_postal_tags.py - used for distribution of initial
  postal_country files in hierarchy

* prepare_osmium.sh - installs new version of osmium and libosmium
  used to split planet into regions
  
* update_distribution.py - used on the distribution server to update
  maps distribution from S3 cloud / HTTP server. See note on the top
  of the script on how to specify mirror path and URL
  
* update_distribution_wrapper.sh - used on the distribution server to
  update distribution from S3 cloud / HTTP server. See the script for
  configuration options. This is a wrapper that is expected to be used
  in cron script to run update_distribution.py
