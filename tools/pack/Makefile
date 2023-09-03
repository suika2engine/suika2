CPPFLAGS=\
	-DUSE_DEBUGGER \
	-I../../src

CFLAGS=\
	-Wformat-truncation=0

CFLAGS_MAC=

SRC=\
	../../src/package.c \
	../../src/log.c \
	pack.c

all:
	@echo 'This is a command line packager for Suika2!'
	@echo 
	@echo 'You can type the following commands:'
	@echo '  make pack'
	@echo '  make pack.mac'
	@echo '  make pack.exe'
	@echo

pack: $(SRC)
	$(CC) -o pack $(CPPFLAGS) $(CFLAGS) $(SRC)

pack.mac: $(SRC)
	$(CC) -o pack.mac -arch arm64 -arch x86_64 $(CPPFLAGS) $(CFLAGS_MAC) $(SRC)

pack.exe: $(SRC)
	i686-w64-mingw32-gcc -o pack.exe -municode $(CPPFLAGS) $(CFLAGS) $(SRC)
