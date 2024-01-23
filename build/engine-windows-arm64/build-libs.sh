#!/bin/sh

set -eu

PREFIX=`pwd`/libroot
PATH=`pwd`/llvm/bin:$PATH
INCLUDE=`pwd`/llvm/aarch64-w64-mingw32/include
CC=aarch64-w64-mingw32-gcc
LD=aarch64-w64-mingw32-ld
WINDRES=aarch64-w64-mingw32-windres

rm -rf tmp libroot llvm llvm-mingw-20230614-ucrt-ubuntu-20.04-x86_64.tar.xz
mkdir -p tmp libroot
mkdir -p libroot/include libroot/lib

if [ ! -z "`uname | grep Linux`" ]; then
	if [ ! -z "`uname -a | grep x86`" ]; then
		wget https://github.com/mstorsjo/llvm-mingw/releases/download/20230614/llvm-mingw-20230614-ucrt-ubuntu-20.04-x86_64.tar.xz;
		tar xJf llvm-mingw-20230614-ucrt-ubuntu-20.04-x86_64.tar.xz;
		mv llvm-mingw-20230614-ucrt-ubuntu-20.04-x86_64 llvm;
	else
		wget https://github.com/mstorsjo/llvm-mingw/releases/download/20230614/llvm-mingw-20230614-ucrt-ubuntu-20.04-aarch64.tar.xz;
		tar xJf llvm-mingw-20230614-ucrt-ubuntu-20.04-aarch64.tar.xz;
		mv llvm-mingw-20230614-ucrt-ubuntu-20.04-aarch64 llvm;
	fi;
else
    curl -L -O https://github.com/mstorsjo/llvm-mingw/releases/download/20230614/llvm-mingw-20230614-ucrt-macos-universal.tar.xz;
    tar xJf llvm-mingw-20230614-ucrt-macos-universal.tar.xz;
    mv llvm-mingw-20230614-ucrt-macos-universal llvm;
fi

cd tmp

echo 'Building bzip2...'
tar xzf ../../libsrc/bzip2-1.0.6.tar.gz
cd bzip2-1.0.6
make libbz2.a PREFIX=aarch64-w64-mingw32-gcc CFLAGS='-O3 -ffunction-sections -fdata-sections' CC=aarch64-w64-mingw32-gcc
cp bzlib.h ../../libroot/include/
cp libbz2.a ../../libroot/lib/
cd ..

echo 'Building libwebp...'
tar xzf ../../libsrc/libwebp-1.3.2.tar.gz
cd libwebp-1.3.2
./configure --prefix=$PREFIX --enable-static --disable-shared --host=aarch64-w64-mingw32 CPPFLAGS=-I$PREFIX/include CFLAGS='-O3 -ffunction-sections -fdata-sections' LDFLAGS=-L$PREFIX/lib CC=aarch64-w64-mingw32-gcc
make
make install
cd ..

echo 'Building zlib...'
tar xzf ../../libsrc/zlib-1.2.11.tar.gz
cd zlib-1.2.11
make -f win32/Makefile.gcc PREFIX=aarch64-w64-mingw32- CFLAGS='-O3 -ffunction-sections -fdata-sections'
mkdir -p ../../libroot/include ../../libroot/lib
cp zlib.h zconf.h ../../libroot/include/
cp libz.a ../../libroot/lib/
cd ..

echo 'Building libpng...'
tar xzf ../../libsrc/libpng-1.6.35.tar.gz
cd libpng-1.6.35
./configure --prefix=$PREFIX --enable-static --disable-shared --host=aarch64-w64-mingw32 CPPFLAGS=-I$PREFIX/include CFLAGS='-O3 -ffunction-sections -fdata-sections' LDFLAGS=-L$PREFIX/lib CC=aarch64-w64-mingw32-gcc
make
make install
cd ..

echo 'Building jpeg9...'
tar xzf ../../libsrc/jpegsrc.v9e.tar.gz
cd jpeg-9e
./configure --prefix=$PREFIX --enable-static --disable-shared --host=aarch64-w64-mingw32 CPPFLAGS=-I$PREFIX/include CFLAGS='-O3 -ffunction-sections -fdata-sections' LDFLAGS=-L$PREFIX/lib CC=aarch64-w64-mingw32-gcc
make
make install
cd ..

echo 'Building libogg...'
tar xzf ../../libsrc/libogg-1.3.3.tar.gz
cd libogg-1.3.3
./configure --prefix=$PREFIX --enable-static --disable-shared --host=aarch64-w64-mingw32 CFLAGS='-O3 -ffunction-sections -fdata-sections' CC=aarch64-w64-mingw32-gcc
make
make install
cd ..

echo 'Building libvorbis...'
tar xzf ../../libsrc/libvorbis-1.3.6.tar.gz
cd libvorbis-1.3.6
./configure --prefix=$PREFIX --enable-static --disable-shared --host=aarch64-w64-mingw32 PKG_CONFIG="" --with-ogg-includes=$PREFIX/include --with-ogg-libraries=$PREFIX/lib CFLAGS='-O3 -ffunction-sections -fdata-sections' CC=aarch64-w64-mingw32-gcc
make
make install
cd ..

echo 'Building freetyp2...'
tar xzf ../../libsrc/freetype-2.13.2.tar.gz
cd freetype-2.13.2
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
# Add |tee to avoid freeze on Emacs shell
./configure --prefix=$PREFIX --host=aarch64-w64-mingw32 --enable-static --disable-shared --with-png=yes --with-harfbuzz=no --with-zlib=yes --with-bzip2=yes --with-brotli=no CFLAGS='-O3 -ffunction-sections -fdata-sections' ZLIB_CFLAGS='-I../../libroot/include' ZLIB_LIBS='-L../../libroot/lib -lz' BZIP2_CFLAGS='-I../../libroot/include' BZIP2_LIBS='-L../../libroot/lib -lbz2' LIBPNG_CFLAGS='-I../../libroot/include' LIBPNG_LIBS='-L../../libroot/lib -lpng' CC=aarch64-w64-mingw32-gcc | tee
make | tee
make install | tee
cd ..

cd ..
rm -rf tmp llvm-mingw-20230614-ucrt-ubuntu-20.04-x86_64.tar.xz
rm -rf tmp llvm-mingw-20230614-ucrt-macos-universal.tar.xz
