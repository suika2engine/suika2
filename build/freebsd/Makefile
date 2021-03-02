#
# Toolchain selection
#

CC = clang 

#
# CPPFLAGS
#

CPPFLAGS = \
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
	-lbz2 \
	-L/usr/local/lib \
	-lasound \
	-lX11 \
	-lXpm \
	-L./libroot/lib \
	-Wl,-dn,-lpng16,-lz,-lvorbisfile,-lvorbis,-logg,-lfreetype,-dy 

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
	cp suika ../../game/

clean:
	rm -rf *~ *.o suika log.txt sav tmp libroot
