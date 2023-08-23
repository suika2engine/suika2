#!/bin/sh

set -eu

# Get the target name.
APP=$1

# Build an archive.
xcodebuild \
    -scheme $APP \
    -project suika.xcodeproj \
    -configuration Release \
    -archivePath $PWD/build/Release/$APP.xcarchive \
    build
