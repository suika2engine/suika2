#
# Toolchain selection
#

CC = clang

#
# CPPFLAGS
#

CPPFLAGS = \
	-DUSE_X11_OPENGL \
	-I./libroot/include \
	-I./libroot/include/freetype2

#
# CFLAGS
#

CFLAGS = \
	-O3 \
	-ffast-math \
	-ftree-vectorize \
	-std=gnu89 \
	-Wall \
	-Werror \
	-Wextra \
	-Wundef \
	-Wconversion

#
# LDFLAGS
#

LDFLAGS = \
	-lm \
	-lpthread \
	-lasound \
	-lX11 \
	-lXpm \
	-lGL \
	-lGLX \
	-L./libroot/lib \
	-Wl,-dn,-ljpeg,-lpng16,-lz,-lvorbisfile,-lvorbis,-logg,-lfreetype,-dy \
	-Wl,--gc-sections \
	-lmvec

#
# Source files
#

include ../common.mk

SRCS = \
	$(SRCS_COMMON) \
	$(SRCS_SSE) \
	../../src/asound.c \
	../../src/glrender.c \
	../../src/gstplay.c \
	../../src/x11main.c

#
# .c.o compilation rules
#

OBJS = $(SRCS:../../src/%.c=%.o) \

%.o: ../../src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) `pkg-config --cflags gstreamer-1.0 gstreamer-video-1.0` $<

#
# Target
#

suika: $(OBJS)
	$(CC) -o suika $(OBJS) $(LDFLAGS) `pkg-config --libs gstreamer-1.0 gstreamer-video-1.0`

#
# Feature specific source files.
#

include ../sse.mk

#
# Phony
#

install: suika
	cp suika ../../

clean:
	rm -rf *~ *.o suika log.txt sav tmp

lines:
	cat ../../src/*.[chm] | wc -l
	grep -E '/\*|\*/|^([ \t]+\*)' ../../src/*.[cm] | wc -l
	grep 'assert' ../../src/*.[chm] | wc -l

analyze:
	$(CC) $(CPPFLAGS) --analyze -finput-charset=utf-8 `pkg-config --cflags gstreamer-1.0 gstreamer-video-1.0` -c $(SRCS)

valgrind:
	cp suika ../../game-en/
	cd ../../game-en && valgrind --leak-check=full ./suika
