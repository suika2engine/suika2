#
# Toolchain selection
#

CC = gcc

#
# CPPFLAGS
#

CPPFLAGS = \
	-DUSE_SDL2 \
	-DUSE_SDL2_OPENGL \
	-I./libroot/include \
	-I./libroot/include/freetype2

#
# CFLAGS
#

CFLAGS = \
	-O0 -g3 \
	-ffast-math \
	-ftree-vectorize \
<<<<<<< HEAD
	-std=gnu89 \
=======
	-fopt-info-vec \
	-std=gnu11 \
>>>>>>> 4939ed6 (Fixed compilation issues for target SDL2)
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
	-lGLEW \
	-lGL \
	-lpthread \
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
	../../src/gstplay.c \
	../../src/sdlmain.c

#
# .c.o compilation rules
#

OBJS = $(SRCS:../../src/%.c=%.o) \

%.o: ../../src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) `pkg-config --cflags sdl2 gstreamer-1.0 gstreamer-video-1.0` $<

#
# Target
#

suika: $(OBJS)
	$(CC) -o suika $(OBJS) $(LDFLAGS) `pkg-config --libs sdl2 gstreamer-1.0 gstreamer-video-1.0`

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
	$(CC) $(CPPFLAGS) -fanalyzer -finput-charset=utf-8 `pkg-config --cflags gstreamer-1.0 gstreamer-video-1.0` -c $(SRCS)

valgrind:
	cp suika ../../game-en/
	cd ../../game-en && valgrind --leak-check=full ./suika
