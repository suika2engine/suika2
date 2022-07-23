#!/bin/sh

rm -rf app/src/main/cpp/freetype
rm -rf app/src/main/cpp/libpng
rm -rf app/src/main/cpp/libogg
rm -rf app/src/main/cpp/libvorbis
rm -rf app/src/main/cpp/jpeg

tar xzf ../libsrc/freetype-2.9.1.tar.gz -C app/src/main/cpp/
mv app/src/main/cpp/freetype-2.9.1 app/src/main/cpp/freetype

tar xzf ../libsrc/libpng-1.6.35.tar.gz -C app/src/main/cpp/
mv app/src/main/cpp/libpng-1.6.35 app/src/main/cpp/libpng
cp app/src/main/cpp/libpng/scripts/pnglibconf.h.prebuilt app/src/main/cpp/libpng/pnglibconf.h

tar xzf ../libsrc/jpegsrc.v9e.tar.gz -C app/src/main/cpp/
mv app/src/main/cpp/jpeg-9e app/src/main/cpp/jpeg
cp cmakelists_for_libjpeg.txt app/src/main/cpp/jpeg/CMakeLists.txt
cp app/src/main/cpp/jpeg/jconfig.txt app/src/main/cpp/jpeg/jconfig.h
