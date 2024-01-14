#!/bin/bash

git add index.html
git add dl/index.html
git add doc/index.html
git add doc/font.html
git add doc/scriptgen.html
git add works/index.html
git add wiki/skin/pukiwiki.skin.php
git add award2023.html

git add en/index.html
git add en/dl/index.html
git add en/doc/index.html
git add en/doc/font.html
git add en/doc/title.html
git add en/doc/wms.html
git add en/doc/scriptgen.html
git add en/doc/faq.html
git add en/works/index.html
git add en/wiki/skin/pukiwiki.skin.php

git commit -m "web: update"
git push github master
