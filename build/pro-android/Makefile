setup:
	@echo 'Removing temporary files.'
	@rm -rf .idea .gradle app/build app/.cxx app/src/main/cpp app/src/main/assets
	@
	@echo 'Creating a directory.'
	@mkdir -p app/src/main/cpp
	@
	@echo 'Extracting libpng.'
	@mkdir -p app/src/main/cpp/libpng
	@tar xzf ../libsrc/libpng-1.6.35.tar.gz -C app/src/main/cpp/libpng --strip-components 1
	@cp app/src/main/cpp/libpng/scripts/pnglibconf.h.prebuilt app/src/main/cpp/libpng/pnglibconf.h
	@
	@echo 'Extracting bzip2.'
	@mkdir -p app/src/main/cpp/bzip2
	@tar xzf ../libsrc/bzip2-1.0.6.tar.gz -C app/src/main/cpp/bzip2 --strip-components 1
	@cp cmake/bzip2.txt app/src/main/cpp/bzip2/CMakeLists.txt
	@
	@echo 'Extracting libwebp.'
	@mkdir -p app/src/main/cpp/libwebp
	@tar xzf ../libsrc/libwebp-1.3.2.tar.gz -C app/src/main/cpp/libwebp --strip-components 1
	@
	@echo 'Extracting freetype2.'
	@mkdir -p app/src/main/cpp/freetype
	@tar xzf ../libsrc/freetype-2.13.2.tar.gz -C app/src/main/cpp/freetype --strip-components 1
	@
	@echo 'Extracting brotli.'
	@mkdir -p app/src/main/cpp/brotli
	@tar xzf ../libsrc/brotli-1.1.0.tar.gz -C app/src/main/cpp/brotli --strip-components 1
	@
	@echo 'Extracting jpeg9.'
	@mkdir -p app/src/main/cpp/jpeg
	@tar xzf ../libsrc/jpegsrc.v9e.tar.gz -C app/src/main/cpp/jpeg --strip-components 1
	@cp cmake/jpeg.txt app/src/main/cpp/jpeg/CMakeLists.txt
	@cp app/src/main/cpp/jpeg/jconfig.txt app/src/main/cpp/jpeg/jconfig.h
	@
	@echo 'Extracting libogg.'
	@mkdir -p app/src/main/cpp/libogg
	@tar xzf ../libsrc/libogg-1.3.3.tar.gz -C app/src/main/cpp/libogg --strip-components 1
	@cp cmake/libogg.txt app/src/main/cpp/libogg/CMakeLists.txt
	@sed -e 's/@INCLUDE_INTTYPES_H@/1/g' -e 's/@INCLUDE_STDINT_H@/1/g' -e 's/@INCLUDE_SYS_TYPES_H@/1/g' -e 's/@SIZE16@/int16_t/g' -e 's/@USIZE16@/uint16_t/g' -e 's/@SIZE32@/int32_t/g' -e 's/@USIZE32@/uint32_t/g' -e 's/@SIZE64@/int64_t/g' -e 's/@USIZE64@/uint64_t/g' < app/src/main/cpp/libogg/include/ogg/config_types.h.in > app/src/main/cpp/libogg/include/ogg/config_types.h
	@
	@echo 'Extracting libvorbis.'
	@mkdir -p app/src/main/cpp/libvorbis
	@tar xzf ../libsrc/libvorbis-1.3.6.tar.gz -C app/src/main/cpp/libvorbis --strip-components 1
	@cp cmake/libvorbis.txt app/src/main/cpp/libvorbis/CMakeLists.txt
	@
	@echo 'Copying source files.'
	@cp ../../src/google/ndkpro.c app/src/main/cpp/
	@cp ../../src/khronos/glrender.h app/src/main/cpp/
	@cp ../../src/khronos/glrender.c app/src/main/cpp/
	@cp ../../src/khronos/slsound.h app/src/main/cpp/
	@cp ../../src/khronos/slsound.c app/src/main/cpp/
	@cp ../../src/anime.h app/src/main/cpp/
	@cp ../../src/anime.c app/src/main/cpp/
	@cp ../../src/conf.c app/src/main/cpp/
	@cp ../../src/conf.h app/src/main/cpp/
	@cp ../../src/event.c app/src/main/cpp/
	@cp ../../src/event.h app/src/main/cpp/
	@cp ../../src/file.h app/src/main/cpp/
	@cp ../../src/file.c app/src/main/cpp/
	@cp ../../src/glyph.c app/src/main/cpp/
	@cp ../../src/glyph.h app/src/main/cpp/
	@cp ../../src/gui.c app/src/main/cpp/
	@cp ../../src/gui.h app/src/main/cpp/
	@cp ../../src/hal.h app/src/main/cpp/
	@cp ../../src/history.c app/src/main/cpp/
	@cp ../../src/history.h app/src/main/cpp/
	@cp ../../src/image.c app/src/main/cpp/
	@cp ../../src/image.h app/src/main/cpp/
	@cp ../../src/key.h app/src/main/cpp/
	@cp ../../src/log.c app/src/main/cpp/
	@cp ../../src/log.h app/src/main/cpp/
	@cp ../../src/main.c app/src/main/cpp/
	@cp ../../src/main.h app/src/main/cpp/
	@cp ../../src/mixer.c app/src/main/cpp/
	@cp ../../src/mixer.h app/src/main/cpp/
	@cp ../../src/motion.h app/src/main/cpp/
	@cp ../../src/package.c app/src/main/cpp/
	@cp ../../src/package.h app/src/main/cpp/
	@cp ../../src/pro.h app/src/main/cpp/
	@cp ../../src/readimage.c app/src/main/cpp/
	@cp ../../src/readpng.c app/src/main/cpp/
	@cp ../../src/readjpeg.c app/src/main/cpp/
	@cp ../../src/readwebp.c app/src/main/cpp/
	@cp ../../src/save.c app/src/main/cpp/
	@cp ../../src/save.h app/src/main/cpp/
	@cp ../../src/scbuf.c app/src/main/cpp/
	@cp ../../src/scbuf.h app/src/main/cpp/
	@cp ../../src/script.c app/src/main/cpp/
	@cp ../../src/script.h app/src/main/cpp/
	@cp ../../src/seen.c app/src/main/cpp/
	@cp ../../src/seen.h app/src/main/cpp/
	@cp ../../src/stage.c app/src/main/cpp/
	@cp ../../src/stage.h app/src/main/cpp/
	@cp ../../src/suika.h app/src/main/cpp/
	@cp ../../src/uimsg.h app/src/main/cpp/
	@cp ../../src/uimsg.c app/src/main/cpp/
	@cp ../../src/types.h app/src/main/cpp/
	@cp ../../src/vars.h app/src/main/cpp/
	@cp ../../src/vars.c app/src/main/cpp/
	@cp ../../src/wave.h app/src/main/cpp/
	@cp ../../src/wave.c app/src/main/cpp/
	@cp ../../src/wms_core.c app/src/main/cpp/
	@cp ../../src/wms_core.h app/src/main/cpp/
	@cp ../../src/wms.h app/src/main/cpp/
	@cp ../../src/wms_impl.c app/src/main/cpp/
	@cp ../../src/wms_lexer.yy.c app/src/main/cpp/
	@cp ../../src/wms_parser.tab.c app/src/main/cpp/
	@cp ../../src/wms_parser.tab.h app/src/main/cpp/
	@cp ../../src/cmd_anime.c app/src/main/cpp/
	@cp ../../src/cmd_bg.c app/src/main/cpp/
	@cp ../../src/cmd_bgm.c app/src/main/cpp/
	@cp ../../src/cmd_cha.c app/src/main/cpp/
	@cp ../../src/cmd_chapter.c app/src/main/cpp/
	@cp ../../src/cmd_ch.c app/src/main/cpp/
	@cp ../../src/cmd_chs.c app/src/main/cpp/
	@cp ../../src/cmd_click.c app/src/main/cpp/
	@cp ../../src/cmd_gosub.c app/src/main/cpp/
	@cp ../../src/cmd_goto.c app/src/main/cpp/
	@cp ../../src/cmd_gui.c app/src/main/cpp/
	@cp ../../src/cmd_if.c app/src/main/cpp/
	@cp ../../src/cmd_layer.c app/src/main/cpp/
	@cp ../../src/cmd_load.c app/src/main/cpp/
	@cp ../../src/cmd_message.c app/src/main/cpp/
	@cp ../../src/cmd_pencil.c app/src/main/cpp/
	@cp ../../src/cmd_return.c app/src/main/cpp/
	@cp ../../src/cmd_se.c app/src/main/cpp/
	@cp ../../src/cmd_set.c app/src/main/cpp/
	@cp ../../src/cmd_setconfig.c app/src/main/cpp/
	@cp ../../src/cmd_setsave.c app/src/main/cpp/
	@cp ../../src/cmd_shake.c app/src/main/cpp/
	@cp ../../src/cmd_skip.c app/src/main/cpp/
	@cp ../../src/cmd_switch.c app/src/main/cpp/
	@cp ../../src/cmd_video.c app/src/main/cpp/
	@cp ../../src/cmd_vol.c app/src/main/cpp/
	@cp ../../src/cmd_wait.c app/src/main/cpp/
	@cp ../../src/cmd_wms.c app/src/main/cpp/
	@
	@echo 'Create a template game zip.'
	@mkdir -p app/src/main/assets
	@cd ../../games/japanese && zip -r ../../build/pro-android/app/src/main/assets/game.zip *
