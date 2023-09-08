#!/bin/sh

if [ `uname -m` = "x86_64" ]; then
    curl -O https://suika2.com/dl/libroot-mac.tar.gz
    tar xzf libroot-mac.tar.gz
    rm libroot-mac.tar.gz
    exit 0
fi

PREFIX=`pwd`/libroot

export MACOSX_DEPLOYMENT_TARGET=10.13

rm -rf tmp libroot
mkdir -p tmp libroot

cd tmp

tar xzf ../../libsrc/zlib-1.2.11.tar.gz
cd zlib-1.2.11
./configure --prefix=$PREFIX --static --archs="-arch arm64 -arch x86_64"
make
make install
cd ..

tar xzf ../../libsrc/libpng-1.6.35.tar.gz
cd libpng-1.6.35
./configure --prefix=$PREFIX --disable-shared CPPFLAGS=-I$PREFIX/include CFLAGS="-arch arm64 -arch x86_64" LDFLAGS="-L$PREFIX/lib -arch arm64 -arch x86_64"
make
make install
cd ..

tar xzf ../../libsrc/jpegsrc.v9e.tar.gz
cd jpeg-9e
./configure --prefix=$PREFIX --disable-shared CPPFLAGS=-I$PREFIX/include CFLAGS="-arch arm64 -arch x86_64" LDFLAGS="-L$PREFIX/lib -arch arm64 -arch x86_64"
make
make install
cd ..

tar xzf ../../libsrc/libogg-1.3.3.tar.gz
cd libogg-1.3.3
./configure --prefix=$PREFIX --disable-shared CFLAGS="-arch arm64 -arch x86_64" LDFLAGS="-arch arm64 -arch x86_64"
make
make install
cd ..

tar xzf ../../libsrc/libvorbis-1.3.6.tar.gz
cd libvorbis-1.3.6
./configure --prefix=$PREFIX --disable-shared PKG_CONFIG="" --with-ogg-includes=$PREFIX/include --with-ogg-libraries=$PREFIX/lib CFLAGS="-arch arm64 -arch x86_64" LDFLAGS="-arch arm64 -arch x86_64"
make
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
./configure --prefix=$PREFIX --disable-shared --with-png=no --with-zlib=no --with-harfbuzz=no --with-bzip2=no CFLAGS="-arch arm64 -arch x86_64" LDFLAGS="-arch arm64 -arch x86_64"
make
make install
cd ..

cd ..
rm -rf tmp

unset MACOSX_DEPLOYMENT_TARGET
