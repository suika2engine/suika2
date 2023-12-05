#!/bin/sh

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
# Load credentials from build/.env file.
#
echo "Checking for build/.env credentials."
if [ ! -e .env ]; then
    echo "Error: please create build/.env file."
    exit 1;
fi
FTP_USER=""
FTP_PASSWORD=""
FTP_URL=""
eval `cat .env`;
if [ -z "$FTP_USER" ]; then
    echo "Error: please specify FTP_USER in build/.env";
    exit 1;
fi
if [ -z "$FTP_PASSWORD" ]; then
    echo "Error: please specify FTP_PASSWORD in build/.env";
    exit 1;
fi
if [ -z "$FTP_URL" ]; then
    echo "Error: please specify FTP_URL in build/.env";
    exit 1;
fi

#
# Guess the release version number.
#
VERSION=`grep -a1 '<!-- BEGIN-LATEST -->' ../doc/readme-jp.html | tail -n1`
VERSION=`echo $VERSION | cut -d '>' -f 2 | cut -d ' ' -f 1`
VERSION=`echo $VERSION | cut -d '/' -f 2`

#
# Get the release notes.
#
NOTE_JP=`cat ../doc/readme-jp.html | awk '/BEGIN-LATEST/,/END-LATEST/' | tail -n +2 | $HEAD -n -1`
NOTE_EN=`cat ../doc/readme-en.html | awk '/BEGIN-LATEST/,/END-LATEST/' | tail -n +2 | $HEAD -n -1`

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
# Update the macOS project version.
#
$SED -i "s/MARKETING_VERSION = .*;/MARKETING_VERSION = $VERSION;/g" engine-macos/suika.xcodeproj/project.pbxproj
$SED -i "s/MARKETING_VERSION = .*;/MARKETING_VERSION = $VERSION;/g" pro-macos/suika.xcodeproj/project.pbxproj
$SED -i "s/MARKETING_VERSION = .*;/MARKETING_VERSION = $VERSION;/g" capture-macos/suika.xcodeproj/project.pbxproj
$SED -i "s/MARKETING_VERSION = .*;/MARKETING_VERSION = $VERSION;/g" replay-macos/suika.xcodeproj/project.pbxproj

#
# Git push (Xcode projects and readmes).
#
git add engine-macos/suika.xcodeproj/project.pbxproj
git add pro-macos/suika.xcodeproj/project.pbxproj
git add capture-macos/suika.xcodeproj/project.pbxproj
git add replay-macos/suika.xcodeproj/project.pbxproj
git add ../doc/readme-jp.html ../doc/readme-en.html
git commit -m "doc: update the version number to $VERSION" || true
git push github master

#
# Build "suika.exe".
#
echo "Building suika.exe"
cd engine-windows-x86
rm -f *.o
if [ ! -e libroot ]; then ./build-libs.sh; fi
make -j8
sign.sh suika.exe
cd ..

#
# Build "suika-pro.exe".
#
echo "Building suika-pro.exe"
cd pro-windows-x86
rm -f *.o
if [ ! -e libroot ]; then cp -Rav ../engine-windows-x86/libroot .; fi
make -j8
sign.sh suika-pro.exe
cd ..

#
# Build "suika.app".
#
echo "Building a macOS main engine."
cd engine-macos
make
cd ..

#
# Build Wasm files.
#
echo "Building Wasm files."
cd engine-wasm
make clean
make
cp html/index.html html/index.js html/index.wasm
cd ..

#
# Build "suika-linux".
#
# echo "Building suika-linux"
# cd engine-linux-x86_64-clang
# rm -f *.o
# if [ ! -e libroot ]; then
#     ./build-libs.sh;
# fi
# make -j8
# cd ..

#
# Build the iOS source tree.
#
echo "Building iOS source tree."
cd engine-ios
./make-src.sh
cd ..

#
# Build the Android source tree.
#
echo "Building Android source tree."
cd engine-android
./make-src.sh
cd ..

#
# Make an installer for Windows.
#
echo "Creating an installer for Windows."

# /
cp -v pro-windows-x86/suika-pro.exe installer-windows/suika-pro.exe
cp -v ../doc/readme-jp.html installer-windows/readme-jp.html
cp -v ../doc/readme-en.html installer-windows/readme-en.html

# /games
rm -rf installer-windows/games
find ../games -name '.*' | xargs rm
cp -Rv ../games installer-windows/games

# /tools
rm -rf installer-windows/tools
mkdir -p installer-windows/tools
cp -v engine-windows-x86/suika.exe suika2/tools/
cp -v engine-macos/mac.dmg suika2/tools/
cp -Rv engine-android/android-src installer-windows/tools/android-src
cp -Rv engine-ios/ios-src suika2/tools/ios-src
mkdir -p installer-windows/tools/web
cp -v engine-wasm/index.html installer-windows/tools/web/index.html
cp -v engine-wasm/index.js installer-windows/tools/web/index.js
cp -v engine-wasm/index.wasm installer-windows/tools/web/index.wasm
cp -Rv ../tools/installer suika2/tools/installer
cp -v ../tools/snippets/jp-normal/plaintext.code-snippets installer-windows/plaintext.code-snippets.jp
cp -v ../tools/snippets/en-normal/plaintext.code-snippets installer-windows/plaintext.code-snippets.en

# Make an installer
cd installer-windows
make
sign.sh suika2-x.x.exe
mv suika2-x.x.exe "suika2-win-$VERSION.exe"

#
# Make an installer for macOS.
#
echo "Creating an installer for macOS."

find ../games -name '.*' | xargs rm

cd pro-macos
make
mv suika.pkg "suika2-mac-$VERSION.pkg"
cd ..

#
# Upload.
#
echo "Uploading files."

curl -T "installer-windows/suika2-win-$VERSION.zip" -u "$FTP_USER:$FTP_PASSWORD" "$FTP_URL/suika2-win-$VERSION.exe"
wait 5
curl -T "pro-macos/suika2-mac-$VERSION.pkg" -u "$FTP_USER:$FTP_PASSWORD" "$FTP_URL/suika2-mac-$VERSION.pkg"
echo "Upload completed."

#
# Update the Web site.
#
# echo ""
# echo "Updating the Web site:"
# SAVE_DIR=`pwd`
# cd ../doc/web && ./update-templates.sh && ./update-version.sh && ./upload.sh
# cd "$SAVE_DIR"
