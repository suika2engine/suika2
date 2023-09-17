#!/bin/sh

PREFIX=`pwd`/libroot

rm -rf tmp libroot
mkdir -p tmp libroot

cd tmp

tar xzf ../../libsrc/zlib-1.2.11.tar.gz
cd zlib-1.2.11
make -j4 -f win32/Makefile.gcc PREFIX=i686-w64-mingw32- CFLAGS='-O3 -ffunction-sections -fdata-sections'
mkdir -p ../../libroot/include ../../libroot/lib
cp zlib.h zconf.h ../../libroot/include/
cp libz.a ../../libroot/lib/
cd ..

tar xzf ../../libsrc/libpng-1.6.35.tar.gz
cd libpng-1.6.35
./configure --prefix=$PREFIX --disable-shared --host=i686-w64-mingw32 CPPFLAGS=-I$PREFIX/include CFLAGS='-O3 -ffunction-sections -fdata-sections' LDFLAGS=-L$PREFIX/lib
make -j4
make install
cd ..

tar xzf ../../libsrc/jpegsrc.v9e.tar.gz
cd jpeg-9e
./configure --prefix=$PREFIX --disable-shared --host=i686-w64-mingw32 CPPFLAGS=-I$PREFIX/include CFLAGS='-O3 -ffunction-sections -fdata-sections' LDFLAGS=-L$PREFIX/lib
make -j4
make install
cd ..

tar xzf ../../libsrc/libogg-1.3.3.tar.gz
cd libogg-1.3.3
./configure --prefix=$PREFIX --host=i686-w64-mingw32 --disable-shared CFLAGS='-O3 -ffunction-sections -fdata-sections'
make -j4
make install
cd ..

tar xzf ../../libsrc/libvorbis-1.3.6.tar.gz
cd libvorbis-1.3.6
./configure --prefix=$PREFIX --host=i686-w64-mingw32 --disable-shared PKG_CONFIG="" --with-ogg-includes=$PREFIX/include --with-ogg-libraries=$PREFIX/lib CFLAGS='-O3 -ffunction-sections -fdata-sections'
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
./configure --host=i686-w64-mingw32 --disable-shared --with-png=no --with-zlib=no --with-harfbuzz=no --with-bzip2=no --prefix=$PREFIX CFLAGS='-O3 -ffunction-sections -fdata-sections' | tee
make -j4 | tee
make install | tee
cd ..

cd ..
rm -rf tmp
