include ../common.mk

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
	-arch i386 \
	-arch x86_64 \
	-mmacosx-version-min=10.6 \
	-O3 \
	-ffast-math \
	-fvectorize \
	-Rpass=loop-vectorize \
	-Rpass-missed=loop-vectorize \
	-Rpass-analysis=loop-vectorize \
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
	-arch i386 \
	-arch x86_64 \
	-mmacosx-version-min=10.6 \
	-L./libroot/lib \
	-lz \
	-lpng16 \
	-logg \
	-lvorbis \
	-lvorbisfile \
	-lfreetype \
	-lpthread \
	-framework Cocoa \
	-framework AudioUnit

#
# Source files
#

SRCS_C = \
	$(SRCS_COMMON) \
	$(SRCS_SSE) \
	../../src/aunit.c

SRCS_M = \
	../../src/nsmain.m

#
# .c/.m Compilation rules
#

OBJS = \
	$(SRCS_C:../../src/%.c=%.o) \
	$(SRCS_M:../../src/%.m=%.o)

%.o: ../../src/%.m
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

%.o: ../../src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

#
# Target
#

Suika: $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS)

#
# Feature specific compilation rules.
#

include ../sse.mk

#
# Phony rules
#

package: suika
	rm -rf suika2
	mkdir suika2
	cp -Rav Suika.app suika2/
	cp -av Suika suika2/Suika.app/Contents/MacOS/Suika
	cp -Rv ../../game/bg suika2/Suika.app/
	cp -Rv ../../game/bgm suika2/Suika.app/
	cp -Rv ../../game/cg suika2/Suika.app/
	cp -Rv ../../game/ch suika2/Suika.app/
	cp -Rv ../../game/conf suika2/Suika.app/
	cp -Rv ../../game/cv suika2/Suika.app/
	cp -Rv ../../game/font suika2/Suika.app/
	cp -Rv ../../game/se suika2/Suika.app/
	mkdir suika2/tool
	cp -v ../../tool/package-mac suika2/tool/
	cp -Rv ../../game/txt suika2/Suika.app/
	cp -v ../../game/COPYING suika2/
	cp -v ../../game/readme.en.html suika2/
	cp -v ../../game/readme.ja.html suika2/
	zip -r suika-2.x.x-mac.zip suika2

clean:
	rm -rf Suika $(OBJS) *~ log.txt sav tmp libroot suika2 suika-2.x.x-mac.zip

erase:
	rm -rf libroot
