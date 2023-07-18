include ../common.mk

#
# Toolchain
#

export PATH = $(shell echo `readlink -f llvm/bin`:$$PATH)

# For cross compilation
CC = aarch64-w64-mingw32-gcc
CXX = aarch64-w64-mingw32-g++
WINDRES = aarch64-w64-mingw32-windres


#
# CPPFLAGS
#

CPPFLAGS = \
	-I./llvm/aarch64-w64-mingw32/include \
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
	-std=gnu89 \
	-finput-charset=utf-8 \
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
	../../src/winmain.c \
	../../src/uimsg.c \
	../../src/glrender.c \
	../../src/d3drender.c \
	../../src/dsound.c

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
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

%.o: ../mingw/res/%.rc
	$(WINDRES) $< $@

#
# Target
#

all: suika-arm64.exe

suika-arm64.exe: libroot $(OBJS)
	$(CC) -s -o suika-arm64.exe $(OBJS) $(LDFLAGS)

libroot:
	./build-libs.sh


#
# Phony
#

install: suika-arm64.exe
	cp suika-arm64.exe ../../

clean:
	rm -rf *.o *~ suika-arm64.exe log.txt sav tmp

erase:
	rm -rf *.o *~ suika-arm64.exe log.txt sav tmp libroot llvm
