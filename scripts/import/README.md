# Importing and Distributing Maps

The import process uses Podman containers. The containers download area PBF file
together with other data, and import them into the formats used by OSM Scout
Server.

At the time of writing, all datasets used by OSM Scout Server are imported as
described in this README, except for libpostal, which uses old training
datasets, and Mapnik, which is no longer actively imported but whose old data
is still distributed.

## Prerequisites

The import host needs:

* Podman
* Python 3
* `s3cmd`, `md5deep`, and `bzip2` for preparing and uploading distribution data
* enough RAM and storage for the selected import area

Run the commands below from this directory.

## Settings

Create `.env` from `.env.template`. Import settings are defined and documented
there, including the imported area, data directories, memory limits, and whether
Nominatim is imported together with the rest of the data or separately.

The main storage settings are:

* `STORE_PLANET` - downloaded PBF files and auxiliary datasets
* `STORE_OUTPUT` - root directory for generated data
* `STORE_IMPORTED` - final imported datasets, under `STORE_OUTPUT` by default
* `STORE_MBTILES`, `STORE_NOMINATIM`, `STORE_VALHALLA`, and `STORE_MISC` -
  intermediate import outputs, under `STORE_OUTPUT` by default

`.env` is sourced by shell scripts, so values must be valid shell assignments.
Avoid paths with spaces.

## Prepare Containers

After configuring `.env`, run the preparation script. It creates the configured
directories and builds or pulls all required container images:

```
./prepare_containers.sh
```

To force a rebuild of the images, use the `--rebuild` option.

## Download Data

Downloading data before import is optional for the all-in-one import mode. If
this step is skipped, `./import.sh` downloads the required data at the start of
the import.

For a separate Nominatim import, run this step first because
`./import-nominatim.sh` expects the PBF and auxiliary data to be available.

To download new map data:

```
./get-data.sh
```

Existing downloads are reused. The main PBF is skipped if
`${STORE_PLANET}/${PBF}` already exists. Nominatim auxiliary files are skipped
individually if they already exist, and libpostal data is skipped if the
libpostal data directory already exists. To force fresh downloads, remove or
move the corresponding files or directories from `STORE_PLANET`.

## Import

There are two supported import modes.

### All-in-One Import

To import everything in one run, keep `NOMINATIM_IMPORT_SEPARATE=false` in
`.env` and run:

```
./import.sh
```

This downloads missing input data and imports it into the formats used by the
server. To reduce server load, imports are performed sequentially. Expect the
process to take a few days for large imports.

### Separate Nominatim Import

Nominatim can be imported separately, optionally on another server. In this
mode, first run `./get-data.sh`, then import Nominatim:

```
./import-nominatim.sh
```

The script leaves the Nominatim Podman pod running after completion so that the
rest of the import can connect to it while importing geocoder data. When run
separately, Nominatim exposes PostgreSQL on `${POSTGRES_PORT:-15432}`.

After Nominatim import completes, set `NOMINATIM_IMPORT_SEPARATE=true` and set
`NOMINATIM_DATABASE_SERVER` in `.env` to the reachable database host and port,
for example `127.0.0.1:15432` or `server.example.org:15432`. Then run:

```
./import.sh
```

## Reimporting and Retrying

Several import stages skip work when their output already exists. This includes
downloads, Planetiler MBTiles, Nominatim database setup, GeocoderNLP import
metadata, MBTiles and Valhalla packaging, Valhalla tile package metadata, and
`countries.json` generation.

Before a full reimport, move old output data out of the way or remove the
corresponding directories and files from `STORE_OUTPUT`, `STORE_IMPORTED`, and
the intermediate output directories. If an import fails and leaves a Podman pod
running, remove that pod before retrying.

Approximate storage required during import:

* 1.5 TB for Nominatim
* 170 GB for MBTiles
* 80 GB for Planet PBF
* 35 GB for Valhalla
* about 200 GB for the prepared distribution directory

## Distribution

To distribute maps:

* Increase the URL version IDs for `geocoder_nlp` and other changed datasets in
  `distribution.json`.

* Check that `bucket_name` contains the correct S3 bucket name.

* Check the location of `countries.json`. This file is generated in
  `STORE_MISC` and must be passed to `prepare_distribution.py`.

* Run the preparation scripts:

```
./prepare_distribution.py --countries data/output/misc/countries.json
./check_import.py
```

`prepare_distribution.py` generates `provided/countries_provided.json`,
`public_http`, and `uploader.sh`. The generated uploader uses `.s3cfg` from the
current directory.

* Test the maps by serving the generated `public_http` directory with Python:

```
(cd public_http && python -m http.server 8231)
```

Then configure OSM Scout Server to use the local URL, for example by adjusting
`url.json` in the maps directory to point to `http://localhost:8231` and setting
`development_disable_url_update=1` in the application configuration file.

* If everything looks good, review and run the generated `./uploader.sh`.

## Hierarchy

The hierarchy of administrative regions is stored in the `hierarchy` subfolder.
It is based on Geofabrik's representation of the world as of early March 2017.

Each region can contain the following files:

* `name` - region display name
* `poly` - polygon used to generate the region PBF
* `ignore` - marks a region that should not be imported; used only to organize
  region splitting
* `postal_country` - specifies which postal country dataset to use. If missing,
  the parent region's `postal_country` is used.

POLY files are imported from Geofabrik. Barcelona is included as an example of
an additional custom region.

If a POLY file is missing, it can be generated at
`http://polygons.openstreetmap.fr`. Find the OSM relation for the border of
interest and use its relation ID to generate the POLY file. The service can also
simplify polygons, which can reduce import processing time.

## Other Scripts

* `pack.sh` - packs created libosmscout and geocoder-nlp datasets using bzip2
  compression

* `misc/get_base_from_geofabrik.py` - used to build the initial hierarchy

* `misc/poly2json-hierarchy.sh` - generates `poly.json` from `poly` files in
  `hierarchy`

* `misc/prepare_postal.sh` - rsyncs postal global and postal country datasets
  into distribution. The split into countries for geocoder-nlp is done in the
  geocoder-nlp import scripts.

* `misc/prepare_postal_tags.py` - used for distribution of initial
  `postal_country` files in `hierarchy`

* `misc/update_distribution.py` - used on the distribution server to update map
  distribution from an S3 cloud or HTTP server. See the note at the top of the
  script for how to specify the mirror path and URL.

* `misc/update_distribution_wrapper.sh` - used on the distribution server to
  update distribution from an S3 cloud or HTTP server. See the script for
  configuration options. This is a wrapper expected to be used in a cron script
  to run `update_distribution.py`.
