FROM postgis/postgis:15-3.4-alpine

# Faster imports with PostgreSQL 15
# https://osm2pgsql.org/news/2023/01/22/faster-with-postgresql15.html

# Nominatim recommended configuration
COPY --chown=postgres:postgres postgresql.conf /etc/postgresql/postgresql.conf
# Configure 2 parameters for database import
COPY --chmod=0755 import.sh /docker-entrypoint-initdb.d/

CMD [ "-c", "config_file=/etc/postgresql/postgresql.conf" ]
