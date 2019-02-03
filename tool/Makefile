dummy:

package-linux32: package.c
	gcc -O2 -Wformat-truncation=0 -o package-linux32 package.c

package-linux64: package.c
	gcc -O2 -Wformat-truncation=0 -o package-linux64 package.c

package-mac: package.c
	clang -O2 -Wformat-truncation=0 -arch i386 -arch x86_64 -o package-mac package.c

package-win.exe: package.c
	i686-w64-mingw32-gcc -O2 -Wformat-truncation=0 -o package-win.exe package.c
