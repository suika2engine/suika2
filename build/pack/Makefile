CPPFLAGS=\
	-DUSE_DEBUGGER \
	-I../../src

CFLAGS=\
	-Wformat-truncation=0

SRC=\
	../../src/package.c \
	../../src/log.c \
	pack.c

pack: $(SRC)
	$(CC) -o pack $(CPPFLAGS) $(CFLAGS) $(SRC)
