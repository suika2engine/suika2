#!/bin/sh

set -eu

SED=sed
if [ ! -z "`which gsed`" ]; then
    SED=gsed;
fi

# Copy dependency source files to $DEPS directory.
DEPS=deps

# Set Suika2 root direcotry
SUIKA2_ROOT="../.."

# Reconstruct $DEPS
rm -rf "$DEPS"
mkdir "$DEPS"

# Copy Suika2 source files to $DEPS directory.
SRC="\
	khronos/glrender.h \
	khronos/glrender.c \
	khronos/glhelper.h \
	linux/asound.h \
	linux/asound.c \
	suika.h \
	pro.h \
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
	cmd_layer.c \
	cmd_load.c \
	cmd_message.c \
	cmd_pencil.c \
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
	anime.h \
	anime.c \
	conf.h \
	conf.c \
	ciel.c \
	event.h \
	event.c \
	file.h \
	file.c \
	glyph.h \
	glyph.c \
	gui.h \
	gui.c \
	hal.h \
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
	motion.h \
	package.h \
	package.c \
	readimage.c \
	readpng.c \
	readjpeg.c \
	readwebp.c \
	save.h \
	save.c \
	scbuf.h \
	scbuf.c \
	script.h \
	script.c \
	seen.h \
	seen.c \
	stage.h \
	stage.c \
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
for file in $SRC; do
    cp "$SUIKA2_ROOT/src/$file" "$DEPS/`basename $file`"
done

# Copy Suika2's CMakeLists.txt
cp cmake/suika2.txt "$DEPS/CMakeLists.txt"

# Copy libogg source files
tar xzf "$SUIKA2_ROOT/build/libsrc/libogg-1.3.3.tar.gz" -C "$DEPS"
mv "$DEPS/libogg-1.3.3" "$DEPS/libogg"
cp deps/libogg/include/ogg/config_types.h.in deps/libogg/include/ogg/config_types.h
$SED -i 's/@INCLUDE_INTTYPES_H@/1/g' deps/libogg/include/ogg/config_types.h
$SED -i 's/@INCLUDE_STDINT_H@/1/g' deps/libogg/include/ogg/config_types.h
$SED -i 's/@INCLUDE_SYS_TYPES_H@/1/g' deps/libogg/include/ogg/config_types.h
$SED -i 's/@SIZE16@/short/g' deps/libogg/include/ogg/config_types.h
$SED -i 's/@USIZE16@/unsigned short/g' deps/libogg/include/ogg/config_types.h
$SED -i 's/@SIZE32@/int/g' deps/libogg/include/ogg/config_types.h
$SED -i 's/@USIZE32@/unsigned int/g' deps/libogg/include/ogg/config_types.h
$SED -i 's/@SIZE64@/long/g' deps/libogg/include/ogg/config_types.h
$SED -i 's/@USIZE64@/unsigned long/g' deps/libogg/include/ogg/config_types.h
cp cmake/libogg.txt "$DEPS/libogg/CMakeLists.txt"

# Copy libvorbis source files
tar xzf "$SUIKA2_ROOT/build/libsrc/libvorbis-1.3.6.tar.gz" -C "$DEPS"
mv "$DEPS/libvorbis-1.3.6" "$DEPS/libvorbis"
cp cmake/libvorbis.txt "$DEPS/libvorbis/CMakeLists.txt"
