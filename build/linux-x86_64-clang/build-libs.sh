#!/bin/sh

PREFIX=`pwd`/libroot
CC=clang

rm -rf tmp libroot
mkdir -p tmp libroot

cd tmp

tar xzf ../../libsrc/zlib-1.2.11.tar.gz
cd zlib-1.2.11
CC=clang LD=lld ./configure --prefix=$PREFIX --static
make -j4 CFLAGS='-O3 -ffunction-sections -fdata-sections'
make install
cd ..

tar xzf ../../libsrc/libpng-1.6.35.tar.gz
cd libpng-1.6.35
CC=clang LD=lld ./configure --prefix=$PREFIX --disable-shared CPPFLAGS=-I$PREFIX/include CFLAGS='-O3 -ffunction-sections -fdata-sections' LDFLAGS=-L$PREFIX/lib
make -j4
make install
cd ..

tar xzf ../../libsrc/jpegsrc.v9e.tar.gz
cd jpeg-9e
CC=clang LD=lld ./configure --prefix=$PREFIX --disable-shared CPPFLAGS=-I$PREFIX/include CFLAGS='-O3 -ffunction-sections -fdata-sections' LDFLAGS=-L$PREFIX/lib
make -j4
make install
cd ..

tar xzf ../../libsrc/libogg-1.3.3.tar.gz
cd libogg-1.3.3
CC=clang LD=lld ./configure --prefix=$PREFIX --disable-shared CFLAGS='-O3 -ffunction-sections -fdata-sections'
make -j4
make install
cd ..

tar xzf ../../libsrc/libvorbis-1.3.6.tar.gz
cd libvorbis-1.3.6
CC=clang LD=lld ./configure --prefix=$PREFIX --disable-shared --with-ogg-includes=$PREFIX/include --with-ogg-libraries=$PREFIX/lib CFLAGS='-O3 -ffunction-sections -fdata-sections'
make -j4
make install
cd ..

tar xzf ../../libsrc/freetype-2.9.1.tar.gz
cd freetype-2.9.1
sed -e 's/FONT_MODULES += type1//' \
    -e 's/FONT_MODULES += cid//' \
    -e 's/FONT_MODULES += pfr//' \
    -e 's/FONT_MODULES += type42//' \
    -e 's/FONT_MODULES += pcf//' \
    -e 's/FONT_MODULES += bdf//' \
    -e 's/FONT_MODULES += pshinter//' \
    -e 's/FONT_MODULES += raster//' \
    -e 's/FONT_MODULES += psaux//' \
    -e 's/FONT_MODULES += psnames//' \
    < modules.cfg > modules.cfg.new
mv modules.cfg.new modules.cfg
CC=clang LD=lld ./configure --prefix=$PREFIX --disable-shared --with-png=no --with-zlib=no --with-harfbuzz=no --with-bzip2=no CFLAGS='-O3 -ffunction-sections -fdata-sections'
make -j4
make install
cd ..

cd ..
rm -rf tmp
