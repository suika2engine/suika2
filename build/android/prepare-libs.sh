#!/bin/sh

rm -rf app/src/main/cpp/freetype app/src/main/cpp/libpng \
       app/src/main/cpp/libogg app/src/main/cpp/libvorbis

tar xzf ../libsrc/freetype-2.9.1.tar.gz -C app/src/main/cpp/
mv app/src/main/cpp/freetype-2.9.1 app/src/main/cpp/freetype

tar xzf ../libsrc/libpng-1.6.35.tar.gz -C app/src/main/cpp/
mv app/src/main/cpp/libpng-1.6.35 app/src/main/cpp/libpng
cp app/src/main/cpp/libpng/scripts/pnglibconf.h.prebuilt app/src/main/cpp/libpng/pnglibconf.h
