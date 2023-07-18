#!/bin/bash

set -eu

echo "Enter version e.g. 2.9.0"
read str
VERSION=$str
[ -n "$VERSION" ]

sed -i "s/MARKETING_VERSION = .*;/MARKETING_VERSION = $VERSION;/g" suika.xcodeproj/project.pbxproj
