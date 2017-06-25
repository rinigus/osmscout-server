
# Importing maps and their distribution

These scripts are used to download PBF files, split them into regions
and import into the formats recognized by OSM Scout Server.


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


## Scripts

Information below is for developers and is not required to add or
change imported regions.

### On every import

* Increase URL id for libosmscout, geocoder-nlp (and if needed for
  others) in prepare_distribution.py
  
* Check that bucket_name is correct

* [TODO: Add Valhalla and Mapnik global instructions]

* Run ./import_master.sh 


### On backend upgrade

In case if there is a new liboscmscout or geocoder-nlp version:

* run ./prepare_backends.sh

* update versions in the end of build.sh script

* update versions in the end of build_mapnik.sh script

* update version for Valhalla in valhalla_country_pack.py module

* check for compatibility in mapmanagerfeature.cpp of the main tree


### Other scripts

* build.sh - performs import for each PBF

* get_base_from_geofabrik.py - used to build initial hierarchy

* pack.sh - packs created libosmscout and geocoder-nlp datasets using
  bzip2 compression
  
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
  
