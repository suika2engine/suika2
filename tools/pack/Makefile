CPPFLAGS=\
	-DUSE_EDITOR \
	-I../../src

CFLAGS=\
	-Os \
	-Wformat-truncation=0

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
	@echo '  make pack-mac'
	@echo '  make pack-win.exe'
	@echo

pack-linux: $(SRC)
	$(CC) -o pack $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(SRC)

pack-mac: $(SRC)
	$(CC) -o pack-mac -arch arm64 -arch x86_64 $(CPPFLAGS) $(CFLAGS_MAC) $(LDFLAGS_MAC) $(SRC)

pack-win.exe: $(SRC)
	i686-w64-mingw32-gcc -o pack-win.exe -municode $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(SRC)
