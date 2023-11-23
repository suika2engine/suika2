#!/bin/sh

set -eu

HEAD='head'
if [ ! -z "`which ghead`" ]; then
	HEAD='ghead';
fi

# Signature for code signing (you can modify here)
SIGNATURE="Open Source Developer, Keiichi Tabata"

#
# Welcome.
#
echo ""
echo "Hello, this is the Suika2 Release Manager."

#
# Guess the version number.
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
# Confirmation.
#
echo "Are you sure you want to release $VERSION?"
echo ""
echo "[Japanese Note]"
echo "$NOTE_JP"
echo ""
echo "[English Note]"
echo "$NOTE_EN"
echo ""
echo "Press enter to proceed:"
read str

#
# Load credentials from .env file.
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
# Make a temporary directory for release binaries.
#
RELEASETMP=`pwd`/release-tmp
echo "Creating a temporary directory release-tmp."
rm -rf $RELEASETMP && mkdir $RELEASETMP
echo "$RELEASETMP created."

#
# Update the macOS project version
#

# cd all-macos
# ./update-version.sh $VERSION
# git add suika.xcodeproj/project.pbxproj ../../doc/readme-jp.html ../../doc/readme-en.html
# git commit -m "doc: update the version number to $VERSION" || true
# git push github master
# cd ..

#
# Build suika.exe
#
echo "Building suika.exe"
cd engine-windows-x86
rm -f *.o
if [ ! -e libroot ]; then ./build-libs.sh; fi
make -j8
cp suika.exe $RELEASETMP/
cd ../

#
# Build suika-64.exe
#
echo "Building suika-64.exe"
cd engine-windows-x86_64
rm -f *.o
if [ ! -e libroot ]; then ./build-libs.sh; fi
make -j8
cp suika-64.exe $RELEASETMP/
cd ../

#
# Build suika-arm64.exe
#
echo "Building suika-arm64.exe"
cd engine-windows-arm64
rm -f *.o
if [ ! -e libroot ]; then ./build-libs.sh; fi
make -j8
cp suika-arm64.exe $RELEASETMP/
cd ../

#
# Build suika-studio.exe
#
echo "Building suika-studio.exe"
cd studio-windows-x86
rm -f *.o
if [ ! -e libroot ]; then cp -Rav ../engine-windows-x86/libroot .; fi
make -j8
cp suika-studio.exe $RELEASETMP/
cd ../

#
# Build suika-pro.exe
#
echo "Building suika-pro.exe"
cd pro-windows-x86
rm -f *.o
if [ ! -e libroot ]; then cp -Rav ../engine-windows-x86/libroot .; fi
make -j8
cp suika-pro.exe $RELEASETMP/
cd ../

#
# Build suika-capture.exe
#
echo "Building suika-capture.exe"
cd capture-windows-x86
rm -f *.o
if [ ! -e libroot ]; then cp -Rav ../engine-windows-x86/libroot .; fi
make -j8
cp suika-capture.exe $RELEASETMP/
cd ../

#
# Build suika-replay.exe
#
echo "Building suika-replay.exe"
cd replay-windows-x86
rm -f *.o
if [ ! -e libroot ]; then cp -Rav ../engine-windows-x86/libroot .; fi
make -j8
cp suika-replay.exe $RELEASETMP/
cd ../

#
# Build suika-linux
#
# if [ ! -z "`uname | grep Linux`" ]; then
#     echo "Building suika-linux";
#     cd engine-linux-x86_64;
#     rm -f *.o;
#     if [ ! -e libroot ]; then ./build-libs.sh; fi
#     make -j8;
#     cp suika $RELEASETMP/suika-linux;
#     cd ../;
# else
#     touch $RELEASETMP/suika-linux
# fi

#
# Build Wasm files
#
echo "Building Wasm files."
cd engine-wasm
make clean
make
cp html/index.html html/index.js html/index.wasm $RELEASETMP/
cd ../

#
# Build Android source tree
#
echo "Building Android source tree."
cd engine-android
./make-src.sh
cd ..

#
# Build iOS source tree
#
echo "Building iOS source tree."
cd engine-ios
./make-src.sh
cd ..

#
# Sign main exe files.
#
echo "Please sign the Windows apps on Windows. (press enter)"
cp "$RELEASETMP/suika.exe" "$RELEASETMP/suika-64.exe" "$RELEASETMP/suika-arm64.exe" "$RELEASETMP/suika-studio.exe" "$RELEASETMP/suika-pro.exe" "$RELEASETMP/suika-capture.exe" "$RELEASETMP/suika-replay.exe" ~/OneDrive/Sign/
read str
cp ~/OneDrive/Sign/suika.exe ~/OneDrive/Sign/suika-64.exe ~/OneDrive/Sign/suika-arm64.exe ~/OneDrive/Sign/suika-studio.exe ~/OneDrive/Sign/suika-pro.exe ~/OneDrive/Sign/suika-capture.exe ~/OneDrive/Sign/suika-replay.exe "$RELEASETMP/"

#
# Build macOS apps.
#
echo "Building macOS apps."
cd all-macos
make main pro
cp mac.dmg "$RELEASETMP/"
cp mac-pro.dmg "$RELEASETMP/"
cd ..

#
# Make a main release file.
#
echo "Creating a main release file."

# Main ZIP
rm -rf suika2
mkdir suika2
mkdir suika2/anime && cp -Rv ../game/anime/* suika2/anime/
mkdir suika2/bg && cp -Rv ../game/bg/* suika2/bg/
mkdir suika2/bgm && cp -Rv ../game/bgm/* suika2/bgm/
mkdir suika2/cg && cp -Rv ../game/cg/* suika2/cg/
mkdir suika2/ch && cp -Rv ../game/ch/* suika2/ch/
mkdir suika2/conf && cp -Rv ../game/conf/*.txt suika2/conf/
mkdir suika2/cv && cp -Rv ../game/cv/* suika2/cv/
mkdir suika2/font && cp -Rv ../game/font/* suika2/font/
mkdir suika2/gui && cp -Rv ../game/gui/*.txt suika2/gui/
mkdir suika2/rule && cp -Rv ../game/rule/* suika2/rule/
mkdir suika2/mov
mkdir suika2/se && cp -Rv ../game/se/* suika2/se/
mkdir suika2/txt && cp -Rv ../game/txt/*.txt suika2/txt/
mkdir suika2/wms && cp -Rv ../game/wms/*.txt suika2/wms/
cp -v ../doc/readme-jp.html suika2/README.html
cp -v ../doc/readme-en.html suika2/README-English.html
mkdir suika2/.vscode && cp -v ../tools/snippets/jp-normal/plaintext.code-snippets suika2/.vscode/
cp -v "$RELEASETMP/suika.exe" suika2/
cp -v "$RELEASETMP/suika-studio.exe" suika2/
mkdir suika2/tools
cp -v "$RELEASETMP/suika-pro.exe" suika2/tools/
cp -v "$RELEASETMP/mac.dmg" suika2/tools/
cp -v "$RELEASETMP/mac-pro.dmg" suika2/
cp -v ../doc/readme-tools-jp.txt suika2/tools/README.txt
cp -v ../doc/readme-tools-en.txt suika2/tools/README-English.txt
cp -v ../doc/readme-tools-zh.txt suika2/tools/README-Chinese.txt
cp -v "$RELEASETMP/suika-capture.exe" suika2/tools/
cp -v "$RELEASETMP/suika-replay.exe" suika2/tools/
cp -v "$RELEASETMP/suika-64.exe" suika2/tools/
cp -v "$RELEASETMP/suika-arm64.exe" suika2/tools/
#cp -v "$RELEASETMP/mac-capture.dmg" suika2/tools/
#cp -v "$RELEASETMP/mac-replay.dmg" suika2/tools/
#cp -v "$RELEASETMP/suika-linux" suika2/tools/
mkdir suika2/tools/web
cp -v "$RELEASETMP/index.html" suika2/tools/web/
cp -v "$RELEASETMP/index.js" suika2/tools/web/
cp -v "$RELEASETMP/index.wasm" suika2/tools/web/
cp -v engine-wasm/about-jp.txt suika2/tools/web/about.txt
cp -v engine-wasm/about-en.txt suika2/tools/web/about-english.txt
cp -R engine-android/android-src suika2/tools/
cp -R engine-ios/ios-src suika2/tools/
cp -R ../tools/installer suika2/tools/
zip -r "$RELEASETMP/suika2-$VERSION.zip" suika2
rm -rf suika2

#
# Upload.
#
echo "Uploading files."

# Copy the release files to FTPLOCAL directory.
cp "$RELEASETMP/suika2-$VERSION.zip" $FTP_LOCAL/

# Upload.
if [ "$DO_UPLOAD" -eq "1" ]; then
    curl -T "$RELEASETMP/suika2-$VERSION.zip" -u "$FTP_USER:$FTP_PASSWORD" "$FTP_URL/suika2-$VERSION.zip" && sleep 5;
else
    echo "Skipped upload.";
    exit 0;
fi
echo "Upload completed."

# Update the Web site.
echo ""
echo "Updating the Web site:"
SAVE_DIR=`pwd`
cd ../doc/web && ./update-templates.sh && ./update-version.sh && ./upload.sh
cd "$SAVE_DIR"
