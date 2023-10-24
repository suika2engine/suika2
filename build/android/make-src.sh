#!/bin/sh

set -eu

TARGET=android-src

# Remove temporary build files.
rm -rf \
   app/build \
   app/.cxx \
   app/src/main/cpp/*.[ch] \
   app/src/main/cpp/jpeg \
   app/src/main/cpp/freetype \
   app/src/main/cpp/libpng \
   "$TARGET"

# Create the target directory.
mkdir "$TARGET"

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
	drawglyph.h \
	drawimage.h \
	event.c \
	event.h \
	file.h \
	glrender.c \
	glrender.h \
	glyph.c \
	glyph.h \
	gui.c \
	gui.h \
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
	ndkfile.c \
	ndkmain.c \
	ndkmain.h \
	ndkwave.c \
	platform.h \
	readimage.c \
	readjpeg.c \
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
	suika.h \
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
    cp "../../src/$f" "$TARGET/app/src/main/cpp/$f";
done

# Extract freetype2 source into the android project tree.
tar xzf ../libsrc/freetype-2.9.1.tar.gz -C "$TARGET/app/src/main/cpp/"
mv "$TARGET/app/src/main/cpp/freetype-2.9.1" "$TARGET/app/src/main/cpp/freetype"

# Extract libpng source into the android project tree.
tar xzf ../libsrc/libpng-1.6.35.tar.gz -C "$TARGET/app/src/main/cpp/"
mv "$TARGET/app/src/main/cpp/libpng-1.6.35" "$TARGET/app/src/main/cpp/libpng"
cp "$TARGET/app/src/main/cpp/libpng/scripts/pnglibconf.h.prebuilt" "$TARGET/app/src/main/cpp/libpng/pnglibconf.h"

# Extract libjpeg source into the android project tree.
tar xzf ../libsrc/jpegsrc.v9e.tar.gz -C "$TARGET/app/src/main/cpp/"
mv "$TARGET/app/src/main/cpp/jpeg-9e" "$TARGET/app/src/main/cpp/jpeg"
cp cmakelists_for_libjpeg.txt "$TARGET/app/src/main/cpp/jpeg/CMakeLists.txt"
cp "$TARGET/app/src/main/cpp/jpeg/jconfig.txt" "$TARGET/app/src/main/cpp/jpeg/jconfig.h"

# Create a placeholder for assets.
mkdir "$TARGET/app/src/main/assets"
