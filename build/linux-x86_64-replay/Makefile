#
# Toolchain selection
#

CC = gcc

#
# CPPFLAGS
#

CPPFLAGS = \
	-DUSE_X11_OPENGL \
	-DUSE_REPLAY \
	-I./libroot/include \
	-I./libroot/include/freetype2

#
# CFLAGS
#

CFLAGS = \
	-coverage \
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
	-s \
	-coverage \
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
	../../src/x11replay.c

#
# .c.o compilation rules
#

OBJS = $(SRCS:../../src/%.c=%.o) \

%.o: ../../src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

#
# Target
#

all: suika-replay

suika-replay: libroot $(OBJS)
	$(CC) -o suika-replay $(OBJS) $(LDFLAGS)

libroot:
	./build-libs.sh

#
# Feature specific source files.
#

include ../sse.mk

#
# Phony
#

install: suika-replay
	cp suika ../../suika-linux-replay

clean:
	rm -rf *~ *.o suika-replay log.txt sav tmp

erase:
	rm -rf *~ *.o suika-replay log.txt sav tmp libroot
