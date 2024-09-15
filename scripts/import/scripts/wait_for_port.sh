#!/bin/sh

set -e

# Check options
if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <host> <port>"
  exit 1
fi

host="$1"
port="$2"

while ! nc $host $port; do
    echo "Waiting for $host:$port to be ready..."
    sleep 10s;
done;
sleep 10s
echo "$host:$port is ready!"
exit 0
