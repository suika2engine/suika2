#!/bin/sh

set -eu

#
# Welcome.
#
echo ""
echo "Hello, this program will release the template games."

#
# Load FTP credentials from .env file.
#
echo "Checking for .env credentials."
FTP_LOCAL=""
FTP_USER=""
FTP_PASSWORD=""
FTP_URL=""
if [ -e .env ]; then
	eval `cat .env`;
fi
if [ -z "$FTP_LOCAL" ]; then
    echo "Error: Please specify FTP_LOCAL in build/.env";
    exit 1;
fi
if [ -z "$FTP_USER" ]; then
    echo "Error: Please specify FTP_USER in build/.env";
    exit 1;
fi
if [ -z "$FTP_PASSWORD" ]; then
    echo "Error: Please specify FTP_PASSWORD in build/.env";
    exit 1;
fi
if [ -z "$FTP_URL" ]; then
    echo "Error: Please specify FTP_URL in build/.env";
    exit 1;
fi

#
# Create and upload templates.
#
for d in ../doc/templates/*; do
    cp -R "$d" .;
    BASE=$(basename $d);
    mkdir -p "./$BASE/anime" "./$BASE/wms" "./$BASE/rule" "./$BASE/cv"
    touch "./$BASE/game.suika2project";
    zip -r "$BASE.zip" "$BASE"
    cp "$BASE.zip" "$FTP_LOCAL/sample/"
    curl -T "$BASE.zip" -u "$FTP_USER:$FTP_PASSWORD" "$FTP_URL/sample/$BASE.zip";
done

#
# Remove tmp.
#
cd ..
rm -rf tmp
