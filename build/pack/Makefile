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

pack.mac: $(SRC)
	$(CC) -o pack.mac -arch arm64 -arch x86_64 $(CPPFLAGS) $(CFLAGS) $(SRC)

pack.exe: $(SRC)
	i686-w64-mingw32-gcc -o pack.exe -municode $(CPPFLAGS) $(CFLAGS) $(SRC)
