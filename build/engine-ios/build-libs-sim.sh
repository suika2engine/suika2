#!/bin/sh

set -eu

PREFIX=`pwd`/libroot

rm -rf tmp libroot
mkdir -p tmp libroot libroot/include libroot/lib libroot/bin

cd tmp

echo 'Building brotli...'
tar xzf ../../libsrc/brotli-1.1.0.tar.gz
cp ../Makefile.brotli-sim brotli-1.1.0/Makefile
cd brotli-1.1.0
make
cd ..

echo 'building bzip2...'
tar xzf ../../libsrc/bzip2-1.0.6.tar.gz
cd bzip2-1.0.6
make libbz2.a CFLAGS="-O3 -arch arm64 -arch x86_64 -isysroot `xcrun -sdk iphonesimulator --show-sdk-path`"
cp bzlib.h ../../libroot/include/
cp libbz2.a ../../libroot/lib/
cd ..

echo 'building libwebp...'
tar xzf ../../libsrc/libwebp-1.3.2.tar.gz
cd libwebp-1.3.2
./configure --prefix=$PREFIX --host=arm-apple-darwin --disable-shared CPPFLAGS=-I$PREFIX/include CFLAGS="-O3 -arch arm64 -arch x86_64 -isysroot `xcrun -sdk iphonesimulator --show-sdk-path`" LDFLAGS="-L$PREFIX/lib -arch arm64 -arch x86_64"
make
make install
cd ..

echo 'building zlib...'
tar xzf ../../libsrc/zlib-1.2.11.tar.gz
cd zlib-1.2.11
CFLAGS="-O3 -arch arm64 -arch x86_64 -isysroot `xcrun -sdk iphonesimulator --show-sdk-path`" ./configure --prefix=$PREFIX --static --archs="-arch arm64 -arch x86_64"
make -j4
make install
cd ..

echo 'building libpng...'
tar xzf ../../libsrc/libpng-1.6.35.tar.gz
cd libpng-1.6.35
./configure --prefix=$PREFIX --host=arm-apple-darwin --disable-shared CPPFLAGS=-I$PREFIX/include CFLAGS="-O3 -arch arm64 -arch x86_64 -isysroot `xcrun -sdk iphonesimulator --show-sdk-path`" LDFLAGS="-L$PREFIX/lib -arch arm64 -arch x86_64"
make
make install
cd ..

echo 'building jpeg9...'
tar xzf ../../libsrc/jpegsrc.v9e.tar.gz
cd jpeg-9e
./configure --prefix=$PREFIX --host=arm-apple-darwin --disable-shared CPPFLAGS=-I$PREFIX/include CFLAGS="-O3 -arch arm64 -arch x86_64 -isysroot `xcrun -sdk iphonesimulator --show-sdk-path`" LDFLAGS="-L$PREFIX/lib -arch arm64 -arch x86_64"
make -j4
make install
cd ..

echo 'building libogg...'
tar xzf ../../libsrc/libogg-1.3.3.tar.gz
cd libogg-1.3.3
./configure --prefix=$PREFIX --host=arm-apple-darwin --disable-shared CFLAGS="-O3 -arch arm64 -arch x86_64 -isysroot `xcrun -sdk iphonesimulator --show-sdk-path`" LDFLAGS="-arch arm64 -arch x86_64"
make -j4
make install
cd ..

echo 'building libvorbis...'
tar xzf ../../libsrc/libvorbis-1.3.6.tar.gz
cd libvorbis-1.3.6
sed 's/-force_cpusubtype_ALL//' configure > configure.new
mv configure.new configure
chmod +x configure
./configure --prefix=$PREFIX --host=arm-apple-darwin --disable-shared PKG_CONFIG="" --with-ogg-includes=$PREFIX/include --with-ogg-libraries=$PREFIX/lib CFLAGS="-O3 -arch arm64 -arch x86_64 -isysroot `xcrun -sdk iphonesimulator --show-sdk-path`" LDFLAGS="-arch arm64 -arch x86_64"
make -j4
make install
cd ..

echo 'building freetype2...'
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
./configure --prefix=$PREFIX --host=arm-apple-darwin --disable-shared --with-png=no --with-zlib=no --with-harfbuzz=no --with-bzip2=no CFLAGS="-O3 -arch arm64 -arch x86_64 -isysroot `xcrun -sdk iphonesimulator --show-sdk-path`" LDFLAGS="-arch arm64 -arch x86_64"
make -j4
make install
cd ..

cd ..
rm -rf tmp

echo 'finished.'
