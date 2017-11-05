# Mapbox GL tiles

Mapbox GL map import is based on [OpenMapTiles schema](https://github.com/openmaptiles/openmaptiles). The schema will be developed further in future and proposed upstream.

In contrast to OpenMapTiles approach, this import procedure does not require docker images and is using the database accessible directly. The scripts from OpenMapTiles repositories have been adjusted, if needed.

## Preparation of PostGIS database

Install PostgreSQL, PostGIS using your distribution.

### Initialize PostGIS

To initialize local PostGIS database:

* In this directory, run to create database storage space (or replace current directory argument with something else where you have space): ```initdb `pwd`/db```

* Create socket directory, here subdirectory is used for that: ```mkdir `pwd`/pg-socket```

* Edit socket directory setting in `db/postgresql.conf` by adjusting
  * `unix_socket_directories` should point to the created socket directory
  * `port` for your PostgreSQL instance should be adjusted

* If you wish, adjust security settings in `db/pg_hba.conf`

* Start PostgreSQL: ```postgres -D `pwd`/db```

* Adjust environment variables in `env.sh`
