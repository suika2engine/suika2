#!/bin/bash

function apply_template_japanese ()
{
    cat $1 | awk '/.*DOCTYPE html.*/,/.*BEGIN-HEADER.*/' > tmp.prologue
    cat $1 | awk '/.*END-HEADER.*/,/.*BEGIN-FOOTER.*/' > tmp.main
    cat $1 | awk '/.*END-FOOTER.*/,/\<\/html\>/' > tmp.epilogue
    cat tmp.prologue inc/header.html tmp.main inc/footer.html tmp.epilogue > $1
}

function apply_template_english ()
{
    cat $1 | awk '/.*DOCTYPE html.*/,/.*BEGIN-EN-HEADER.*/' > tmp.prologue
    cat $1 | awk '/.*END-EN-HEADER.*/,/.*BEGIN-EN-FOOTER.*/' > tmp.main
    cat $1 | awk '/.*END-EN-FOOTER.*/,/\<\/html\>/' > tmp.epilogue
    cat tmp.prologue inc/header_en.html tmp.main inc/footer_en.html tmp.epilogue > $1
}

apply_template_japanese index.html
apply_template_japanese dl/index.html
apply_template_japanese dl/material/index.html

apply_template_english en/index.html
apply_template_english en/dl/index.html

apply_template_japanese award2023.html
apply_template_japanese award2024.html
apply_template_japanese doc/index.html
apply_template_japanese doc/font.html
apply_template_japanese doc/scriptgen.html
apply_template_japanese works/index.html
apply_template_japanese wiki/skin/pukiwiki.skin.php

apply_template_english en/doc/index.html
apply_template_english en/doc/font.html
apply_template_english en/doc/title.html
apply_template_english en/doc/wms.html
apply_template_english en/doc/faq.html
apply_template_english en/doc/scriptgen.html
apply_template_english en/works/index.html
apply_template_english en/wiki/skin/pukiwiki.skin.php

rm tmp.prologue tmp.main tmp.epilogue
