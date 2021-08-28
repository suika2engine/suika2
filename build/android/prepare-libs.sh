#!/bin/sh

tar xzf ../libsrc/freetype-2.9.1.tar.gz -C app/src/main/cpp/
mv app/src/main/cpp/freetype-2.9.1 app/src/main/cpp/freetype

tar xzf ../libsrc/libpng-1.6.35.tar.gz -C app/src/main/cpp/
mv app/src/main/cpp/libpng-1.6.35 app/src/main/cpp/libpng
