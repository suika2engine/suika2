#!/bin/sh

set -eu

TARGET=android-src

# Remove temporary build files.
rm -rf \
   app/build \
   app/.cxx \
   app/src/main/cpp \
   "$TARGET"

# Create the target directory.
mkdir "$TARGET"
mkdir -p "$TARGET/app/src/main/cpp"

# Copy the base files.
COPY_LIST="\
	app \
	gradle.properties \
	build.gradle \
	gradlew \
	settings.gradle \
	gradlew.bat \
	gradle \
"
for f in $COPY_LIST; do
    cp -R "$f" "$TARGET/";
done

# Copy the Suika2 source files.
COPY_LIST="\
	google/ndkmain.h \
	google/ndkmain.c \
	google/ndkfile.c \
	google/ndkwave.c \
	khronos/glrender.c \
	khronos/glrender.h \
	suika.h \
	anime.c \
	anime.h \
	cmd_anime.c \
	cmd_bg.c \
	cmd_bgm.c \
	cmd_cha.c \
	cmd_chapter.c \
	cmd_ch.c \
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
	conf.c \
	conf.h \
	event.c \
	event.h \
	file.h \
	glyph.c \
	glyph.h \
	gui.c \
	gui.h \
	hal.h \
	history.c \
	history.h \
	image.c \
	image.h \
	log.c \
	log.h \
	main.c \
	main.h \
	mixer.c \
	mixer.h \
	readimage.c \
	readpng.c \
	readjpeg.c \
	readwebp.c \
	save.c \
	save.h \
	scbuf.c \
	scbuf.h \
	script.c \
	script.h \
	seen.c \
	seen.h \
	stage.c \
	stage.h \
	types.h \
	vars.c \
	vars.h \
	wave.h \
	wms_core.c \
	wms_core.h \
	wms.h \
	wms_impl.c \
	wms_lexer.yy.c \
	wms_parser.tab.c \
	wms_parser.tab.h \
"
for f in $COPY_LIST; do
    cp "../../src/$f" "$TARGET/app/src/main/cpp/`basename $f`";
done

# Extract libpng source into the android project tree.
mkdir -p "$TARGET/app/src/main/cpp/libpng"
tar xzf ../libsrc/libpng-1.6.35.tar.gz -C "$TARGET/app/src/main/cpp/libpng" --strip-components 1
cp "$TARGET/app/src/main/cpp/libpng/scripts/pnglibconf.h.prebuilt" "$TARGET/app/src/main/cpp/libpng/pnglibconf.h"

# Extract jpeg9 source into the android project tree.
mkdir -p "$TARGET/app/src/main/cpp/jpeg"
tar xzf ../libsrc/jpegsrc.v9e.tar.gz -C "$TARGET/app/src/main/cpp/jpeg" --strip-components 1
cp cmake/jpeg.txt "$TARGET/app/src/main/cpp/jpeg/CMakeLists.txt"
cp "$TARGET/app/src/main/cpp/jpeg/jconfig.txt" "$TARGET/app/src/main/cpp/jpeg/jconfig.h"

# Extract bzip2 into the android project tree.
mkdir -p "$TARGET/app/src/main/cpp/bzip2"
tar xzf ../libsrc/bzip2-1.0.6.tar.gz -C "$TARGET/app/src/main/cpp/bzip2" --strip-components 1
cp cmake/bzip2.txt "$TARGET/app/src/main/cpp/bzip2/CMakeLists.txt"

# Extracting libwebp into the android project tree.
mkdir -p "$TARGET/app/src/main/cpp/libwebp"
tar xzf ../libsrc/libwebp-1.3.2.tar.gz -C "$TARGET/app/src/main/cpp/libwebp" --strip-components 1

# Extract freetype2 source into the android project tree.
mkdir -p "$TARGET/app/src/main/cpp/freetype"
tar xzf ../libsrc/freetype-2.9.1.tar.gz -C "$TARGET/app/src/main/cpp/freetype" --strip-components 1

# Create a placeholder for assets.
mkdir -p "$TARGET/app/src/main/assets"
