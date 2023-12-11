include ../common.mk

#
# Toolchain selection
#

CC = gcc

#
# CPPFLAGS
#

CPPFLAGS = \
	-I./libroot/include \
	-I./libroot/include/freetype2

#
# CFLAGS
#

CFLAGS = \
	-O0 -g3 \
	-ffast-math \
	-ftree-vectorize \
	-std=gnu11 \
	-Wall \
	-Werror \
	-Wextra \
	-Wundef \
	-Wconversion \
	-Wno-multichar

#
# LDFLAGS
#

LDFLAGS = \
	-L./libroot/lib \
	-Wl,-dn,-lpng16,-ljpeg,-lwebp,-lfreetype,-lvorbisfile,-lvorbis,-logg,-lz,-lbz2,-dy \
	-Wl,--gc-sections \
	-lpthread \
	-lasound \
	-lX11 \
	-lXpm \
	-lGL \
	-lGLX \
	-lm

#
# Source files
#

SRCS = \
	$(SRCS_COMMON) \
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

all: suika

suika: libroot $(OBJS)
	$(CC) -o suika $(OBJS) $(LDFLAGS) `pkg-config --libs gstreamer-1.0 gstreamer-video-1.0`

libroot:
	./build-libs.sh

#
# Phony
#

install: suika
	cp suika ../../suika-linux

clean:
	rm -rf *~ *.o suika log.txt sav tmp

erase:
	rm -rf *~ *.o suika log.txt sav tmp libroot

lines:
	cat ../../src/*.[chm] | wc -l
	grep -E '/\*|\*/|^([ \t]+\*)' ../../src/*.[cm] | wc -l
	grep 'assert' ../../src/*.[chm] | wc -l

analyze:
	$(CC) $(CPPFLAGS) -fanalyzer -finput-charset=utf-8 `pkg-config --cflags gstreamer-1.0 gstreamer-video-1.0` -c $(SRCS)

valgrind:
	cp suika ../../game/demo
	cd ../../game/demo && valgrind --leak-check=full ./suika
	rm ../../game/demo/suika
