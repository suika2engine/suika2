include ../common.mk

#
# Toolchain
#

# For cross compilation
CC = i686-w64-mingw32-gcc
CXX = i686-w64-mingw32-g++
WINDRES = i686-w64-mingw32-windres

#
# CPPFLAGS
#

CPPFLAGS = \
	-DUSE_REPLAY \
	-I. \
	-I../mingw/res \
	-I./libroot/include \
	-I./libroot/include/freetype2

#
# CFLAGS
#

CFLAGS = \
	-O3 \
	-municode \
	-ffast-math \
	-fopt-info-vec \
	-finput-charset=utf-8 \
	-fexec-charset=utf-8 \
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
	-municode \
	-lgdi32 \
	-lole32 \
	-ldxguid \
	-ldsound \
	-lstrmiids \
	-lwinmm \
	-lcomctl32 \
	-lopengl32 \
	-ld3d9 \
	-L./libroot/lib \
	-Wl,-dn,-ljpeg,-lpng16,-lz,-lvorbisfile,-lvorbis,-logg,-lfreetype,-dy \
	-Wl,--gc-sections

#
# Source files
#

SRCS_C = \
	$(SRCS_COMMON) \
	$(SRCS_SSE) \
	../../src/winmain.c \
	../../src/winreplay.c \
	../../src/uimsg.c \
	../../src/dsound.c \
	../../src/glrender.c \
	../../src/d3drender.cc

SRCS_CC = \
	../../src/dsvideo.cc

SRCS_R = ../mingw/res/resource.rc

#
# .c.o compilation rules
#

OBJS = \
	$(SRCS_C:../../src/%.c=%.o) \
	$(SRCS_CC:../../src/%.cc=%.o) \
	$(SRCS_R:../mingw/res/%.rc=%.o)

%.o: ../../src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

%.o: ../../src/%.cc
	$(CXX) -c $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $<

%.o: ../mingw/res/%.rc
	$(WINDRES) -DUSE_DEBUGGER $< $@

#
# Target
#

all: suika-replay.exe

suika-replay.exe: libroot $(OBJS)
	$(CC) -s -o suika-replay.exe $(OBJS) $(LDFLAGS)

libroot:
	./build-libs.sh

#
# Feature specific source files.
#

include ../sse.mk

#
# Phony
#

install: suika-replay.exe
	cp suika-replay.exe ../../

clean:
	rm -rf *.o *~ suika-replay.exe log.txt sav tmp

erase:
	rm -rf *.o *~ suika-replay.exe log.txt sav tmp libroot
