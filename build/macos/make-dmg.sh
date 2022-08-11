#!/bin/sh

set -eu

# Change this line to create your own release file.
SIGNATURE='Developer ID Application: Keiichi TABATA'

echo ""
echo "Checking for the symbolic link to Windows file sharing."
[ -e fileserver ]

# Create mac.dmg
rm -rf tmp mac.dmg
mkdir tmp
cp -Rv suika.app tmp/
hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac mac.dmg
codesign --sign "$SIGNATURE" mac.dmg
rm -rf tmp
cp mac.dmg fileserver/

# Create mac-pro.dmg
rm -rf tmp mac-pro.dmg
mkdir tmp
cp -Rv suika-pro.app tmp/
hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-pro-mac mac-pro.dmg
codesign --sign "$SIGNATURE" mac-pro.dmg
rm -rf tmp
cp mac-pro.dmg fileserver/

rm -rf tmp mac.dmg mac-pro.dmg
