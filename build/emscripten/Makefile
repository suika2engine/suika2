SRC = \
	../../src/cmd_bg.c \
	../../src/cmd_bgm.c \
	../../src/cmd_ch.c \
	../../src/cmd_cha.c \
	../../src/cmd_click.c \
	../../src/cmd_gosub.c \
	../../src/cmd_goto.c \
	../../src/cmd_if.c \
	../../src/cmd_load.c \
	../../src/cmd_menu.c \
	../../src/cmd_message.c \
	../../src/cmd_retrospect.c \
	../../src/cmd_return.c \
	../../src/cmd_se.c \
	../../src/cmd_select.c \
	../../src/cmd_set.c \
	../../src/cmd_setsave.c \
	../../src/cmd_shake.c \
	../../src/cmd_switch.c \
	../../src/cmd_vol.c \
	../../src/cmd_wait.c \
	../../src/conf.c \
	../../src/emopenal.c \
	../../src/event.c \
	../../src/file.c \
	../../src/glyph.c \
	../../src/history.c \
	../../src/image.c \
	../../src/log.c \
	../../src/main.c \
	../../src/mixer.c \
	../../src/readimage.c \
	../../src/save.c \
	../../src/scbuf.c \
	../../src/script.c \
	../../src/stage.c \
	../../src/vars.c \
	../../src/wave.c \
	../../src/emmain.c

all:
	rm -f html/index.*
	mkdir -p html
	emcc $(SRC) -o html/index.html -s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_OGG=1 -s USE_VORBIS=1 -s USE_FREETYPE=1 -s TOTAL_MEMORY=536870912 -s EXPORTED_RUNTIME_METHODS=ccall -lz -lpng -logg -lvorbis -lfreetype -lopenal --shell-file shell.html --pre-js pre.js --use-preload-plugins

run:
	python -m SimpleHTTPServer

clean:
	rm -rf html
