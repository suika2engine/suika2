#!/bin/bash

SED='sed'
if [ ! -z "`which gsed`" ]; then
	SED='gsed';
fi

function upload()
{
    ftp-upload.sh $1;

#   $SED -e 's|.*INSERT-GTAG-HERE.*|\<script async src="https://www.googletagmanager.com/gtag/js\?id=G-PLYR5Y3JSJ"\>\</script\>\<script\>window.dataLayer = window.dataLayer \|\| \[\];function gtag\(\){dataLayer.push\(arguments\);}gtag\("js", new Date\(\)\);gtag\("config", "G-PLYR5Y3JSJ"\);\</script\>|' < $1 > tmp;
#   ftp-upload.sh tmp $1;
#   rm tmp;
}

upload index.html
upload dl/index.html
upload dl/material/index.html
upload doc/index.html
upload doc/font.html
upload doc/scriptgen.html
upload works/index.html
upload wiki/skin/pukiwiki.skin.php
upload award2023.html
upload award2024.html

upload en/index.html
upload en/dl/index.html
upload en/doc/index.html
upload en/doc/font.html
upload en/doc/title.html
upload en/doc/wms.html
upload en/doc/scriptgen.html
upload en/doc/faq.html
upload en/works/index.html
upload en/wiki/skin/pukiwiki.skin.php
