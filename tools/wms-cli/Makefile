CFLAGS=\
	-I../../src \
	-Wall \
	-Werror \
	-Wextra \
	-Wundef \
	-Wno-sign-compare \
	-std=gnu89

SOURCE=\
	../../src/wms_parser.tab.c \
	../../src/wms_lexer.yy.c \
	../../src/wms_core.c \
	main.c

# default target
all: wms

# Linux CLI binary
wms: $(SOURCE)
	gcc -O0 -g3 $(CFLAGS) -o $@ $^

# Windows CLI binary
wms.exe: $(SOURCE)
	i686-w64-mingw32-gcc -O2 $(CFLAGS) -s -o $@ $^

# parser
../../src/wms_parser.tab.c: ../../src/wms_parser.y
	bison -d -p wms_yy -o ../../src/wms_parser.tab.c ../../src/wms_parser.y

# lexer
../../src/wms_lexer.yy.c: ../../src/wms_lexer.l
	flex -o ../../src/wms_lexer.yy.c --prefix=wms_yy ../../src/wms_lexer.l

# analyzer
analyze-gcc: ../../src/wms_core.c main.c
	gcc -fanalyzer $(CFLAGS) $^

# analyzer
analyze-clang: $(SOURCE)
	clang --analyze $(CFLAGS) $^

# cleanup
clean:
	rm -rf wms wms.dSYM out *~ testcase/*~
