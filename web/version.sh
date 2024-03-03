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
HEADER=`grep -a1 '<!-- BEGIN-LATEST-JP -->' ../ChangeLog | tail -n1`
VERSION=`echo $HEADER | cut -d '>' -f 2 | cut -d ' ' -f 1`
VERSION_DIGIT=`echo $VERSION | cut -d '/' -f 2`
DATE=`echo $HEADER | cut -d ' ' -f 2`

# Get the release notes for Japanese and English.
NOTE_JP=`cat ../ChangeLog | awk '/BEGIN-LATEST-JP/,/END-LATEST-JP/' | tail -n +2 | $HEAD -n -1`
NOTE_EN=`cat ../ChangeLog | awk '/BEGIN-LATEST-EN/,/END-LATEST-EN/' | tail -n +2 | $HEAD -n -1`

# Update /index.html
$SED -e "s|.*LATEST-RELEASE.*|<!--LATEST-RELEASE--><li><a href=\"/dl/\">$DATE $VERSION リリース</a></li>|" -i index.html

# Update /dl/index.html
cat dl/index.html | \
$SED -e "s|.*LATEST-VERSION-AND-DATE.*|<!-- LATEST-VERSION-AND-DATE -->最新版 $VERSION \($DATE\):<br>|" \
     -e "s|.*LATEST-EXE.*|<!-- LATEST-EXE --><a href=\"/dl/suika2-$VERSION_DIGIT.exe\">Windows版 Suika2 Pro Desktop インストーラ</a><br>|" \
     -e "s|.*LATEST-DMG.*|<!-- LATEST-DMG --><a href=\"/dl/suika2-$VERSION_DIGIT.dmg\">macOS版 Suika2 Pro Desktop インストーラ</a><br>|" \
     -e "s|.*LATEST-ZIP.*|<!-- LATEST-ZIP --><a href=\"/dl/suika2-$VERSION_DIGIT.7z\">7zip配布のSuika2 Pro Desktop (Windows版)</a><br>|" \
 > tmp
cat tmp | awk '/.*DOCTYPE html.*/,/.*LATEST-RELEASE-NOTE.*/' > before
cat tmp | awk '/.*LATEST-RELEASE-NOTE.*/,/\<\/html\>/' | tail -n +2 > after
cat before > new
echo $NOTE_JP >> new
cat after >> new
cp new dl/index.html
rm -f before after new tmp

# Update /en/index.html
$SED -e "s|.*LATEST-RELEASE.*|<!--LATEST-RELEASE--><li><a href=\"/en/dl/\">$DATE $VERSION Released!!</a></li>|" -i en/index.html

# Update /en/dl/index.html
cat en/dl/index.html | \
$SED -e "s|.*LATEST-VERSION-AND-DATE.*|<!-- LATEST-VERSION-AND-DATE -->Latest $VERSION \($DATE\):<br>|" \
     -e "s|.*LATEST-EXE.*|<!-- LATEST-EXE --><a href=\"/dl/suika2-$VERSION_DIGIT.exe\">Windows version of Suika2 Pro Desktop Installer</a><br>|" \
     -e "s|.*LATEST-DMG.*|<!-- LATEST-DMG --><a href=\"/dl/suika2-$VERSION_DIGIT.dmg\">macOS version of Suika2 Pro Desktop Installer</a><br>|" \
  > tmp
cat tmp | awk '/.*DOCTYPE html.*/,/.*LATEST-RELEASE-NOTE.*/' > before
cat tmp | awk '/.*LATEST-RELEASE-NOTE.*/,/\<\/html\>/' | tail -n +2 > after
cat before > new
echo $NOTE_EN >> new
cat after >> new
cp new en/dl/index.html
rm -f before after new tmp
