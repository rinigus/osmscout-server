#!/bin/bash

# repo folder and mirror url have to be also specified in a separate
# files. See update_distribution.py
REPO_FOLDER=YOUR_FOLDER_WHERE_HTTP_IS_SERVED

# specify folder containing osmscout-server git clone, mirror_url, and mirror_path 
CODE_FOLDER=FOLDER_NAME_HERE

LOCKFILE=/tmp/osm_scout_server_sync_lock.txt

if [ -e ${LOCKFILE} ] && kill -0 `cat ${LOCKFILE}`; then
    echo "Already running"
    exit
fi

# make sure the lockfile is removed when we exit and then claim it
trap "rm -f ${LOCKFILE}; exit" INT TERM EXIT
echo $$ > ${LOCKFILE}

cd ${CODE_FOLDER}

# if needed, specify some python path containing requests module (PYTHONPATH=requests)
./osmscout-server/scripts/import/update_distribution.py && find ${REPO_FOLDER} -type d -empty -delete

echo "OSM Scout Server repository size & free space:"
echo ""
du -d1 -h ${REPO_FOLDER}
echo ""
df -h ${REPO_FOLDER}

rm -f ${LOCKFILE}
