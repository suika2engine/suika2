#!/bin/bash

set -eu

VERSION="$1"

SED='sed'
if [ ! -z "`which gsed`" ]; then
	SED='gsed';
fi

HEAD='head'
if [ ! -z "`which ghead`" ]; then
	HEAD='ghead';
fi

# Update /dl/index.html
$SED -e "s|.*STABLE-VERSION-AND-DATE.*|<!-- STABLE-VERSION-AND-DATE -->最新版 Suika2/$VERSION:<br>|" \
     -e "s|.*STABLE-EXE.*|<!-- STABLE-EXE --><a href=\"/dl/suika2-$VERSION.exe\">Suika2 (Windows)</a><br>|" \
     -e "s|.*STABLE-DMG.*|<!-- STABLE-DMG --><a href=\"/dl/suika2-$VERSION.dmg\">Suika2 (macOS)</a><br>|" \
	 -i dl/index.html

# Update /en/dl/index.html
$SED -e "s|.*STABLE-VERSION-AND-DATE.*|<!-- STABLE-VERSION-AND-DATE -->Stable Suika2/$VERSION:<br>|" \
     -e "s|.*STABLE-EXE.*|<!-- STABLE-EXE --><a href=\"/dl/suika2-$VERSION.exe\">Suika2 (Windows)</a><br>|" \
     -e "s|.*STABLE-DMG.*|<!-- STABLE-DMG --><a href=\"/dl/suika2-$VERSION.dmg\">Suika2 (macOS)</a><br>|" \
	 -i en/dl/index.html
