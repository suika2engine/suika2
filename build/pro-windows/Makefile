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
	-DUSE_EDITOR \
	-DUSE_DEBUGGER \
	-D_WIN32_WINNT=0x0601 \
	-I. \
	-I./libroot/include \
	-I./libroot/include/freetype2

#
# CFLAGS
#

CFLAGS = \
	-O3 \
	-municode \
	-ffast-math \
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
	-ldsound \
	-ldxguid \
	-ld3d9 \
	-ld3dx9_43 \
	-lstrmiids \
	-lcomctl32 \
	-limm32 \
	-L./libroot/lib \
	-Wl,-dn,-lpng16,-ljpeg,-lwebp,-lfreetype,-lz,-lbz2,-lvorbisfile,-lvorbis,-logg,-dy \
	-Wl,--gc-sections

#
# Source files
#

SRCS_C = \
	../../src/microsoft/winpro.c \
	../../src/microsoft/dsound.c

SRCS_CC = \
	../../src/microsoft/dx9render.cc \
	../../src/microsoft/dsvideo.cc

SRCS_R = resource.rc

HDRS = \
	../../src/microsoft/dsound.h \
	../../src/microsoft/dx9render.h \
	../../src/microsoft/dsvideo.h \
	../../src/microsoft/tts_sapi.h

#
# .c.o compilation rules
#

OBJS = \
	$(SRCS_C:../../src/microsoft/%.c=%.o) \
	$(SRCS_CC:../../src/microsoft/%.cc=%.o) \
	$(SRCS_MAIN:../../src/%.c=%.o) \
	$(SRCS_PRO:../../src/%.c=%.o) \
	$(SRCS_R:%.rc=%.o)

%.o: ../../src/microsoft/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

%.o: ../../src/microsoft/%.cc
	$(CXX) -c $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $<

%.o: ../../src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

%.o: %.rc
	$(WINDRES) -DUSE_EDITOR $< $@

#
# Target
#

all: suika-pro.exe

suika-pro.exe: libroot $(OBJS) $(HDRS_MAIN) $(HDRS)
	$(CC) -s -o suika-pro.exe $(OBJS) $(LDFLAGS)

libroot:
	./build-libs.sh

#
# Phony
#

install: suika-pro.exe
	cp suika-pro.exe ../../

clean:
	rm -rf *.o *~ suika-pro.exe log.txt sav tmp

erase:
	rm -rf *.o *~ suika-pro.exe log.txt sav tmp libroot

analyze:
	$(CC) $(CPPFLAGS) -municode -fanalyzer -finput-charset=utf-8 -c $(SRCS_C) $(SRCS_CC)
