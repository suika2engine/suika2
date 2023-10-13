#!/bin/sh

set -eu

#
# Welcome.
#
echo ""
echo "Hello, this program will release the template games."

#
# Guess the version number.
#
VERSION=`grep -a1 '<!-- BEGIN-LATEST -->' ../doc/readme-jp.html | tail -n1`
VERSION=`echo $VERSION | cut -d '>' -f 2 | cut -d ' ' -f 1`
VERSION=`echo $VERSION | cut -d '/' -f 2`

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
    echo "Warning: Please specify FTP_LOCAL in build/.env";
    echo "         Release files will be copied to this directory.";
    echo "         We will use `pwd`/ftp directory instead.";
    FTP_LOCAL=`pwd`/ftp;
    mkdir -p "$FTP_LOCAL";
fi
DO_UPLOAD=1
if [ -z "$FTP_USER" ]; then
    echo "Warning: Please specify FTP_USER in build/.env";
    echo "         This information is utilized to upload release files.";
    echo "         We will not upload release files this time.";
    DO_UPLOAD=0;
fi
if [ -z "$FTP_PASSWORD" ]; then
    echo "Warning: Please specify FTP_PASSWORD in build/.env";
    echo "         This information is utilized to upload release files.";
    echo "         We will not upload release files this time.";
    DO_UPLOAD=0;
fi
if [ -z "$FTP_URL" ]; then
    echo "Warning: Please specify FTP_URL in build/.env";
    echo "         This information is utilized to upload release files.";
    echo "         We will not upload release files this time.";
    DO_UPLOAD=0;
fi

#
# Extract the latest ZIP file.
#
mkdir -p tmp
cd tmp
unzip "$FTP_LOCAL/suika2-$VERSION.zip"

#
# Create and upload templates.
#
for d in ../../doc/templates/*; do
    cp -R "$d" .;
    BASE=$(basename $d);
    cp suika2/suika.exe "./$BASE/";
    cp suika2/suika-pro.exe "./$BASE/";
    cp suika2/mac.dmg "./$BASE/";
    cp suika2/mac-pro.dmg "./$BASE/";
    cp -R suika2/tools "./$BASE/";
    zip -r "$BASE.zip" "$BASE"
    cp "$BASE.zip" "$FTP_LOCAL/sample/"
    if [ "$DO_UPLOAD" -eq "1" ]; then
	curl -T "$BASE.zip" -u "$FTP_USER:$FTP_PASSWORD" "$FTP_URL/sample/$BASE.zip";
    fi;
done

#
# Remove tmp.
#
cd ..
rm -rf tmp
