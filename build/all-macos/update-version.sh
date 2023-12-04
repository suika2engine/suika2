#!/bin/bash

SED='sed'
if [ ! -z "`which gsed`" ]; then
    SED='gsed';
fi

VERSION=$1
if [ ! -n "$VERSION" ]; then
    echo "Please specify a version string.";
    exit 1;
fi

$SED -i "s/MARKETING_VERSION = .*;/MARKETING_VERSION = $VERSION;/g" suika.xcodeproj/project.pbxproj
