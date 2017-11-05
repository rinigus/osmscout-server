# Mapbox GL tiles

Mapbox GL map import is based on [OpenMapTiles schema](https://github.com/openmaptiles/openmaptiles).

In contrast to OpenMapTiles approach, this import procedure does not require docker images and is using the directly accessible database. The scripts from OpenMapTiles repositories have been adjusted, if needed. All import is done in Linux, but similar approach could work on other platforms as well.

## Preparation of PostGIS database

Install PostgreSQL, PostGIS using your distribution.

To initialize local PostGIS database:

* In this directory, run to create database storage space (or replace current directory argument with something else where you have space): ```initdb `pwd`/db```

* Create socket directory, here subdirectory is used for that: ```mkdir `pwd`/pg-socket```

* Edit socket directory setting in `db/postgresql.conf` by adjusting
  * `unix_socket_directories` should point to the created socket directory
  * `port` for your PostgreSQL instance should be adjusted

* If you wish, adjust security settings in `db/pg_hba.conf`

* Start PostgreSQL: ```postgres -D `pwd`/db```

* Adjust environment variables in `env.sh`

* Create the database by running `./create_database.sh`

## Obtaining the required programs

There are several programs that are used during import and are possibly not available in a general Linux distributions. To get the corresponding tools, run `prepare_tools.sh`

## Obtaining the data

In addition to Planet OSM, there are several other datasets used in generation of the tiles. Run `prepare_datasets.sh` to get the datasets.

## Importing data into PostGIS

* Run: `./import_datasets.sh`

* Make a link to the planet.pbf: `ln -s ../planet/planet-latest.osm.pbf .`

* Run: `./import_osm.sh planet-latest.osm.pbf`
