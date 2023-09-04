#!/bin/bash

SED='sed'

if [ -z "`sed --version | grep GNU`" ]; then
    if [ ! -z `which gsed` ]; then
	SED='gsed';
    else
	echo "Please install GNU sed by Homebrew.";
	exit 1;
    fi
fi

VERSION=$1
if [ ! -n "$VERSION" ]; then
    echo "Please specify a version string.";
    exit 1;
fi

sed -i "s/MARKETING_VERSION = .*;/MARKETING_VERSION = $VERSION;/g" suika.xcodeproj/project.pbxproj
