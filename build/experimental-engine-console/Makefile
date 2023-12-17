#
# Toolchain selection
#
TARGET := suika
SWITCH_IP := 
SUIKA_DIR := $(TARGET)
ROMFS := ../../game

# must use msys2 env in windows, cmd might failed
ifeq ($(strip $(DEVKITPRO)),)
  $(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif
DEVKITPRO := $(subst \\,/,$(DEVKITPRO))
include $(DEVKITPRO)/libnx/switch_rules

APP_TITLE := $(TARGET)
APP_VERSION := 12.1.0
APP_AUTHOR := devseed
APP_ICON := icon.jpg
NROFLAGS += --icon=$(APP_ICON) --nacp=$(TARGET).nacp
# NROFLAGS += --romfsdir=$(ROMFS)

ARCH :=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
ASFLAGS	:= $(ARCH)
export LD := $(CC)
export NROFLAGS

#
# INCLUDES
#

INCLUDES = \
	-I$(PORTLIBS)/include \
	-I$(PORTLIBS)/include/freetype2 \
	-I$(LIBNX)/include

#
# CFLAGS
#

CFLAGS = \
	-O3 \
	-ffast-math \
	-ftree-vectorize \
	-std=gnu99 \
	-Wall \
	-Wextra \
	-Wno-undef \
	-Wno-conversion \
	-Wno-sign-conversion \
	-Wno-implicit-function-declaration \
	-DCONSOLE_SAMPLE \
	$(ARCH)

#
# LDFLAGS
#

LIBDIRS	:= $(PORTLIBS) $(LIBNX)
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

LDFLAGS = \
	-specs=$(DEVKITPRO)/libnx/switch.specs -fPIE \
	-L$(DEVKITPRO)/libnx/lib \
	-L$(DEVKITPRO)/portlibs/switch/lib \
	-lSDL2 -lGLESv2 -lEGL \
	-ldrm_nouveau -lglapi -lnx -lc -lstdc++ \
	-lwebp -ljpeg -lpng16 -lz -lbz2 \
	-lvorbisfile -lvorbis -lm \
	-logg -lfreetype

#
# Source files
#

include ../common.mk

SRCS_KHRONOS = ../../src/khronos/glrender.c
SRCS_ECTRA = ../../src/extra/consolemain.c

all: $(TARGET).nro

#
# .c.o compilation rules
#

OBJS = \
	$(SRCS_KHRONOS:../../src/khronos/%.c=%.o) \
	$(SRCS_EXTRA:../../src/extra/%.c=%.o) \
	$(SRCS_MAIN:../../src/%.c=%.o)

%.o: ../../src/khronos/%.c
	$(CC) -c $(INCLUDES) $(CFLAGS) $<

%.o: ../../src/extra/%.c
	$(CC) -c $(INCLUDES) $(CFLAGS) $<

%.o: ../../src/%.c
	$(CC) -c $(INCLUDES) $(CFLAGS) $<

#
# Target
#

$(TARGET).elf : $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(TARGET).nacp : $(TARGET).elf

$(TARGET).nro : $(TARGET).elf $(TARGET).nacp

$(OUTPUT).nso :	$(OUTPUT).elf

$(OUTPUT).nsp : $(OUTPUT).nso $(OUTPUT).npdm
#
# Phony
#

install: $(TARGET).nro
	cp $(TARGET).nro ../../

install_switch: $(TARGET).nro
	curl -T $< ftp://$(SWITCH_IP):5000/switch/$(SUIKA_DIR)/

debug: $(TARGET).nro
	$(DEVKITPRO)/tools/bin/nxlink -s -a $(SWITCH_IP) $< --args gamedir:$(SUIKA_DIR) savedir:$(SUIKA_DIR)

clean:
	rm -rf *~ *.o *.nsp *.nso *.npdm suika log.txt sav tmp

.Phony: all install install_switch debug
