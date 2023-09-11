#!/bin/sh

# This script builds suika-pro.exe with mingw toolchain on Linux.

SUIKA2_ROOT="../.."

BUILDTMP=/mnt/c/Users/`powershell.exe '$env:UserName' | tr -d '\r\n'`/suika-pro-tmp;
rm -rf "$BUILDTMP"
mkdir "$BUILDTMP"
mkdir "$BUILDTMP/build"

DEPS="$BUILDTMP/deps"
mkdir "$DEPS"

# Copy CMakeLists.txt.
cp CMakeLists.txt "$BUILDTMP/"

# Copy the Pro source files.
SRC_PRO="\
	main.cpp \
	openglwidget.h \
	openglwidget.cpp \
	glwrapper.cpp \
	mainwindow.h \
	mainwindow.cpp \
	mainwindow.ui \
"
for file in $SRC_PRO; do cp "$SUIKA2_ROOT/src/qt$file" "$BUILDTMP/$file"; done

# Copy the engine source files.
SRC_MAIN="\
	anime.h \
	anime.c \
	cmd_anime.c \
	cmd_bg.c \
	cmd_bgm.c \
	cmd_ch.c \
	cmd_cha.c \
	cmd_chapter.c \
	cmd_chs.c \
	cmd_click.c \
	cmd_gosub.c \
	cmd_goto.c \
	cmd_gui.c \
	cmd_if.c \
	cmd_load.c \
	cmd_menu.c \
	cmd_message.c \
	cmd_retrospect.c \
	cmd_return.c \
	cmd_se.c \
	cmd_set.c \
	cmd_setconfig.c \
	cmd_setsave.c \
	cmd_shake.c \
	cmd_skip.c \
	cmd_switch.c \
	cmd_video.c \
	cmd_vol.c \
	cmd_wait.c \
	cmd_wms.c \
	conf.h \
	conf.c \
	drawglyph.h \
	drawimage.h \
	event.h \
	event.c \
	file.h \
	file.c \
	glhelper.h \
	glrender.h \
	glrender.c \
	glyph.h \
	glyph.c \
	gui.h \
	gui.c \
	history.h \
	history.c \
	image.h \
	image.c \
	key.h \
	log.h \
	log.c \
	main.h \
	main.c \
	mixer.h \
	mixer.c \
	package.h \
	package.c \
	platform.h \
	readimage.c \
	readjpeg.c \
	save.h \
	save.c \
	scalesamples.h \
	scbuf.h \
	scbuf.c \
	script.h \
	script.c \
	seen.h \
	seen.c \
	stage.h \
	stage.c \
	suika.h \
	types.h \
	uimsg.h \
	uimsg.c \
	vars.h \
	vars.c \
	wave.h \
	wave.c \
	wms.h \
	wms_core.h \
	wms_core.c \
	wms_impl.c \
	wms_lexer.yy.c \
	wms_parser.tab.h \
	wms_parser.tab.c \
"
for file in $SRC_MAIN; do cp "$SUIKA2_ROOT/src/$file" "$DEPS/"; done

# Add a Suika2 CMakeLists.txt
cp cmake/suika2.txt "$DEPS/CMakeLists.txt"

# Copy libpng source files and add a custom CMakeLists.txt
tar xzf "$SUIKA2_ROOT/build/libsrc/libpng-1.6.35.tar.gz" -C "$DEPS"
mv "$DEPS/libpng-1.6.35" "$DEPS/libpng"
cp "$DEPS/libpng/scripts/pnglibconf.h.prebuilt" "$DEPS/libpng/pnglibconf.h"
cp cmake/libpng.txt "$DEPS/libpng/CMakeLists.txt"

# Copy zlib source files and add a custom CMakeLists.txt as a child of libpng
tar xzf "$SUIKA2_ROOT/build/libsrc/zlib-1.2.11.tar.gz" -C "$DEPS/libpng"
mv "$DEPS/libpng/zlib-1.2.11" "$DEPS/libpng/zlib"
cp cmake/zlib.txt "$DEPS/libpng/zlib/CMakeLists.txt"

# Copy jpeg source files and add a custom CMakeLists.txt
tar xzf "$SUIKA2_ROOT/build/libsrc/jpegsrc.v9e.tar.gz" -C "$DEPS"
mv "$DEPS/jpeg-9e" "$DEPS/jpeg"
cp cmake/jpeg.txt "$DEPS/jpeg/CMakeLists.txt"
cp "$DEPS/jpeg/jconfig.txt" "$DEPS/jpeg/jconfig.h"

# Copy libogg source files add a custom CMakeLists.txt
tar xzf "$SUIKA2_ROOT/build/libsrc/libogg-1.3.3.tar.gz" -C "$DEPS"
mv "$DEPS/libogg-1.3.3" "$DEPS/libogg"
cp "$DEPS/libogg/include/ogg/config_types.h.in" "$DEPS/libogg/include/ogg/config_types.h"
sed -i 's/@INCLUDE_INTTYPES_H@/1/g' "$DEPS/libogg/include/ogg/config_types.h"
sed -i 's/@INCLUDE_STDINT_H@/1/g' "$DEPS/libogg/include/ogg/config_types.h"
sed -i 's/@INCLUDE_SYS_TYPES_H@/1/g' "$DEPS/libogg/include/ogg/config_types.h"
sed -i 's/@SIZE16@/short/g' "$DEPS/libogg/include/ogg/config_types.h"
sed -i 's/@USIZE16@/unsigned short/g' "$DEPS/libogg/include/ogg/config_types.h"
sed -i 's/@SIZE32@/int/g' "$DEPS/libogg/include/ogg/config_types.h"
sed -i 's/@USIZE32@/unsigned int/g' "$DEPS/libogg/include/ogg/config_types.h"
sed -i 's/@SIZE64@/long/g' "$DEPS/libogg/include/ogg/config_types.h"
sed -i 's/@USIZE64@/unsigned long/g' "$DEPS/libogg/include/ogg/config_types.h"
cp cmake/libogg.txt "$DEPS/libogg/CMakeLists.txt"

# Copy libvorbis source files add a custom CMakeLists.txt
tar xzf "$SUIKA2_ROOT/build/libsrc/libvorbis-1.3.6.tar.gz" -C "$DEPS"
mv "$DEPS/libvorbis-1.3.6" "$DEPS/libvorbis"
cp cmake/libvorbis.txt "$DEPS/libvorbis/CMakeLists.txt"

# Copy freetype source files add a custom CMakeLists.txt
tar xzf "$SUIKA2_ROOT/build/libsrc/freetype-2.9.1.tar.gz" -C "$DEPS"
mv "$DEPS/freetype-2.9.1" "$DEPS/freetype"
cp cmake/freetype.txt "$DEPS/freetype/CMakeLists.txt"

# Run cmake.
rm -rf build
mkdir build
cd build
cmake \
    -DCMAKE_PREFIX_PATH=/opt/qt/lib/cmake \
    -DCMAKE_TOOLCHAIN_FILE=/opt/qt6/lib/cmake/Qt6/qt.toolchain.cmake \
    -DQt6_DIR=/opt/qt \
    -DQT_HOST_PATH=/usr/lib/qt6 \
    -DCMAKE_FIND_ROOT_PATH=/opt/qt \
    -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc \
    -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ \
    ..
cmake --build .
