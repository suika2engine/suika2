include ../common.mk

#
# Toolchain
#

# For native compilation
CC = gcc
WINDRES = windres

# For cross compilation
#CC = i686-w64-mingw32-gcc
#WINDRES = i686-w64-mingw32-windres

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

run: suika.exe
	cp suika.exe ../../suika2; cd ../../suika2; ./suika.exe

install: suika.exe
	cp suika.exe ../../suika2

clean:
	rm -rf *.o *~ suika.exe log.txt sav tmp

erase:
	rm -rf libroot
