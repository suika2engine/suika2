include ../common.mk

#
# Toolchain
#

# For cross compilation
CC = i686-w64-mingw32-gcc
WINDRES = i686-w64-mingw32-windres

# For native compilation
#CC = gcc
#WINDRES = windres

#
# CPPFLAGS
#

CPPFLAGS = \
	-I. \
	-Ires \
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

SRCS_R = res/resource.rc

#
# .c.o compilation rules
#

OBJS = \
	$(SRCS_C:../../src/%.c=%.o) \
	$(SRCS_R:res/%.rc=%.o)

%.o: ../../src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

%.o: res/%.rc
	$(WINDRES) $< $@

#
# Target
#

all: suika.exe

suika.exe: $(OBJS)
	$(CC) -o suika.exe $(OBJS) $(LDFLAGS)

#
# Feature specific source files.
#

include ../sse.mk

#
# Phony rules
#

package: suika.exe
	rm -rf suika2
	mkdir suika2
	cp -Rv ../../game/bg suika2/
	cp -Rv ../../game/bgm suika2/
	cp -Rv ../../game/cg suika2/
	cp -Rv ../../game/ch suika2/
	cp -Rv ../../game/conf suika2/
	cp -Rv ../../game/cv suika2/
	cp -Rv ../../game/font suika2/
	cp -Rv ../../game/se suika2/
	mkdir suika2/tool
	cp -v ../../tool/package-win.exe suika2/tool/
	cp -Rv ../../game/txt suika2/
	cp -v ../../game/COPYING suika2/
	cp -v ../../game/readme.en.html suika2/
	cp -v ../../game/readme.ja.html suika2/
	cp -v suika.exe suika2/
	zip -r suika-2.x.x-win.zip suika2

run: suika.exe
	cd suika2 && wine suika.exe

clean:
	rm -rf *.o *~ suika.exe log.txt sav tmp libroot suika2 suika-2.x.x-win.zip
