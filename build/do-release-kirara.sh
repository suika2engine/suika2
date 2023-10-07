#!/bin/sh

set -eu

# Signature for code signing (you can modify here)
SIGNATURE="Open Source Developer, Keiichi Tabata"

#
# Welcome.
#
echo ""
echo "Hello, this is the Kirara Release Manager."

#
# Guess the version number.
#
VERSION=`grep -a1 '<!-- BEGIN-LATEST -->' ../doc/readme-jp.html | tail -n1`
VERSION=`echo $VERSION | cut -d '>' -f 2 | cut -d ' ' -f 1`
VERSION=`echo $VERSION | cut -d '/' -f 2`

#
# Confirmation.
#
echo "Are you sure you want to release Kirara $VERSION?"
echo ""
echo "Press enter to proceed:"
read str

#
# Search signtool.exe.
#
echo "Searching Windows Kits..."

# Path to signtool.exe (we will find a suitable version)
SIGNTOOL=`find '/mnt/c/Program Files (x86)/Windows Kits/10/bin' -name 'signtool.exe' | grep 'x86/signtool.exe' | tail -n1 | tr -d '\r\n'`
if [ -z "$SIGNTOOL" ]; then
    echo "signtool.exe not found.";
else
    echo "signtool.exe found.";
fi

# Set the path to cmd.exe (we use an absolute path for a bad PATH variable)
CMDEXE='/mnt/c/Windows/system32/cmd.exe'

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
    MACOS_HOST_IP=`getent ahosts "$MACOS_HOST" | grep "$MACOS_HOST" | awk '{ print $1 }'`;
    if [ -z "$MACOS_USER" ]; then
	echo "Error: Cannot resolve the IP address for the macOS host $MACOS_HOST.";
	echo "       Try again.";
	exit 1;
    fi;
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
	RELEASETMP=/mnt/c/Users/`powershell.exe '$env:UserName' | tr -d '\r\n'`/suika2-kirara-release-tmp;
	DO_SIGN=1;
	rm -rf $RELEASETMP && mkdir $RELEASETMP
    fi
fi
if [ -z "$RELEASETMP" ]; then
    echo "Creating a temporary directory release-tmp.";
    RELEASETMP=`pwd`/release-kirara-tmp;
    rm -rf $RELEASETMP && mkdir $RELEASETMP
fi	
echo "$RELEASETMP created."

#
# Build Kirara (Windows App)
#
echo "Building a Kirara Windows app."
cd ../tools/kirara
make update
make win
cp "dist/Kirara Setup 1.0.0.exe" "$RELEASETMP/kirara-win.exe"
cd ../../build
if [ "$DO_SIGN" -eq "1" ]; then
    echo "Signing the Kirara Windows app on Windows.";
    SAVE_WD=`pwd`;
    cd "$RELEASETMP";
    $CMDEXE /C start "" "`wslpath -w "$SIGNTOOL"`" sign /n "$SIGNATURE" /td sha256 /fd sha256 /tr http://time.certum.pl/ /v kirara-win.exe | tee;
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
    ssh "$MACOS_HOST_IP" "cd /Users/$MACOS_USER/src/suika2/tools/kirara && make update";
    until \
	ssh "$MACOS_HOST_IP" "cd /Users/$MACOS_USER/src/suika2/tools/kirara && make mac";
    do \
	echo "Retrying due to a codesign issue...";
    done;
    scp "$MACOS_HOST_IP:/Users/$MACOS_USER/src/suika2/tools/kirara/dist/Kirara-1.0.0.dmg" "$RELEASETMP/kirara-mac-$VERSION.dmg";
else
    echo "Building on localhost...";
    cd ../tools/kirara;
    make update;
    make mac;
    cp dist/Kirara-1.0.0.dmg "$RELEASETMP/kirara-mac-$VERSION.dmg";
    cd ../../build;
fi

#
# Upload.
#
echo "Uploading files."

# Copy release files to FTPLOCAL directory.
cp "$RELEASETMP/kirara-win-$VERSION.exe" $FTP_LOCAL/
cp "$RELEASETMP/kirara-mac-$VERSION.dmg" $FTP_LOCAL/

# Upload.
if [ "$DO_UPLOAD" -eq "1" ]; then
    curl -T "$RELEASETMP/kirara-win-$VERSION.exe" -u "$FTP_USER:$FTP_PASSWORD" "$FTP_URL/kirara-win-$VERSION.exe" && sleep 5;
    curl -T "$RELEASETMP/kirara-mac-$VERSION.dmg" -u "$FTP_USER:$FTP_PASSWORD" "$FTP_URL/kirara-mac-$VERSION.dmg" && sleep 5;
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
