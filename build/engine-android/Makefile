all:
	@echo 'Usage:'
	@echo '  make debug'
	@echo '  make src'

debug:
	@echo 'Making a source code tree for a build test.'
	@rm -rf .idea .gradle app/build app/.cxx app/src/main/cpp app/src/main/assets
	@mkdir -p app/src/main/cpp
	@
	@cp ../../src/google/ndkmain.h app/src/main/cpp/
	@cp ../../src/google/ndkmain.c app/src/main/cpp/
	@cp ../../src/google/ndkfile.c app/src/main/cpp/
	@cp ../../src/khronos/glrender.h app/src/main/cpp/
	@cp ../../src/khronos/glrender.c app/src/main/cpp/
	@cp ../../src/khronos/slsound.h app/src/main/cpp/
	@cp ../../src/khronos/slsound.c app/src/main/cpp/
	@cp ../../src/suika.h app/src/main/cpp/
	@cp ../../src/anime.h app/src/main/cpp/
	@cp ../../src/anime.c app/src/main/cpp/
	@cp ../../src/conf.h app/src/main/cpp/
	@cp ../../src/conf.c app/src/main/cpp/
	@cp ../../src/ciel.c app/src/main/cpp/
	@cp ../../src/event.h app/src/main/cpp/
	@cp ../../src/event.c app/src/main/cpp/
	@cp ../../src/file.h app/src/main/cpp/
	@cp ../../src/glyph.h app/src/main/cpp/
	@cp ../../src/glyph.c app/src/main/cpp/
	@cp ../../src/gui.h app/src/main/cpp/
	@cp ../../src/gui.c app/src/main/cpp/
	@cp ../../src/hal.h app/src/main/cpp/
	@cp ../../src/history.h app/src/main/cpp/
	@cp ../../src/history.c app/src/main/cpp/
	@cp ../../src/image.h app/src/main/cpp/
	@cp ../../src/image.c app/src/main/cpp/
	@cp ../../src/log.h app/src/main/cpp/
	@cp ../../src/log.c app/src/main/cpp/
	@cp ../../src/main.h app/src/main/cpp/
	@cp ../../src/main.c app/src/main/cpp/
	@cp ../../src/mixer.h app/src/main/cpp/
	@cp ../../src/mixer.c app/src/main/cpp/
	@cp ../../src/motion.h app/src/main/cpp/
	@cp ../../src/readimage.c app/src/main/cpp/
	@cp ../../src/readpng.c app/src/main/cpp/
	@cp ../../src/readjpeg.c app/src/main/cpp/
	@cp ../../src/readwebp.c app/src/main/cpp/
	@cp ../../src/save.h app/src/main/cpp/
	@cp ../../src/save.c app/src/main/cpp/
	@cp ../../src/scbuf.h app/src/main/cpp/
	@cp ../../src/scbuf.c app/src/main/cpp/
	@cp ../../src/script.h app/src/main/cpp/
	@cp ../../src/script.c app/src/main/cpp/
	@cp ../../src/seen.h app/src/main/cpp/
	@cp ../../src/seen.c app/src/main/cpp/
	@cp ../../src/stage.h app/src/main/cpp/
	@cp ../../src/stage.c app/src/main/cpp/
	@cp ../../src/uimsg.h app/src/main/cpp/
	@cp ../../src/uimsg.c app/src/main/cpp/
	@cp ../../src/types.h app/src/main/cpp/
	@cp ../../src/uimsg.h app/src/main/cpp/
	@cp ../../src/uimsg.c app/src/main/cpp/
	@cp ../../src/vars.h app/src/main/cpp/
	@cp ../../src/vars.c app/src/main/cpp/
	@cp ../../src/wave.h app/src/main/cpp/
	@cp ../../src/wave.c app/src/main/cpp/
	@cp ../../src/wms.h app/src/main/cpp/
	@cp ../../src/wms_core.h app/src/main/cpp/
	@cp ../../src/wms_core.c app/src/main/cpp/
	@cp ../../src/wms_impl.c app/src/main/cpp/
	@cp ../../src/wms_lexer.yy.c app/src/main/cpp/
	@cp ../../src/wms_parser.tab.h app/src/main/cpp/
	@cp ../../src/wms_parser.tab.c app/src/main/cpp/
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
	@mkdir -p app/src/main/cpp/libpng
	@tar xzf ../libsrc/libpng-1.6.35.tar.gz -C app/src/main/cpp/libpng --strip-components 1
	@cp app/src/main/cpp/libpng/scripts/pnglibconf.h.prebuilt app/src/main/cpp/libpng/pnglibconf.h
	@
	@mkdir -p app/src/main/cpp/jpeg
	@tar xzf ../libsrc/jpegsrc.v9e.tar.gz -C app/src/main/cpp/jpeg --strip-components 1
	@cp cmake/jpeg.txt app/src/main/cpp/jpeg/CMakeLists.txt
	@cp app/src/main/cpp/jpeg/jconfig.txt app/src/main/cpp/jpeg/jconfig.h
	@
	@mkdir -p app/src/main/cpp/libwebp
	@tar xzf ../libsrc/libwebp-1.3.2.tar.gz -C app/src/main/cpp/libwebp --strip-components 1
	@
	@mkdir -p app/src/main/cpp/bzip2
	@tar xzf ../libsrc/bzip2-1.0.6.tar.gz -C app/src/main/cpp/bzip2 --strip-components 1
	@cp cmake/bzip2.txt app/src/main/cpp/bzip2/CMakeLists.txt
	@
	@mkdir -p app/src/main/cpp/libogg
	@tar xzf ../libsrc/libogg-1.3.3.tar.gz -C app/src/main/cpp/libogg --strip-components 1
	@cp cmake/libogg.txt app/src/main/cpp/libogg/CMakeLists.txt
	@sed -e 's/@INCLUDE_INTTYPES_H@/1/g' -e 's/@INCLUDE_STDINT_H@/1/g' -e 's/@INCLUDE_SYS_TYPES_H@/1/g' -e 's/@SIZE16@/int16_t/g' -e 's/@USIZE16@/uint16_t/g' -e 's/@SIZE32@/int32_t/g' -e 's/@USIZE32@/uint32_t/g' -e 's/@SIZE64@/int64_t/g' -e 's/@USIZE64@/uint64_t/g' < app/src/main/cpp/libogg/include/ogg/config_types.h.in > app/src/main/cpp/libogg/include/ogg/config_types.h
	@
	@mkdir -p app/src/main/cpp/libvorbis
	@tar xzf ../libsrc/libvorbis-1.3.6.tar.gz -C app/src/main/cpp/libvorbis --strip-components 1
	@cp cmake/libvorbis.txt app/src/main/cpp/libvorbis/CMakeLists.txt
	@
	@mkdir -p app/src/main/cpp/freetype
	@tar xzf ../libsrc/freetype-2.13.2.tar.gz -C app/src/main/cpp/freetype --strip-components 1
	@
	@mkdir -p app/src/main/cpp/brotli
	@tar xzf ../libsrc/brotli-1.1.0.tar.gz -C app/src/main/cpp/brotli --strip-components 1
	@
	@mkdir -p app/src/main/assets
	@cp -R ../../games/demo/* app/src/main/assets/

src:
	@echo 'Making a source code tree for the export feature.'
	@rm -rf app/build app/.cxx app/src/main/cpp app/src/main/assets android-src
	@mkdir -p android-src
	@mkdir -p android-src/app/src/main/cpp
	@
	@cp -R app android-src/
	@cp -R gradle.properties android-src/
	@cp -R build.gradle android-src/
	@cp -R gradlew android-src/
	@cp -R settings.gradle android-src/
	@cp -R gradlew.bat android-src/
	@cp -R gradle android-src/
	@
	@cp ../../src/google/ndkmain.h android-src/app/src/main/cpp/
	@cp ../../src/google/ndkmain.c android-src/app/src/main/cpp/
	@cp ../../src/google/ndkfile.c android-src/app/src/main/cpp/
	@cp ../../src/khronos/glrender.h android-src/app/src/main/cpp/
	@cp ../../src/khronos/glrender.c android-src/app/src/main/cpp/
	@cp ../../src/khronos/slsound.h android-src/app/src/main/cpp/
	@cp ../../src/khronos/slsound.c android-src/app/src/main/cpp/
	@cp ../../src/suika.h android-src/app/src/main/cpp/
	@cp ../../src/anime.h android-src/app/src/main/cpp/
	@cp ../../src/anime.c android-src/app/src/main/cpp/
	@cp ../../src/conf.h android-src/app/src/main/cpp/
	@cp ../../src/conf.c android-src/app/src/main/cpp/
	@cp ../../src/ciel.c android-src/app/src/main/cpp/
	@cp ../../src/event.h android-src/app/src/main/cpp/
	@cp ../../src/event.c android-src/app/src/main/cpp/
	@cp ../../src/file.h android-src/app/src/main/cpp/
	@cp ../../src/glyph.h android-src/app/src/main/cpp/
	@cp ../../src/glyph.c android-src/app/src/main/cpp/
	@cp ../../src/gui.h android-src/app/src/main/cpp/
	@cp ../../src/gui.c android-src/app/src/main/cpp/
	@cp ../../src/hal.h android-src/app/src/main/cpp/
	@cp ../../src/history.h android-src/app/src/main/cpp/
	@cp ../../src/history.c android-src/app/src/main/cpp/
	@cp ../../src/image.h android-src/app/src/main/cpp/
	@cp ../../src/image.c android-src/app/src/main/cpp/
	@cp ../../src/log.h android-src/app/src/main/cpp/
	@cp ../../src/log.c android-src/app/src/main/cpp/
	@cp ../../src/main.h android-src/app/src/main/cpp/
	@cp ../../src/main.c android-src/app/src/main/cpp/
	@cp ../../src/mixer.h android-src/app/src/main/cpp/
	@cp ../../src/mixer.c android-src/app/src/main/cpp/
	@cp ../../src/motion.h android-src/app/src/main/cpp/
	@cp ../../src/readimage.c android-src/app/src/main/cpp/
	@cp ../../src/readpng.c android-src/app/src/main/cpp/
	@cp ../../src/readjpeg.c android-src/app/src/main/cpp/
	@cp ../../src/readwebp.c android-src/app/src/main/cpp/
	@cp ../../src/save.h android-src/app/src/main/cpp/
	@cp ../../src/save.c android-src/app/src/main/cpp/
	@cp ../../src/scbuf.h android-src/app/src/main/cpp/
	@cp ../../src/scbuf.c android-src/app/src/main/cpp/
	@cp ../../src/script.h android-src/app/src/main/cpp/
	@cp ../../src/script.c android-src/app/src/main/cpp/
	@cp ../../src/seen.h android-src/app/src/main/cpp/
	@cp ../../src/seen.c android-src/app/src/main/cpp/
	@cp ../../src/stage.h android-src/app/src/main/cpp/
	@cp ../../src/stage.c android-src/app/src/main/cpp/
	@cp ../../src/types.h android-src/app/src/main/cpp/
	@cp ../../src/uimsg.h android-src/app/src/main/cpp/
	@cp ../../src/uimsg.c android-src/app/src/main/cpp/
	@cp ../../src/vars.h android-src/app/src/main/cpp/
	@cp ../../src/vars.c android-src/app/src/main/cpp/
	@cp ../../src/wave.h android-src/app/src/main/cpp/
	@cp ../../src/wave.c android-src/app/src/main/cpp/
	@cp ../../src/wms.h android-src/app/src/main/cpp/
	@cp ../../src/wms_core.h android-src/app/src/main/cpp/
	@cp ../../src/wms_core.c android-src/app/src/main/cpp/
	@cp ../../src/wms_impl.c android-src/app/src/main/cpp/
	@cp ../../src/wms_lexer.yy.c android-src/app/src/main/cpp/
	@cp ../../src/wms_parser.tab.h android-src/app/src/main/cpp/
	@cp ../../src/wms_parser.tab.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_anime.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_bg.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_bgm.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_cha.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_chapter.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_ch.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_chs.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_click.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_gosub.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_goto.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_gui.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_if.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_layer.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_load.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_message.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_pencil.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_return.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_se.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_set.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_setconfig.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_setsave.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_shake.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_skip.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_switch.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_video.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_vol.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_wait.c android-src/app/src/main/cpp/
	@cp ../../src/cmd_wms.c android-src/app/src/main/cpp/
	@
	@mkdir -p android-src/app/src/main/cpp/libpng
	@tar xzf ../libsrc/libpng-1.6.35.tar.gz -C android-src/app/src/main/cpp/libpng --strip-components 1
	@cp android-src/app/src/main/cpp/libpng/scripts/pnglibconf.h.prebuilt android-src/app/src/main/cpp/libpng/pnglibconf.h
	@
	@mkdir -p android-src/app/src/main/cpp/jpeg
	@tar xzf ../libsrc/jpegsrc.v9e.tar.gz -C android-src/app/src/main/cpp/jpeg --strip-components 1
	@cp cmake/jpeg.txt android-src/app/src/main/cpp/jpeg/CMakeLists.txt
	@cp android-src/app/src/main/cpp/jpeg/jconfig.txt android-src/app/src/main/cpp/jpeg/jconfig.h
	@
	@mkdir -p android-src/app/src/main/cpp/bzip2
	@tar xzf ../libsrc/bzip2-1.0.6.tar.gz -C android-src/app/src/main/cpp/bzip2 --strip-components 1
	@cp cmake/bzip2.txt android-src/app/src/main/cpp/bzip2/CMakeLists.txt
	@
	@mkdir -p android-src/app/src/main/cpp/libwebp
	@tar xzf ../libsrc/libwebp-1.3.2.tar.gz -C android-src/app/src/main/cpp/libwebp --strip-components 1
	@
	@mkdir -p android-src/app/src/main/cpp/freetype
	@tar xzf ../libsrc/freetype-2.13.2.tar.gz -C android-src/app/src/main/cpp/freetype --strip-components 1
	@
	@mkdir -p android-src/app/src/main/cpp/brotli
	@tar xzf ../libsrc/brotli-1.1.0.tar.gz -C android-src/app/src/main/cpp/brotli --strip-components 1
	@
	@mkdir -p android-src/app/src/main/cpp/libogg
	@tar xzf ../libsrc/libogg-1.3.3.tar.gz -C android-src/app/src/main/cpp/libogg --strip-components 1
	@cp cmake/libogg.txt android-src/app/src/main/cpp/libogg/CMakeLists.txt
	@sed -e 's/@INCLUDE_INTTYPES_H@/1/g' -e 's/@INCLUDE_STDINT_H@/1/g' -e 's/@INCLUDE_SYS_TYPES_H@/1/g' -e 's/@SIZE16@/int16_t/g' -e 's/@USIZE16@/uint16_t/g' -e 's/@SIZE32@/int32_t/g' -e 's/@USIZE32@/uint32_t/g' -e 's/@SIZE64@/int64_t/g' -e 's/@USIZE64@/uint64_t/g' < android-src/app/src/main/cpp/libogg/include/ogg/config_types.h.in > android-src/app/src/main/cpp/libogg/include/ogg/config_types.h
	@rm -rf android-src/app/src/main/cpp/libogg/macosx
	@
	@mkdir -p android-src/app/src/main/cpp/libvorbis
	@tar xzf ../libsrc/libvorbis-1.3.6.tar.gz -C android-src/app/src/main/cpp/libvorbis --strip-components 1
	@cp cmake/libvorbis.txt android-src/app/src/main/cpp/libvorbis/CMakeLists.txt
	@rm -rf android-src/app/src/main/cpp/libvorbis/macosx
	@
	@mkdir -p android-src/app/src/main/assets
