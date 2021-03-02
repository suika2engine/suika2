dummy:

package-win.exe: package.c
	i686-w64-mingw32-gcc -O2 -Wformat-truncation=0 -o package-win.exe package.c

package-mac: package.c
	clang -O2 -arch arm64 -arch x86_64 -mmacosx-version-min=10.9 -o package-mac package.c

package-linux: package.c
	gcc -O2 -Wformat-truncation=0 -o package-linux package.c
