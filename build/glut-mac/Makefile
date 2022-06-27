#
# Toolchain selection
#

CC = clang

#
# CPPFLAGS
#

CPPFLAGS = \
	-DUSE_OPENGL \
	-I./libroot/include \
	-I./libroot/include/freetype2

#
# CFLAGS
#

CFLAGS = \
	-O0 -g3 \
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
	-framework OpenGL \
	-framework GLUT \
	-framework AudioUnit \
	-lpthread \
	-L./libroot/lib \
	-lpng16 \
	-lz \
	-lvorbisfile \
	-lvorbis \
	-logg \
	-lfreetype

#
# Source files
#

include ../common.mk

SRCS = \
	$(SRCS_COMMON) \
	../../src/aunit.c \
	../../src/glesrender.c \
	../../src/glutmain.c

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
# Feature specific source files.
#

include ../sse.mk

#
# Phony
#

install: suika
	cp suika ../../

clean:
	rm -rf *~ *.o suika log.txt sav tmp libroot
