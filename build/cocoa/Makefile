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

install: Suika
	cp -av Suika ../../suika2-mac/Suika.app/Contents/MacOS/Suika
	cp -av ../../suika2/COPYING ../../suika2-mac/Suika.app
	cp -Rav ../../suika2/conf ../../suika2-mac/Suika.app
	cp -Rav ../../suika2/se ../../suika2-mac/Suika.app
	cp -Rav ../../suika2/bg ../../suika2-mac/Suika.app
	cp -Rav ../../suika2/cv ../../suika2-mac/Suika.app
	cp -Rav ../../suika2/bgm ../../suika2-mac/Suika.app
	cp -Rav ../../suika2/font ../../suika2-mac/Suika.app
	cp -Rav ../../suika2/cg ../../suika2-mac/Suika.app
	cp -av ../../suika2/tool/package-mac ../../suika2-mac/tool/
	cp -Rav ../../suika2/ch ../../suika2-mac/Suika.app
	cp -Rav ../../suika2/txt ../../suika2-mac/Suika.app
	rm -rf ../../suika2-mac/Suika.app/sav
	rm -f ../../suika2-mac/Suika.app/log.txt

clean:
	rm -rf Suika $(OBJS) *~ log.txt sav tmp

erase:
	rm -rf libroot
