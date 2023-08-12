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
	-I./libroot/include/freetype2 \
	-I/usr/local/include

#
# CFLAGS
#

CFLAGS = \
	-O3 \
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
	-lbz2 \
	-L/usr/local/lib \
	-lasound \
	-lX11 \
	-lXpm \
	-lGL \
	-L./libroot/lib \
	-Wl,-dn,-ljpeg,-lpng16,-lz,-lvorbisfile,-lvorbis,-logg,-lfreetype,-dy \
	-Wl,--gc-sections

#
# Source files
#

include ../common.mk

SRCS = \
	$(SRCS_COMMON) \
	$(SRCS_SSE) \
	../../src/asound.c \
	../../src/glrender.c \
	../../src/x11main.c

#
# .c.o compilation rules
#

OBJS = $(SRCS:../../src/%.c=%.o) \

%.o: ../../src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

#
# Target
#

suika: libroot $(OBJS)
	$(CC) -s -o suika $(OBJS) $(LDFLAGS)

libroot:
	./build-libs.sh

#
# Feature specific source files.
#  - To build on non-x86 platform, please comment out following line.
#

include ../sse.mk

#
# Phony
#

install: suika
	cp suika ../../

clean:
	rm -rf *~ *.o suika log.txt sav tmp libroot
