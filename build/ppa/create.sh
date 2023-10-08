#!/bin/sh

set -eu

# Get the version of Suika2.
VERSION=`grep -a1 '<!-- BEGIN-LATEST -->' ../../doc/readme-jp.html | tail -n1`
VERSION=`echo $VERSION | cut -d '>' -f 2 | cut -d ' ' -f 1`
VERSION=`echo $VERSION | cut -d '/' -f 2`

# Get the minor version.
MINOR=1
if [ $# != 0 ]; then
	MINOR="$1";
fi

# Update the changelog
if [ -z "`grep $VERSION meta/debian/changelog`" ]; then
    echo "suika2 ($VERSION-1) jammy; urgency=medium" > meta/debian/changelog.new;
    echo '' >> meta/debian/changelog.new;
    echo '  * Sync upstream' >> meta/debian/changelog.new;
    echo '' >> meta/debian/changelog.new;
    echo " -- Keiichi Tabata <tabata@luxion.jp>  `date`" >> meta/debian/changelog.new;
    echo '' >> meta/debian/changelog.new;
	cat meta/debian/changelog >> meta/debian/changelog.new;
	mv meta/debian/changelog.new meta/debian/changelog;
fi

# Make a directory and enter it.
rm -rf work
mkdir work
cd work

# Create a source tarball.
SAVE_DIR=`pwd`
cd ../../../
git archive HEAD --output=build/ppa/work/suika2_$VERSION.orig.tar.gz
cd "$SAVE_DIR"

# Make a sub-directory with version number, and enter it.
rm -rf suika2-$VERSION
mkdir suika2-$VERSION
cp -R ../meta/debian suika2-$VERSION/
cd suika2-$VERSION
tar xzf ../suika2_$VERSION.orig.tar.gz

# Build a source package.
debuild -S -sa
cd ..

# Sign.
debsign -k E9C18AA6087AA39F5114E668EEB70B9FAF1F01C5 "suika2_${VERSION}-${MINOR}_source.changes"

# Upload.
dput ppa:ktabata/ppa suika2_${VERSION}-${MINOR}_source.changes 
cd ../

# Cleanup.
rm -rf work
