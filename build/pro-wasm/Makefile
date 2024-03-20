SHELL=/bin/bash

SRC = \
	../../src/wasm/empro.c \
	../../src/wasm/emfile.c \
	../../src/wasm/alsound.c \
	../../src/khronos/glrender.c \
	../../src/anime.c \
	../../src/conf.c \
	../../src/ciel.c \
	../../src/event.c \
	../../src/glyph.c \
	../../src/gui.c \
	../../src/history.c \
	../../src/image.c \
	../../src/log.c \
	../../src/main.c \
	../../src/mixer.c \
	../../src/readimage.c \
	../../src/readpng.c \
	../../src/readjpeg.c \
	../../src/save.c \
	../../src/scbuf.c \
	../../src/script.c \
	../../src/seen.c \
	../../src/stage.c \
	../../src/vars.c \
	../../src/wave.c \
	../../src/wms_core.c \
	../../src/wms_impl.c \
	../../src/wms_lexer.yy.c \
	../../src/wms_parser.tab.c \
	../../src/cmd_anime.c \
	../../src/cmd_bg.c \
	../../src/cmd_bgm.c \
	../../src/cmd_ch.c \
	../../src/cmd_cha.c \
	../../src/cmd_chapter.c \
	../../src/cmd_chs.c \
	../../src/cmd_click.c \
	../../src/cmd_gosub.c \
	../../src/cmd_goto.c \
	../../src/cmd_gui.c \
	../../src/cmd_if.c \
	../../src/cmd_layer.c \
	../../src/cmd_load.c \
	../../src/cmd_message.c \
	../../src/cmd_pencil.c \
	../../src/cmd_return.c \
	../../src/cmd_se.c \
	../../src/cmd_set.c \
	../../src/cmd_setconfig.c \
	../../src/cmd_setsave.c \
	../../src/cmd_shake.c \
	../../src/cmd_skip.c \
	../../src/cmd_switch.c \
	../../src/cmd_video.c \
	../../src/cmd_vol.c \
	../../src/cmd_wait.c \
	../../src/cmd_wms.c \

all: html

html: emsdk $(SRC) shell.html
	EMSDK_QUIET=1 source ./emsdk/emsdk_env.sh && \
	rm -f html/index.* && \
	mkdir -p html && \
	emcc \
		$(SRC) \
		-o html/index.html \
		-O3 \
		-DUSE_OPENGL \
		-DUSE_EDITOR \
		-DNO_WEBP \
		-s USE_ZLIB=1 \
		-s USE_LIBPNG=1 \
		-s USE_OGG=1 \
		-s USE_VORBIS=1 \
		-s USE_FREETYPE=1 \
		-s USE_LIBJPEG=1 \
		-s TOTAL_MEMORY=536870912 \
		-s NO_EXIT_RUNTIME=1 \
		-s EXPORTED_FUNCTIONS=["_onLoadProject","_onClickContinue","_onClickNext","_onClickStop","_setVisible","_setHidden","_malloc","_onEditorRangeChange","_onEditorCtrlReturn"] \
		-s EXPORTED_RUNTIME_METHODS=["ccall","UTF8ToString","writeArrayToMemory"] \
		-s ASYNCIFY \
		-lz \
		-lpng \
		-logg \
		-lvorbis \
		-lfreetype \
		-lopenal \
		--pre-js pre.js \
		--shell-file shell.html \
		--use-preload-plugins

emsdk:
	git clone https://github.com/emscripten-core/emsdk.git
	cd emsdk && ./emsdk install latest && cd ..
	cd emsdk && ./emsdk activate latest && cd ..

upload:
	ftp-upload.sh html/index.html vls/index.html
	ftp-upload.sh html/index.js vls/index.js
	ftp-upload.sh html/index.wasm vls/index.wasm

clean:
	rm -rf html
