#!/bin/sh

set -eu

# Change this line to create your own release file.
SIGNATURE='Developer ID Application: Keiichi TABATA'

# Create mac.dmg
rm -rf tmp mac.dmg
mkdir tmp
cp -Rv suika.app tmp/
hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac mac.dmg
codesign --sign "$SIGNATURE" mac.dmg
rm -rf tmp
