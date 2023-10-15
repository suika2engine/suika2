#!/bin/bash

function upload()
{
    if [ ! -z "$GTAG" ]; then
		sed -e 's|.*INSERT-GTAG-HERE.*|\<script async src="https://www.googletagmanager.com/gtag/js\?id=G-PLYR5Y3JSJ"\>\</script\>\<script\>window.dataLayer = window.dataLayer \|\| \[\];function gtag\(\){dataLayer.push\(arguments\);}gtag\("js", new Date\(\)\);gtag\("config", "$GTAG"\);\</script\>|' < $1 > tmp
        ftp-upload.sh tmp $1;
		rm tmp
    else
        ftp-upload.sh $1 $1;
    fi
}

upload index.html
upload dl/index.html
upload doc/index.html
upload doc/tutorial.html
upload doc/reference.html
upload doc/font.html
upload doc/title.html
upload doc/wms.html
upload doc/scriptgen.html
upload doc/faq.html
upload works/index.html
upload wiki/skin/pukiwiki.skin.php

upload en/index.html
upload en/dl/index.html
upload en/doc/index.html
upload en/doc/tutorial.html
upload en/doc/reference.html
upload en/doc/font.html
upload en/doc/title.html
upload en/doc/wms.html
upload en/doc/scriptgen.html
upload en/doc/faq.html
upload en/works/index.html
upload en/wiki/skin/pukiwiki.skin.php
