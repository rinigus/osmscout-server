#!/bin/sh

set -e

# Check if arguments are provided
if [ $# -lt 2 ]; then
    echo "Usage: $0 <directory> <url1> [url2] [url3] ..."
    exit 1
fi

TARGET_DIR=$1
shift

mkdir -p $TARGET_DIR
cd $TARGET_DIR

for URL in "$@"; do
  echo "Processing URL: $URL"

  # Get the filename from the URL
  FILENAME=$(basename "$URL")

  # Check if the file already exists in the target directory
  if [ ! -f "$TARGET_DIR/$FILENAME" ]; then
    echo "Downloading $URL to $TARGET_DIR/$FILENAME"
    wget $URL
  else
    echo "File already exists: $TARGET_DIR/$FILENAME"
  fi
done

