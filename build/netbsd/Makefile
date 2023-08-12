#
# Toolchain selection
#

CC = gcc

#
# CPPFLAGS
#  - To use OpenGL, please add '-DUSE_X11_OPENGL' to CPPFLAGS.
#

CPPFLAGS = \
	-I./libroot/include \
	-I./libroot/include/freetype2 \
	-I/usr/X11R7/include \
	-I/usr/pkg/include

#
# CFLAGS
#

CFLAGS = \
	-O3 \
	-ffast-math \
	-ftree-vectorize \
	-fopt-info-vec \
	-std=gnu11 \
	-Wall \
	-Werror \
	-Wextra \
	-Wundef \
	-Wconversion \
	-Wno-multichar

#
# LDFLAGS
#  - To use OpenGL, please add '-lGL' to 'LDFLAGS'.
#

LDFLAGS = \
	-lm \
	-lpthread \
	-L/usr/pkg/lib \
	-lasound \
	-L/usr/X11R7/lib \
	-lX11 \
	-lXpm \
	-L./libroot/lib \
	-Wl,-dn,-ljpeg,-lpng16,-lz,-lvorbisfile,-lvorbis,-logg,-lfreetype,-dy \
	-Wl,--gc-sections

#
# Source files
#  - To use OpenGL, add '../../src/glrender.c' to 'SRCS'.
#

include ../common.mk

SRCS = \
	$(SRCS_COMMON) \
	$(SRCS_SSE) \
	../../src/asound.c \
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
