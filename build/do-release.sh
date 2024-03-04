#!/bin/sh

# Stop when failed.
set -eu

#
# Show a welcome message.
#
echo ""
echo "Hello, this is the Suika2 Release Manager."

#
# Check if we run on a macOS host.
#
if [ -z "`uname -a | grep Darwin`" ]; then
    echo "Error: please run on macOS.";
    exit 1;
fi

#
# Check for GNU coreutils.
#
SED='sed'
if [ ! -z "`which gsed`" ]; then
    SED='gsed';
fi
HEAD='head'
if [ ! -z "`which ghead`" ]; then
    HEAD='ghead';
fi

#
# Guess the release version number.
#
VERSION=`grep -a1 '<!-- BEGIN-LATEST-JP -->' ../ChangeLog | tail -n1`
VERSION=`echo $VERSION | cut -d '>' -f 2 | cut -d ' ' -f 1`
VERSION=`echo $VERSION | cut -d '/' -f 2`

#
# Get the release notes.
#
NOTE_JP=`cat ../ChangeLog | awk '/BEGIN-LATEST-JP/,/END-LATEST-JP/' | tail -n +2 | $HEAD -n -1`
NOTE_EN=`cat ../ChangeLog | awk '/BEGIN-LATEST-EN/,/END-LATEST-EN/' | tail -n +2 | $HEAD -n -1`

#
# Do an interactive confirmation.
#
echo "Are you sure you want to release version $VERSION?"
echo ""
echo "[Japanese Note]"
echo "$NOTE_JP"
echo ""
echo "[English Note]"
echo "$NOTE_EN"
echo ""
echo "(press enter to proceed)"
read str

#
# Build "suika.exe".
#
echo "Building suika.exe"
cd engine-windows
rm -f *.o
if [ ! -e libroot ]; then
    ./build-libs.sh;
fi
make -j20
sign.sh suika.exe
cd ..

#
# Build the "Suika.app".
#
echo "Building Suika.app (suika-mac.dmg)."
cd engine-macos
rm -f suika-mac.dmg suika-mac-nosign.dmg
make suika-mac.dmg
cp suika-mac.dmg suika-mac-nosign.dmg
codesign --sign 'Developer ID Application: Keiichi TABATA' suika-mac.dmg
cd ..

#
# Build the Wasm files.
#
echo "Building Wasm files."
cd engine-wasm
make clean
make
cd ..

#
# Build the iOS source tree.
#
echo "Building iOS source tree."
cd engine-ios
make src
cd ..

#
# Build the Android source tree.
#
echo "Building Android source tree."
cd engine-android
make debug
make src
cd ..
cd pro-android
make
cd ..

#
# Build "suika-pro.exe".
#
echo "Building suika-pro.exe"
cd pro-windows
rm -f *.o
if [ ! -e libroot ]; then
    cp -Rav ../engine-windows/libroot .;
fi
make -j20 VERSION="$VERSION"
sign.sh suika-pro.exe
cd ..

#
# Build "web-test.exe"
#
echo "Building web-test.exe"
cd ../tools/web-test
make
cd ../../build

#
# Make an installer for Windows.
#
echo "Creating an installer for Windows."

# /
cp -v pro-windows/suika-pro.exe installer-windows/suika-pro.exe

# /games
rm -rf installer-windows/games
find ../games -name '.*' | xargs rm
mkdir installer-windows/games
cp -Rv ../games/japanese installer-windows/games/
cp -Rv ../games/english installer-windows/games/
cp -Rv ../games/nvl installer-windows/games/
cp -Rv ../games/nvl-tategaki installer-windows/games/
cp -Rv ../games/nvl-en installer-windows/games/

# /tools
rm -rf installer-windows/tools
mkdir -p installer-windows/tools
cp -v engine-windows/suika.exe installer-windows/tools/
cp -v engine-macos/suika-mac.dmg installer-windows/tools/
cp -Rv engine-android/android-src installer-windows/tools/android-src
cp -Rv engine-ios/ios-src installer-windows/tools/ios-src
mkdir -p installer-windows/tools/web
cp -v engine-wasm/html/index.html installer-windows/tools/web/index.html
cp -v engine-wasm/html/index.js installer-windows/tools/web/index.js
cp -v engine-wasm/html/index.wasm installer-windows/tools/web/index.wasm
cp -v ../tools/web-test/web-test.exe installer-windows/tools/web-test.exe
cp -Rv ../tools/installer installer-windows/tools/installer
cp -v ../tools/snippets/jp-normal/plaintext.code-snippets installer-windows/plaintext.code-snippets.jp
cp -v ../tools/snippets/en-normal/plaintext.code-snippets installer-windows/plaintext.code-snippets.en

# Make an installer
cd installer-windows
make
sign.sh suika2-installer.exe

# Also, make a zip
make zip
cd ..

#
# Build "Suika2 Pro.app".
#
echo "Building Suika2 Pro.app (suika2.dmg)"
cd pro-macos
rm -f suika2.dmg
make
cd ..

#
# Upload.
#
echo "Uploading files."

ftp-upload.sh installer-windows/suika2-installer.exe "dl/suika2-$VERSION.exe"
ftp-upload.sh installer-windows/suika2.7z "dl/suika2-$VERSION.7z"
ftp-upload.sh pro-macos/suika2.dmg "dl/suika2-$VERSION.dmg"
echo "Upload completed."

#
# Update the Web site.
#
echo ""
echo "Updating the Web site."
SAVE_DIR=`pwd`
cd ../web && \
    ./templates.sh && \
    ./version.sh && \
    ftp-upload.sh index.html && \
    ftp-upload.sh dl/index.html && \
    ftp-upload.sh en/index.html && \
    ftp-upload.sh en/dl/index.html && \
    ./push.sh
cd "$SAVE_DIR"

#
# Restore a non-signed dmg for a store release.
#
mv engine-macos/suika-mac-nosign.dmg engine-macos/suika-mac.dmg

#
# Post to the Discord server.
#
discord-release-bot.sh

#
# Upload to GitHub.
#
git add -u
git commit -m "release: Suika2/$VERSION" || true
git tag -a "v2.$VERSION" -m "release"
git push github "v2.$VERSION"
yes "" | gh release create "v2.$VERSION" --title "v2.$VERSION" ~/Sites/suika2.com/dl/suika2-$VERSION.exe ~/Sites/suika2.com/dl/suika2-$VERSION.dmg

#
# Finish.
#
echo "Finished. $VERSION was released!"
echo "$NOTE_JP"
