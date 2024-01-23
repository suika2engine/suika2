CC = clang
CPPFLAGS = -Ic/include
CFLAGS = -arch arm64

SRC_COMMON = \
	c/common/constants.c \
	c/common/dictionary.c \
	c/common/shared_dictionary.c \
	c/common/context.c \
	c/common/platform.c \
	c/common/transform.c

SRC_DEC = \
	c/dec/bit_reader.c \
	c/dec/decode.c \
	c/dec/huffman.c \
	c/dec/state.c

OBJ_COMMON = $(SRC_COMMON:c/common/%.c=%.o)

OBJ_DEC = $(SRC_DEC:c/dec/%.c=%.o)

%.o: c/common/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

%.o: c/dec/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

all: libbrotlicommon.a libbrotlidec.a
	cp -R c/include/brotli ../../libroot/include/

libbrotlicommon.a: $(OBJ_COMMON)
	$(AR) -rcu ../../libroot/lib/libbrotlicommon.a $(OBJ_COMMON)

libbrotlidec.a: $(OBJ_DEC)
	$(AR) -rcu ../../libroot/lib/libbrotlidec.a $(OBJ_DEC)
