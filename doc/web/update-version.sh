#!/bin/bash

SED='sed'
if [ ! -z "`which gsed`" ]; then
	SED='gsed';
fi

HEAD='head'
if [ ! -z "`which ghead`" ]; then
	HEAD='ghead';
fi

# Get the version and the date strings.
HEADER=`grep -a1 '<!-- BEGIN-LATEST -->' ../readme-jp.html | tail -n1`
VERSION=`echo $HEADER | cut -d '>' -f 2 | cut -d ' ' -f 1`
VERSION_DIGIT=`echo $VERSION | cut -d '/' -f 2`
DATE=`echo $HEADER | cut -d ' ' -f 2`

# Get the release notes for Japanese and English.
NOTE_JP=`cat ../readme-jp.html | awk '/BEGIN-LATEST/,/END-LATEST/' | tail -n +2 | $HEAD -n -1`
NOTE_EN=`cat ../readme-en.html | awk '/BEGIN-LATEST/,/END-LATEST/' | tail -n +2 | $HEAD -n -1`

# Update /index.html
$SED -e "s|.*LATEST-RELEASE.*|<!--LATEST-RELEASE--><li><a href=\"/dl/\">$DATE $VERSION リリース</a></li>|" -i index.html

# Update /dl/index.html
cat dl/index.html | \
$SED -e "s|.*LATEST-VERSION-AND-DATE.*|<!-- LATEST-VERSION-AND-DATE -->最新版 $VERSION \($DATE\):<br>|" \
     -e "s|.*LATEST-EXE.*|<!-- LATEST-EXE --><a href=\"/dl/suika2-$VERSION_DIGIT.exe\">Windows exe<br>|" \
     -e "s|.*LATEST-DMG.*|<!-- LATEST-DMG --><a href=\"/dl/suika2-$VERSION_DIGIT.dmg\">macOS dmg<br>|" \
  > tmp
cat tmp | awk '/.*DOCTYPE html.*/,/.*LATEST-RELEASE-NOTE.*/' > before
cat tmp | awk '/.*LATEST-RELEASE-NOTE.*/,/\<\/html\>/' | tail -n +2 > after
cat before > new
echo $NOTE_JP >> new
cat after >> new
cp new dl/index.html
rm before after new

# Update /en/index.html
$SED -e "s|.*LATEST-RELEASE.*|<!--LATEST-RELEASE--><li><a href=\"/en/dl/\">$DATE $VERSION Released!!</a></li>|" -i en/index.html

# Update /en/dl/index.html
cat en/dl/index.html | \
$SED -e "s|.*LATEST-VERSION-AND-DATE.*|<!-- LATEST-VERSION-AND-DATE -->Latest $VERSION \($DATE\):<br>|" \
     -e "s|.*LATEST-EXE.*|<!-- LATEST-EXE --><a href=\"/dl/suika2-$VERSION_DIGIT.exe\">Windows exe</a><br>|" \
     -e "s|.*LATEST-DMG.*|<!-- LATEST-DMG --><a href=\"/dl/suika2-$VERSION_DIGIT.dmg\">macOS exe</a><br>|" \
  > tmp
cat tmp | awk '/.*DOCTYPE html.*/,/.*LATEST-RELEASE-NOTE.*/' > before
cat tmp | awk '/.*LATEST-RELEASE-NOTE.*/,/\<\/html\>/' | tail -n +2 > after
cat before > new
echo $NOTE_EN >> new
cat after >> new
cp new en/dl/index.html
rm before after new
