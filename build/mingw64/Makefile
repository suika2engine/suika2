include ../common.mk

#
# Toolchain
#

# For cross compilation
CC = x86_64-w64-mingw32-gcc
WINDRES = x86_64-w64-mingw32-windres

#
# CPPFLAGS
#

CPPFLAGS = \
	-I. \
	-I../mingw/res \
	-I./libroot/include \
	-I./libroot/include/freetype2

#
# CFLAGS
#

CFLAGS = \
	-O3 \
	-ffast-math \
	-fopt-info-vec \
	-std=gnu89 \
	-finput-charset=utf-8 \
	-fexec-charset=cp932 \
	-Wall \
	-Werror \
	-Wextra \
	-Wundef \
	-Wconversion

#
# LDFLAGS
#

LDFLAGS = \
	-mwindows \
	-lgdi32 \
	-lole32 \
	-ldxguid \
	-ldsound \
	-L./libroot/lib \
	-Wl,-dn,-lpng16,-lz,-lvorbisfile,-lvorbis,-logg,-lfreetype,-dy

#
# Source files
#

SRCS_C = \
	$(SRCS_COMMON) \
	$(SRCS_SSE) \
	../../src/winmain.c \
	../../src/dsound.c

SRCS_R = ../mingw/res/resource.rc

#
# .c.o compilation rules
#

OBJS = \
	$(SRCS_C:../../src/%.c=%.o) \
	$(SRCS_R:../mingw/res/%.rc=%.o)

%.o: ../../src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

%.o: ../mingw/res/%.rc
	$(WINDRES) $< $@

#
# Target
#

all: suika64.exe

suika64.exe: $(OBJS)
	$(CC) -s -o suika64.exe $(OBJS) $(LDFLAGS)

#
# Feature specific source files.
#

include ../sse.mk

#
# Phony
#

install: suika64.exe
	cp suika64.exe ../../

clean:
	rm -rf *.o *~ suika64.exe log.txt sav tmp libroot
