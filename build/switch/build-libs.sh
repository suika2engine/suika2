#!/bin/sh

# prepare env
CORE_NUM=$(cat /proc/cpuinfo | grep -c ^processor)
if [ -z "$DEVKITPRO" ]; then
    echo please make the DEVKITPRO in your env
    exit
fi
export PATH=$DEVKITPRO/devkitA64/bin:$PATH
export CC=$(which aarch64-none-elf-gcc)
export AR=$(which aarch64-none-elf-ar)
echo "## DEVKITPRO=$DEVKITPRO"
echo "## CC=$CC"

# prepare dir
PREFIX=`pwd`/libroot

# rm -rf tmp libroot
mkdir -p tmp libroot

cd tmp

# build zlib
tar xzf ../../libsrc/zlib-1.2.11.tar.gz
cd zlib-1.2.11
./configure --prefix=$PREFIX --static
make -j$CORE_NUM CFLAGS='-O3 -ffunction-sections -fdata-sections'
make install
cd ..

# build png
tar xzf ../../libsrc/libpng-1.6.35.tar.gz
cd libpng-1.6.35
./configure --prefix=$PREFIX \
    --disable-shared --host=aarch64-none-elf \
    CPPFLAGS=-I$PREFIX/include \
    CFLAGS='-O3 -ffunction-sections -fdata-sections' \
    LDFLAGS=-L$PREFIX/lib
make -j$CORE_NUM
make install
cd ..

# build jpeg
tar xzf ../../libsrc/jpegsrc.v9e.tar.gz
cd jpeg-9e
./configure --prefix=$PREFIX \
    --disable-shared --host=aarch64-none-elf \
    CPPFLAGS=-I$PREFIX/include \
    CFLAGS='-O3 -ffunction-sections -fdata-sections' \
    LDFLAGS=-L$PREFIX/lib
make -j$CORE_NUM
make install
cd ..

# build ogg
tar xzf ../../libsrc/libogg-1.3.3.tar.gz
cd libogg-1.3.3
./configure --prefix=$PREFIX \
    --disable-shared --host=aarch64-none-elf \
    CFLAGS='-O3 -ffunction-sections -fdata-sections'
make -j$CORE_NUM
make install
cd ..

# build vorbis
tar xzf ../../libsrc/libvorbis-1.3.6.tar.gz
cd libvorbis-1.3.6
./configure --prefix=$PREFIX \
    --disable-shared --host=aarch64-none-elf \
    --with-ogg-includes=$PREFIX/include --with-ogg-libraries=$PREFIX/lib \
    CFLAGS='-O3 -ffunction-sections -fdata-sections'
make -j$CORE_NUM
make install
cd ..

# build free type, if windows must have gcc in path
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
./configure --prefix=$PREFIX \
    --disable-shared --host=aarch64-none-elf \
    --with-png=no --with-zlib=no --with-harfbuzz=no --with-bzip2=no \
    CFLAGS='-O3 -ffunction-sections -fdata-sections'
make -j$CORE_NUM
make install
cd ..
exit

cd ..
rm -rf tmp
