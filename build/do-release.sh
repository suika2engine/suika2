#!/bin/sh

set -eu

#
# Settings
#

# Path to cmd.exe (we use an absolute path for a bad PATH variable)
CMDEXE='/mnt/c/Windows/system32/cmd.exe'

# Path to signtool.exe (we will find a suitable version)
SIGNTOOL=`find '/mnt/c/Program Files (x86)/Windows Kits/10/bin' -name 'signtool.exe' | grep 'x86/signtool.exe' | head -n1 | tr -d '\r\n'`

# Signature for code signing (you can modify here)
SIGNATURE="Open Source Developer, Keiichi Tabata"

#
# Input a version number.
#
echo "Enter version e.g. 12.38"
read str
VERSION=$str
if [ ! -n "$VERSION" ]; then
    echo "Please enter a version string.";
    exit 1;
fi

#
# Confirmation.
#
echo "Are you sure you want to release $VERSION? (press return)"
read str

#
# Load credentials from .env file.
#
echo "Checking for .env credentials."
MACOS_HOST=""
MACOS_USER=""
MACOS_PASSWORD=""
FTP_LOCAL=""
FTP_USER=""
FTP_PASSWORD=""
FTP_URL=""
if [ -e .env ]; then
	eval `cat .env`;
fi
if [ -z "`uname | grep Darwin`" ]; then
    if [ -z "$MACOS_HOST" ]; then
	echo "Error: Please specify MACOS_HOST in build/.env";
	echo "       This information is utilized to build macOS apps by ssh.";
	exit 1;
    fi;
    MACOS_HOST_IP=`getent ahosts "$MACOS_HOST" | grep "$MACOS_HOST" | awk '{ print $1 }'`
    if [ -z "$MACOS_HOST_IP" ]; then
	echo "Error: Could not resolve host name $MACOS_HOST. It may be a problem of mDNS. Try again.";
	exit 1;
    fi
    if [ -z "$MACOS_USER" ]; then
	echo "Error: Please specify MACOS_USER in build/.env";
	echo "       This information is utilized to determine your home directory.";
	exit 1;
    fi
    if [ -z "$MACOS_PASSWORD" ]; then
	echo "Error: Please specify MACOS_PASSWORD in build/.env";
	echo "       This information is utilized to unlock your keychain in ssh sessions.";
	exit 1;
    fi
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
RELEASETMP=""
DO_SIGN=0;
if [ ! -z "`uname | grep Linux`" ]; then
    if [ ! -z "`grep -i WSL2 /proc/version`" ]; then
	echo "Creating a temporary folder on Windows.";
	RELEASETMP=/mnt/c/Users/`powershell.exe '$env:UserName' | tr -d '\r\n'`/suika2-release-tmp;
	DO_SIGN=1;
	rm -rf $RELEASETMP && mkdir $RELEASETMP
    fi
fi
if [ -z "$RELEASETMP" ]; then
    echo "Creating a temporary directory release-tmp.";
    RELEASETMP=`pwd`/release-tmp;
    rm -rf $RELEASETMP && mkdir $RELEASETMP
fi	
echo "$RELEASETMP created."

#
# Build suika.exe
#
echo "Building suika.exe"
cd mingw
make erase
make libroot
make -j24
cp suika.exe $RELEASETMP/
cd ../

#
# Build suika-pro.exe
#
echo "Building suika-pro.exe"
cd mingw-pro
make erase
cp -Rav ../mingw/libroot .
make -j24
cp suika-pro.exe $RELEASETMP/
cd ../

#
# Build suika-capture.exe
#
echo "Building suika-capture.exe"
cd mingw-capture
make clean
cp -Rav ../mingw/libroot .
make -j24
cp suika-capture.exe $RELEASETMP/
cd ../

#
# Build suika-replay.exe
#
echo "Building suika-replay.exe"
cd mingw-replay
make clean
cp -Rav ../mingw/libroot .
make -j24
cp suika-replay.exe $RELEASETMP/
cd ../

#
# Build suika-64.exe
#
echo "Building suika-64.exe"
cd mingw-64
make erase
make libroot
make -j24
cp suika-64.exe $RELEASETMP/
cd ../

#
# Build suika-arm64.exe
#
echo "Building suika-arm64.exe"
cd mingw-arm64
make erase
make libroot
make -j24
cp suika-arm64.exe $RELEASETMP/
cd ../

#
# Build suika-linux
#
if [ ! -z "`uname | grep Linux`" ]; then
	echo "Building suika-linux";
	cd linux-x86_64;
	make erase;
	make libroot;
	make -j24;
	cp suika $RELEASETMP/suika-linux;
	cd ../;
else
	touch $RELEASETMP/suika-linux
fi

#
# Build Web files
#
echo "Building Emscripten files."
cd emscripten
make clean
make
cp html/index.html html/index.js html/index.wasm $RELEASETMP/
cd ../

#
# Sign main exe files.
#
if [ "$DO_SIGN" -eq "1" ]; then
    echo "Signing the Windows apps on Windows.";
    SAVE_WD=`pwd`;
    cd "$RELEASETMP";
    $CMDEXE /C start "" "`wslpath -w "$SIGNTOOL"`" sign /n "$SIGNATURE" /td sha256 /fd sha256 /tr http://time.certum.pl/ /v suika.exe suika-pro.exe suika-capture.exe suika-replay.exe suika-64.exe suika-arm64.exe;
    cd "$SAVE_WD";
else
    echo "Skipping code signing for Windows binaries because we are not running on Windows."
fi

#
# Build macOS apps.
#
echo "Building macOS apps."

if [ -z "`uname | grep Darwin`" ]; then
    echo "Building on a remote host...";
    until \
		ssh "$MACOS_HOST_IP" "cd /Users/$MACOS_USER/src/suika2 && git pull github master && make all-macos";
	do \
		echo "Retrying due to a codesign issue...";
	done;
    scp "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/mac.dmg" "$RELEASETMP/";
    scp "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/mac-pro.dmg" "$RELEASETMP/";
    scp "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/mac.zip" "$RELEASETMP/";
    scp "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/pack.mac" "$RELEASETMP/";
    #scp "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/mac-capture.dmg" "$RELEASETMP/";
    #scp "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/mac-replay.dmg" "$RELEASETMP/";
else
    echo "Building on localhost..."
    cd ../
    make all-macos;
    cp mac.dmg "$RELEASETMP/";
    cp mac-pro.dmg "$RELEASETMP/";
    cp mac.zip "$RELEASETMP/";
    cp pack.mac "$RELEASETMP/";
    #cp mac-capture.dmg "$RELEASETMP/";
    #cp mac-replay.dmg "$RELEASETMP/";
    cd build
fi

#
# Create a helper zip for Kirara.
#  - This file is used by 'tools/kirara/apps/update-suika2.sh'
#
echo "Creating a helper zip for Kirara."

SAVE_WD=`pwd`
cd "$RELEASETMP"
zip kirara-helper-current.zip suika.exe suika-pro.exe mac.dmg mac.zip pack.mac index.html index.js index.wasm
cd "$SAVE_WD"

#
# Build Kirara (Windows App)
#
echo "Building a Kirara Windows app."
cd ../tools/kirara
cp "$RELEASETMP/suika.exe" apps/
cp "$RELEASETMP/suika-pro.exe" apps/
cp "$RELEASETMP/mac.dmg" apps/
cp "$RELEASETMP/mac.zip" apps/
cp "$RELEASETMP/pack.mac" apps/
cp "$RELEASETMP/index.html" apps/
cp "$RELEASETMP/index.js" apps/
cp "$RELEASETMP/index.wasm" apps/
make win
cp "dist/Kirara Setup 1.0.0.exe" "$RELEASETMP/kirara-win.exe"
cd ../../build
if [ "$DO_SIGN" -eq "1" ]; then
    echo "Signing the Kirara Windows app on Windows.";
    SAVE_WD=`pwd`;
    cd "$RELEASETMP";
    $CMDEXE /C start "" "`wslpath -w "$SIGNTOOL"`" sign /n "$SIGNATURE" /td sha256 /fd sha256 /tr http://time.certum.pl/ /v kirara-win.exe;
    cd "$SAVE_WD";
    mv "$RELEASETMP/kirara-win.exe" "$RELEASETMP/kirara-win-$VERSION.exe";
else
    echo "Skipping code signing for Kirara Windows app."
    mv "$RELEASETMP/kirara-win.exe" "$RELEASETMP/kirara-win-$VERSION.exe";
fi

#
# Build Kirara (macOS App)
#
echo "Building a Kirara macOS app."
if [ -z "`uname | grep Darwin`" ]; then
    echo "Building on a remote host...";
    scp "$RELEASETMP/suika.exe" "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/tools/kirara/apps/";
    scp "$RELEASETMP/suika-pro.exe" "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/tools/kirara/apps/";
    scp "$RELEASETMP/mac.dmg" "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/tools/kirara/apps/";
    scp "$RELEASETMP/mac.zip" "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/tools/kirara/apps/";
    scp "$RELEASETMP/pack.mac" "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/tools/kirara/apps/";
    scp "$RELEASETMP/index.html" "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/tools/kirara/apps/";
    scp "$RELEASETMP/index.js" "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/tools/kirara/apps/";
    scp "$RELEASETMP/index.wasm" "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/tools/kirara/apps/";
    until \
		ssh "$MACOS_HOST_IP" "cd /Users/$MACOS_USER/src/suika2/tools/kirara && make mac";
	do \
		echo "Retrying due to a codesign issue...";
	done;
    scp "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/tools/kirara/dist/Kirara-1.0.0.dmg" "$RELEASETMP/kirara-mac-$VERSION.dmg";
else
    echo "Building on localhost...";
    cd ../tools/kirara;
    make mac;
    cp dist/Kirara-1.0.0.dmg "$RELEASETMP/kirara-mac-$VERSION.dmg";
    cd ../../build;
fi

#
# Make main release files.
#
echo "Creating main release files."

# Japanese ZIP
rm -rf suika2
mkdir suika2
mkdir suika2/anime && cp -Rv ../game-jp/anime/* suika2/anime/
mkdir suika2/bg && cp -Rv ../game-jp/bg/* suika2/bg/
mkdir suika2/bgm && cp -Rv ../game-jp/bgm/* suika2/bgm/
mkdir suika2/cg && cp -Rv ../game-jp/cg/* suika2/cg/
mkdir suika2/ch && cp -Rv ../game-jp/ch/* suika2/ch/
mkdir suika2/conf && cp -Rv ../game-jp/conf/*.txt suika2/conf/
mkdir suika2/cv && cp -Rv ../game-jp/cv/* suika2/cv/
mkdir suika2/font && cp -Rv ../game-jp/font/* suika2/font/
mkdir suika2/gui && cp -Rv ../game-jp/gui/*.txt suika2/gui/
mkdir suika2/rule && cp -Rv ../game-jp/rule/* suika2/rule/
mkdir suika2/mov
mkdir suika2/se && cp -Rv ../game-jp/se/* suika2/se/
mkdir suika2/txt && cp -Rv ../game-jp/txt/*.txt suika2/txt/
mkdir suika2/wms && cp -Rv ../game-jp/wms/*.txt suika2/wms/
cp -v ../doc/COPYING suika2/
cp -v ../doc/readme-jp.html suika2/README.html
mkdir suika2/.vscode && cp -v ../tools/snippets/jp-normal/plaintext.code-snippets suika2/.vscode/
cp -v "$RELEASETMP/suika.exe" suika2/
cp -v "$RELEASETMP/suika-pro.exe" suika2/
cp -v "$RELEASETMP/mac.dmg" suika2/
cp -v "$RELEASETMP/mac-pro.dmg" suika2/
mkdir suika2/tools
cp -v ../doc/readme-tools-jp.txt suika2/tools/README.TXT
cp -v "$RELEASETMP/suika-capture.exe" suika2/tools/
cp -v "$RELEASETMP/suika-replay.exe" suika2/tools/
cp -v "$RELEASETMP/suika-64.exe" suika2/tools/
cp -v "$RELEASETMP/suika-arm64.exe" suika2/tools/
#cp -v "$RELEASETMP/mac-capture.dmg" suika2/tools/
#cp -v "$RELEASETMP/mac-replay.dmg" suika2/tools/
cp -v "$RELEASETMP/suika-linux" suika2/tools/
mkdir suika2/tools/web
cp -v "$RELEASETMP/index.html" suika2/tools/web/
cp -v "$RELEASETMP/index.js" suika2/tools/web/
cp -v "$RELEASETMP/index.wasm" suika2/tools/web/
cp -v emscripten/about-jp.txt suika2/tools/web/about.txt
zip -r "$RELEASETMP/suika2-$VERSION-jp.zip" suika2
rm -rf suika2

# English ZIP
rm -rf suika2
mkdir suika2
mkdir suika2/anime && cp -Rv ../game-en/anime/* suika2/anime/
mkdir suika2/bg && cp -Rv ../game-en/bg/* suika2/bg/
mkdir suika2/bgm && cp -Rv ../game-en/bgm/* suika2/bgm/
mkdir suika2/cg && cp -Rv ../game-en/cg/* suika2/cg/
mkdir suika2/ch && cp -Rv ../game-en/ch/* suika2/ch/
mkdir suika2/conf && cp -Rv ../game-en/conf/*.txt suika2/conf/
mkdir suika2/cv && cp -Rv ../game-en/cv/* suika2/cv/
mkdir suika2/font && cp -Rv ../game-en/font/* suika2/font/
mkdir suika2/gui && cp -Rv ../game-en/gui/*.txt suika2/gui/
mkdir suika2/rule && cp -Rv ../game-en/rule/* suika2/rule/
mkdir suika2/mov
mkdir suika2/se && cp -Rv ../game-en/se/* suika2/se/
mkdir suika2/txt && cp -Rv ../game-en/txt/*.txt suika2/txt/
mkdir suika2/wms && cp -Rv ../game-en/wms/*.txt suika2/wms/
cp -v ../doc/COPYING suika2/
cp -v ../doc/readme-en.html suika2/README.html
mkdir suika2/.vscode && cp -v ../tools/snippets/en-normal/plaintext.code-snippets suika2/.vscode/
cp -v "$RELEASETMP/suika.exe" suika2/
cp -v "$RELEASETMP/suika-pro.exe" suika2/
cp -v "$RELEASETMP/mac.dmg" suika2/
cp -v "$RELEASETMP/mac-pro.dmg" suika2/
mkdir suika2/tools
cp -v ../doc/readme-tools-en.txt suika2/tools/README.TXT
cp -v "$RELEASETMP/suika-capture.exe" suika2/tools/
cp -v "$RELEASETMP/suika-replay.exe" suika2/tools/
cp -v "$RELEASETMP/suika-64.exe" suika2/tools/
cp -v "$RELEASETMP/suika-arm64.exe" suika2/tools/
#cp -v "$RELEASETMP/mac-capture.dmg" suika2/tools/
#cp -v "$RELEASETMP/mac-replay.dmg" suika2/tools/
cp -v "$RELEASETMP/suika-linux" suika2/tools/
mkdir suika2/tools/web
cp -v "$RELEASETMP/index.html" suika2/tools/web/
cp -v "$RELEASETMP/index.js" suika2/tools/web/
cp -v "$RELEASETMP/index.wasm" suika2/tools/web/
cp -v emscripten/about-en.txt suika2/tools/web/about.txt
zip -r "$RELEASETMP/suika2-$VERSION-en.zip" suika2
rm -rf suika2

#
# Upload.
#
echo "Uploading files."

# Copy release files to FTPLOCAL directory.
cp "$RELEASETMP/suika2-$VERSION-jp.zip" $FTP_LOCAL/
cp "$RELEASETMP/suika2-$VERSION-en.zip" $FTP_LOCAL/
cp "$RELEASETMP/kirara-win-$VERSION.exe" $FTP_LOCAL/
cp "$RELEASETMP/kirara-mac-$VERSION.dmg" $FTP_LOCAL/
cp "$RELEASETMP/kirara-helper-current.zip" $FTP_LOCAL/

# Upload.
if [ "$DO_UPLOAD" -eq "1" ]; then
    curl -T "$RELEASETMP/suika2-$VERSION-jp.zip" -u "$FTP_USER:$FTP_PASSWORD" "$FTP_URL/suika2-$VERSION-jp.zip" && sleep 5;
    curl -T "$RELEASETMP/suika2-$VERSION-en.zip" -u "$FTP_USER:$FTP_PASSWORD" "$FTP_URL/suika2-$VERSION-en.zip" && sleep 5;
    curl -T "$RELEASETMP/kirara-win-$VERSION.exe" -u "$FTP_USER:$FTP_PASSWORD" "$FTP_URL/kirara-win-$VERSION.exe" && sleep 5;
    curl -T "$RELEASETMP/kirara-mac-$VERSION.dmg" -u "$FTP_USER:$FTP_PASSWORD" "$FTP_URL/kirara-mac-$VERSION.dmg" && sleep 5;
    curl -T "$RELEASETMP/kirara-helper-current.zip" -u "$FTP_USER:$FTP_PASSWORD" "$FTP_URL/kirara-helper-current.zip";
else
    echo "Skipped upload.";
fi

#
# Complete!
#
echo ""
echo "Release completed."
if [ "$DO_SIGN" -eq "0" ]; then
    echo "Note: We have not signed to the Windows binaries.";
fi
if [ "$DO_UPLOAD" -eq "0" ]; then
    echo "Note: We have not uploaded the release files.";
fi
echo ""
