#
# Toolchain selection
#

CC = gcc

#
# CPPFLAGS
#

CPPFLAGS = \
	-DUSE_X11_OPENGL \
	-DUSE_CAPTURE \
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
	-lm \
	-lpthread \
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
	../../src/glrender.c \
	../../src/x11main.c \
	../../src/capture.c

#
# .c.o compilation rules
#

OBJS = $(SRCS:../../src/%.c=%.o) \

%.o: ../../src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

#
# Target
#

all: suika-capture

suika-capture: libroot $(OBJS)
	$(CC) -o suika-capture $(OBJS) $(LDFLAGS)

libroot:
	./build-libs.sh

#
# Feature specific source files.
#

include ../sse.mk

#
# Phony
#

install: suika-capture
	cp suika-capture ../../suika-linux-capture

clean:
	rm -rf *~ *.o suika-capture log.txt sav tmp

erase:
	rm -rf *~ *.o suika-capture log.txt sav tmp libroot
