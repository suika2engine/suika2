CPPFLAGS=\
	-DUSE_DEBUGGER \
	-I../../src

CFLAGS=\
	-Os \
	-Wformat-truncation=0

CFLAGS_MAC=

LDFLAGS=-s

SRC=\
	../../src/package.c \
	../../src/log.c \
	pack.c

all:
	@echo 'This is a command line packager for Suika2!'
	@echo 
	@echo 'You can type the following commands:'
	@echo '  make pack-linux'
	@echo '  make pack.mac'
	@echo '  make pack.exe'
	@echo

pack: $(SRC)
	$(CC) -o pack $(CPPFLAGS) $(CFLAGS) $(LDFLAGS)$(SRC)

pack.mac: $(SRC)
	$(CC) -o pack.mac -arch arm64 -arch x86_64 $(CPPFLAGS) $(CFLAGS_MAC) $(LDFLAGS) $(SRC)

pack.exe: $(SRC)
	i686-w64-mingw32-gcc -o pack.exe -municode $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(SRC)
