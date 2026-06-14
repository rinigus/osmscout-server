#!/bin/bash

set -e

# This script only runs if the database does not exist

# Set 2 additional parameters on import
# https://nominatim.org/release-docs/latest/admin/Installation/#tuning-the-postgresql-database

echo "Running import.sh since the database does exist.  Running as:"
whoami
printf "\n# for Import\nfsync = off\nfull_page_writes = off\n" >> /etc/postgresql/postgresql.conf
