#!/bin/sh

# Extract bzip2 source into the android project tree.
rm -rf app/src/main/cpp/zlib2
tar xzf ../libsrc/bzip2-1.0.6.tar.gz -C app/src/main/cpp/
mv app/src/main/cpp/bzip2-1.0.6 app/src/main/cpp/bzip2
cp cmakelists_for_bzip2.txt app/src/main/cpp/bzip2/CMakeLists.txt

# Extract libwebp source into the android project tree.
rm -rf app/src/main/cpp/libwebp
tar xzf ../libsrc/libwebp-1.3.2.tar.gz -C app/src/main/cpp/
mv app/src/main/cpp/libwebp-1.3.2 app/src/main/cpp/libwebp

# Extract freetype2 source into the android project tree.
rm -rf app/src/main/cpp/freetype
tar xzf ../libsrc/freetype-2.9.1.tar.gz -C app/src/main/cpp/
mv app/src/main/cpp/freetype-2.9.1 app/src/main/cpp/freetype

# Extract libpng source into the android project tree.
rm -rf app/src/main/cpp/libpng
tar xzf ../libsrc/libpng-1.6.35.tar.gz -C app/src/main/cpp/
mv app/src/main/cpp/libpng-1.6.35 app/src/main/cpp/libpng
cp app/src/main/cpp/libpng/scripts/pnglibconf.h.prebuilt app/src/main/cpp/libpng/pnglibconf.h

# Extract libjpeg source into the android project tree.
rm -rf app/src/main/cpp/jpeg
tar xzf ../libsrc/jpegsrc.v9e.tar.gz -C app/src/main/cpp/
mv app/src/main/cpp/jpeg-9e app/src/main/cpp/jpeg
cp cmakelists_for_libjpeg.txt app/src/main/cpp/jpeg/CMakeLists.txt
cp app/src/main/cpp/jpeg/jconfig.txt app/src/main/cpp/jpeg/jconfig.h

# Copy the source files into the android project tree.
# We do copy because Windows doesn't support symlinks in a normal way.
cp ../../src/anime.c app/src/main/cpp/anime.c
cp ../../src/anime.h app/src/main/cpp/anime.h
cp ../../src/cmd_anime.c app/src/main/cpp/cmd_anime.c
cp ../../src/cmd_bg.c app/src/main/cpp/cmd_bg.c
cp ../../src/cmd_bgm.c app/src/main/cpp/cmd_bgm.c
cp ../../src/cmd_cha.c app/src/main/cpp/cmd_cha.c
cp ../../src/cmd_chapter.c app/src/main/cpp/cmd_chapter.c
cp ../../src/cmd_ch.c app/src/main/cpp/cmd_ch.c
cp ../../src/cmd_chs.c app/src/main/cpp/cmd_chs.c
cp ../../src/cmd_click.c app/src/main/cpp/cmd_click.c
cp ../../src/cmd_gosub.c app/src/main/cpp/cmd_gosub.c
cp ../../src/cmd_goto.c app/src/main/cpp/cmd_goto.c
cp ../../src/cmd_gui.c app/src/main/cpp/cmd_gui.c
cp ../../src/cmd_if.c app/src/main/cpp/cmd_if.c
cp ../../src/cmd_load.c app/src/main/cpp/cmd_load.c
cp ../../src/cmd_message.c app/src/main/cpp/cmd_message.c
cp ../../src/cmd_pencil.c app/src/main/cpp/cmd_pencil.c
cp ../../src/cmd_return.c app/src/main/cpp/cmd_return.c
cp ../../src/cmd_se.c app/src/main/cpp/cmd_se.c
cp ../../src/cmd_set.c app/src/main/cpp/cmd_set.c
cp ../../src/cmd_setconfig.c app/src/main/cpp/cmd_setconfig.c
cp ../../src/cmd_setsave.c app/src/main/cpp/cmd_setsave.c
cp ../../src/cmd_shake.c app/src/main/cpp/cmd_shake.c
cp ../../src/cmd_skip.c app/src/main/cpp/cmd_skip.c
cp ../../src/cmd_switch.c app/src/main/cpp/cmd_switch.c
cp ../../src/cmd_video.c app/src/main/cpp/cmd_video.c
cp ../../src/cmd_vol.c app/src/main/cpp/cmd_vol.c
cp ../../src/cmd_wait.c app/src/main/cpp/cmd_wait.c
cp ../../src/cmd_wms.c app/src/main/cpp/cmd_wms.c
cp ../../src/conf.c app/src/main/cpp/conf.c
cp ../../src/conf.h app/src/main/cpp/conf.h
cp ../../src/drawglyph.h app/src/main/cpp/drawglyph.h
cp ../../src/drawimage.h app/src/main/cpp/drawimage.h
cp ../../src/event.c app/src/main/cpp/event.c
cp ../../src/event.h app/src/main/cpp/event.h
cp ../../src/file.h app/src/main/cpp/file.h
cp ../../src/glrender.c app/src/main/cpp/glrender.c
cp ../../src/glrender.h app/src/main/cpp/glrender.h
cp ../../src/glyph.c app/src/main/cpp/glyph.c
cp ../../src/glyph.h app/src/main/cpp/glyph.h
cp ../../src/gui.c app/src/main/cpp/gui.c
cp ../../src/gui.h app/src/main/cpp/gui.h
cp ../../src/history.c app/src/main/cpp/history.c
cp ../../src/history.h app/src/main/cpp/history.h
cp ../../src/image.c app/src/main/cpp/image.c
cp ../../src/image.h app/src/main/cpp/image.h
cp ../../src/log.c app/src/main/cpp/log.c
cp ../../src/log.h app/src/main/cpp/log.h
cp ../../src/main.c app/src/main/cpp/main.c
cp ../../src/main.h app/src/main/cpp/main.h
cp ../../src/mixer.c app/src/main/cpp/mixer.c
cp ../../src/mixer.h app/src/main/cpp/mixer.h
cp ../../src/ndkfile.c app/src/main/cpp/ndkfile.c
cp ../../src/ndkmain.c app/src/main/cpp/ndkmain.c
cp ../../src/ndkmain.h app/src/main/cpp/ndkmain.h
cp ../../src/ndkwave.c app/src/main/cpp/ndkwave.c
cp ../../src/platform.h app/src/main/cpp/platform.h
cp ../../src/readimage.c app/src/main/cpp/readimage.c
cp ../../src/readjpeg.c app/src/main/cpp/readjpeg.c
cp ../../src/readwebp.c app/src/main/cpp/readwebp.c
cp ../../src/save.c app/src/main/cpp/save.c
cp ../../src/save.h app/src/main/cpp/save.h
cp ../../src/scbuf.c app/src/main/cpp/scbuf.c
cp ../../src/scbuf.h app/src/main/cpp/scbuf.h
cp ../../src/script.c app/src/main/cpp/script.c
cp ../../src/script.h app/src/main/cpp/script.h
cp ../../src/seen.c app/src/main/cpp/seen.c
cp ../../src/seen.h app/src/main/cpp/seen.h
cp ../../src/stage.c app/src/main/cpp/stage.c
cp ../../src/stage.h app/src/main/cpp/stage.h
cp ../../src/suika.h app/src/main/cpp/suika.h
cp ../../src/types.h app/src/main/cpp/types.h
cp ../../src/vars.c app/src/main/cpp/vars.c
cp ../../src/vars.h app/src/main/cpp/vars.h
cp ../../src/wave.h app/src/main/cpp/wave.h
cp ../../src/wms_core.c app/src/main/cpp/wms_core.c
cp ../../src/wms_core.h app/src/main/cpp/wms_core.h
cp ../../src/wms.h app/src/main/cpp/wms.h
cp ../../src/wms_impl.c app/src/main/cpp/wms_impl.c
cp ../../src/wms_lexer.yy.c app/src/main/cpp/wms_lexer.yy.c
cp ../../src/wms_parser.tab.c app/src/main/cpp/wms_parser.tab.c
cp ../../src/wms_parser.tab.h app/src/main/cpp/wms_parser.tab.h

# Copy the game assets for our testing builds.
# You can comment this out when your game is ready.
mkdir -p app/src/main/assets
cp -R ../../game/* app/src/main/assets/

# Please modify and use this line for your game.
#mkdir app/src/main/assets
#cp ../../data01.arc app/src/main/assets/
