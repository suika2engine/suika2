#!/bin/sh

PREFIX=`pwd`/libroot

rm -rf tmp libroot
mkdir -p tmp libroot

cd tmp

tar xzf ../../libsrc/zlib-1.2.11.tar.gz
cd zlib-1.2.11
make -f win32/Makefile.gcc
mkdir -p ../../libroot/include ../../libroot/lib
cp zlib.h zconf.h ../../libroot/include/
cp libz.a ../../libroot/lib/
cd ..

tar xzf ../../libsrc/libpng-1.6.28.tar.gz
cd libpng-1.6.28
./configure --prefix=$PREFIX --disable-shared CPPFLAGS=-I$PREFIX/include LDFLAGS=-L$PREFIX/lib
make
make install
cd ..

tar xzf ../../libsrc/libogg-1.3.2.tar.gz
cd libogg-1.3.2
./configure --prefix=$PREFIX --disable-shared
make
make install
cd ..

tar xzf ../../libsrc/libvorbis-1.3.5.tar.gz
cd libvorbis-1.3.5
./configure --prefix=$PREFIX --disable-shared --with-ogg-includes=$PREFIX/include --with-ogg-libraries=$PREFIX/lib
make
make install
cd ..

tar xzf ../../libsrc/freetype-2.7.1.tar.gz
cd freetype-2.7.1
./configure --disable-shared --with-png=no --with-zlib=no --with-harfbuzz=no --with-bzip2=no --prefix=$PREFIX
make
make install
cd ..

cd ..
rm -rf tmp
