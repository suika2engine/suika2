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
	-L./libroot/lib \
	-Wl,-dn,-lpng16,-lz,-lvorbisfile,-lvorbis,-logg,-lfreetype,-dy \
	-lmvec

#
# Source files
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
	rm -rf *~ *.o suika log.txt sav tmp libroot suika2 suika-2.x.x-linux64.zip

lines:
	cat ../../src/*.[chm] | wc -l
	grep -E '/\*|\*/|^([ \t]+\*)' ../../src/*.[cm] | wc -l
	grep 'assert' ../../src/*.[chm] | wc -l
