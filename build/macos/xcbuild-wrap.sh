#!/bin/sh

set -eu

# Change this line to create your own release file.
SIGNATURE='Developer ID Application: Keiichi TABATA'

# Check for the symbolic link to the cloud storage.
[ -e ../cloud ]

# Get the target name.
APP=$1

# Build an archive.
xcodebuild \
    -scheme $APP \
    -project suika.xcodeproj \
    -configuration Release \
    -archivePath $PWD/build/Release/$APP.xcarchive \
    archive

# Upload the archive and ask for notarization.
xcodebuild \
    -exportArchive \
    -archivePath $PWD/build/Release/$APP.xcarchive \
    -exportOptionsPlist export-options.plist

# Wait and export the notarized app.
rm -rf build/Release/$APP.app
until \
    xcodebuild \
	-exportNotarizedApp \
	-archivePath $PWD/build/Release/$APP.xcarchive \
	-exportPath $PWD/build/Release; \
do \
    echo waiting...; \
    sleep 10; \
done

# Make a zip file.
SUFFIX=${APP#suika}
rm -rf tmp mac$SUFFIX.dmg
mkdir tmp
cp -Rv build/Release/$APP.app tmp/
hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac$SUFFIX mac$SUFFIX.dmg
codesign --sign "$SIGNATURE" mac$SUFFIX.dmg
rm -rf tmp
cp mac$SUFFIX.dmg ../cloud/
rm mac$SUFFIX.dmg
