
# Importing maps and their distribution

Import is implemented using Podman containers. Containers are used to download 
PBF files, split them into regions and import into the formats recognized 
by OSM Scout Server.

## Import

At the moment of writing, all datasets used by OSM Scout Server are imported as described
in this README. Exceptions are libpostal (old training datasets are used) and Mapnik (not
imported actively anymore, but old data is distributed).

### Prepare containers

Configure the containers by making a file `.env` by using `.env.template` as a
template. Among other settings, directories where to store the data, and whether
to import full planet or a region.

When directories are determined in `.env`, run a script that will create them
for you, will build and pull all required containers:
```
./prepare_containers.sh
```

To force rebuild of the containers, use `--rebuild` option.

### Download data

To download new map data:
```
./get-data.sh
```

Note that if `STORE_PLANET` folder (as defined in .env) has already a file with the same name as the downloaded
one, it will assume that you don't need to download it again. To refresh the dataset, remove all
files in `STORE_PLANET`.

### Import

Import the maps by running:
```
./import.sh
```

This will trigger the import into the different formats used by the server. To reduce the stress on the server, the
imports are performed one after another. Expect that the process will take few days.

It is possible to import Nominatim in another server separately. In this case, set `NOMINATIM_IMPORT_SEPARATE`
to `true` and define corresponding connection values in `.env`. Import Nominatim using
```
./import-nominatim.sh
```
and wait for it to complete before running `./import.sh`. Note that `./import-nominatim.sh` keeps
Nominatim podman pod running after completion. This is to allow it to connect to while importing geocoder data
by `./import.sh`.

Note that requiered storage during import is about:

- 1.5 TB for Nominatim
- 170 GB for MBTiles
- 80 GB for Planet.pbf
- 35 GB for Valhalla
- For distribution, about 200 GB

### Distribution

To distribute maps:

* Increase URL id for geocoder-nlp, and others in `distribution.json`

* Check that bucket_name is correct

* Check location of `countries.json`, this will have to be used when calling `prepare_distribution.py`. The file should be in `STORE_MISC`
  folder
  
* Run preparation scripts:
```
./prepare_distribution.py --countries data/misc/countries.json
./check_import.py
```

* Test the maps by serving them using Python:
```
(cd public_http && python -m http.server 8231)
```
and downloading them by OSM Scout Server (you would have to adjust server URL used by
application in file `url.json` of your maps directory and setting `development_disable_url_update=1` in the application configuration file). 

* If all is good, upload it using `./uploader.sh`
  
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


## Other scripts

* pack.sh - packs created libosmscout and geocoder-nlp datasets using
  bzip2 compression

* misc/get_base_from_geofabrik.py - used to build initial hierarchy

* misc/poly2json-hierarchy.sh - generates poly.json from poly in hierarchy
  
* misc/prepare_postal.sh - rsyncs postal global and postal country datasets
  into distribution. Split into countries for geocoder-nlp is done in
  geocoder-nlp import scripts
  
* misc/prepare_postal_tags.py - used for distribution of initial
  postal_country files in hierarchy

* misc/update_distribution.py - used on the distribution server to update
  maps distribution from S3 cloud / HTTP server. See note on the top
  of the script on how to specify mirror path and URL
  
* misc/update_distribution_wrapper.sh - used on the distribution server to
  update distribution from S3 cloud / HTTP server. See the script for
  configuration options. This is a wrapper that is expected to be used
  in cron script to run update_distribution.py
