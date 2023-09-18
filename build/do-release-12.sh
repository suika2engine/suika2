#!/bin/sh

set -eu

#
# Settings
#

# Path to cmd.exe (we use an absolute path for a bad PATH variable)
CMDEXE='/mnt/c/Windows/system32/cmd.exe'

# Path to signtool.exe (we will find a suitable version)
SIGNTOOL=`find '/mnt/c/Program Files (x86)/Windows Kits/10/bin' -name 'signtool.exe' | grep 'x86/signtool.exe' | tail -n1 | tr -d '\r\n'`

# Signature for code signing (you can modify here)
SIGNATURE="Open Source Developer, Keiichi Tabata"

#
# Input a version number.
#
echo "Enter version e.g. 12.0"
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
RELEASETMP=""
DO_SIGN=0;
if [ ! -z "`uname | grep Linux`" ]; then
    if [ ! -z "`grep -i WSL2 /proc/version`" ]; then
	echo "Creating a temporary folder on Windows.";
	RELEASETMP=/mnt/c/Users/`powershell.exe '$env:UserName' | tr -d '\r\n'`/suika2-12-release-tmp;
	DO_SIGN=1;
	rm -rf $RELEASETMP && mkdir $RELEASETMP
    fi
fi
if [ -z "$RELEASETMP" ]; then
    echo "Creating a temporary directory release-tmp.";
    RELEASETMP=`pwd`/release-12-tmp;
    rm -rf $RELEASETMP && mkdir $RELEASETMP
fi	
echo "$RELEASETMP created."

#
# Build suika.exe
#
echo "Building suika.exe"
cd mingw
rm -f *.o
if [ ! -e libroot ]; then ./build-libs.sh; fi
make -j24
cp suika.exe $RELEASETMP/
cd ../

#
# Build suika-pro.exe
#
echo "Building suika-pro.exe"
cd mingw-pro
rm -f *.o
if [ ! -e libroot ]; then cp -Ra ../mingw/libroot .; fi
make -j24
cp suika-pro.exe $RELEASETMP/
cd ../

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
    $CMDEXE /C start "" "`wslpath -w "$SIGNTOOL"`" sign /n "$SIGNATURE" /td sha256 /fd sha256 /tr http://time.certum.pl/ /v suika.exe suika-pro.exe | tee;
    cd "$SAVE_WD";
else
    echo "Skipping code signing for Windows binaries because we are not running on Windows."
fi

#
# Make a main release file.
#
echo "Creating a main release file."

# Main ZIP
rm -rf suika2
mkdir suika2
cp -v ../doc/COPYING suika2/
cp -v ../doc/readme-jp.html suika2/README.html
cp -v "$RELEASETMP/suika.exe" suika2/
cp -v "$RELEASETMP/suika-pro.exe" suika2/
mkdir suika2/web
cp -v "$RELEASETMP/index.html" suika2/web/
cp -v "$RELEASETMP/index.js" suika2/web/
cp -v "$RELEASETMP/index.wasm" suika2/web/
zip -r "$RELEASETMP/suika2-$VERSION.zip" suika2
rm -rf suika2

#
# Upload.
#
echo "Uploading files."

# Copy release files to FTPLOCAL directory.
cp "$RELEASETMP/suika2-$VERSION.zip" $FTP_LOCAL/

# Upload.
if [ "$DO_UPLOAD" -eq "1" ]; then
    curl -T "$RELEASETMP/suika2-$VERSION.zip" -u "$FTP_USER:$FTP_PASSWORD" "$FTP_URL/suika2-$VERSION.zip" && sleep 5;
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
