#
# Toolchain selection
#

CC = gcc

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
	-O0 -g3 \
	-ffast-math \
	-ftree-vectorize \
	-fopt-info-vec \
	-std=gnu11 \
	-Wall \
	-Werror \
	-Wextra \
	-Wundef

#
# LDFLAGS
#

LDFLAGS = \
	-lm \
	-lmvec \
	-lpthread \
	-lasound \
	-lX11 \
	-lXpm \
	-lGL \
	-lGLX \
	-L./libroot/lib \
	-Wl,-dn,-lpng16,-ljpeg,-lwebp,-lfreetype,-lvorbisfile,-lvorbis,-logg,-lz,-lbz2,-dy \
	-Wl,--gc-sections

#
# Source files
#

include ../common.mk

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

suika: libroot $(OBJS)
	$(CC) -o suika $(OBJS) $(LDFLAGS) `pkg-config --libs gstreamer-1.0 gstreamer-video-1.0`

libroot:
	./build-libs.sh

#
# Phony
#

install: suika
	cp suika ../../

clean:
	rm -rf *~ *.o suika log.txt sav tmp libroot
