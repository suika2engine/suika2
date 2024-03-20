#
# Common source and header files that are referred from Makefile-based build directories.
#

# Headers for Suika2 Engine
HDRS_MAIN = \
	../../src/suika.h \
	../../src/anime.h \
	../../src/conf.h \
	../../src/event.h \
	../../src/file.h \
	../../src/glyph.h \
	../../src/gui.h \
	../../src/history.h \
	../../src/image.h \
	../../src/log.h \
	../../src/main.h \
	../../src/mixer.h \
	../../src/motion.h \
	../../src/save.h \
	../../src/scbuf.h \
	../../src/script.h \
	../../src/seen.h \
	../../src/stage.h \
	../../src/uimsg.h \
	../../src/vars.h \
	../../src/wave.h \
	../../src/wms.h

# Sources for Suika2 Engine
SRCS_MAIN = \
	../../src/anime.c \
	../../src/conf.c \
	../../src/ciel.c \
	../../src/event.c \
	../../src/file.c \
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
	../../src/readwebp.c \
	../../src/save.c \
	../../src/scbuf.c \
	../../src/script.c \
	../../src/seen.c \
	../../src/stage.c \
	../../src/uimsg.c \
	../../src/vars.c \
	../../src/wave.c \
	../../src/wms_core.c \
	../../src/wms_lexer.yy.c \
	../../src/wms_parser.tab.c \
	../../src/wms_impl.c \
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
	../../src/cmd_wms.c

# Headers for Suika2 Pro
HDRS_PRO = \
	../../src/pro.h \
	../../src/package.h

# Source for Suika2 Pro
SRCS_PRO = \
	../../src/package.c
