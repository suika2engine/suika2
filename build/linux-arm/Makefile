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
	-mfpu=neon \
	-march=armv7-a \
	-O3 \
	-ffast-math \
	-ftree-vectorize \
	-fopt-info-vec \
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
	-Wl,-dn,-lpng16,-lz,-lvorbisfile,-lvorbis,-logg,-lfreetype,-dy

#
# Source files
#

include ../common.mk

SRCS = \
	$(SRCS_COMMON) \
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

suika: $(OBJS)
	$(CC) -o suika $(OBJS) $(LDFLAGS)

#
# Phony
#

install: suika
	cp suika ../../

clean:
	rm -rf *~ *.o suika log.txt sav tmp libroot suika2 suika-2.x.x-linux64.zip

lines:
	cat ../../src/*.[chm] | wc -l
	grep -E '/\*|\*/|^([ \t]+\*)' ../../src/*.[cm] | wc -l
	grep 'assert' ../../src/*.[chm] | wc -l
