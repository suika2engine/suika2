#
# This is for test build before a git-commit.
#
build-test:
	cd mingw && rm -f *.o suika.exe && make -j24 && cd ..
	cd mingw-pro && rm -f *.o suika-pro.exe && make -j24 && cd ..
	cd mingw-64 && rm -f *.o suika-64.exe && make -j24 && cd ..
	cd mingw-arm64 && rm -f *.o suika-arm64.exe && make -j24 && cd ..
	cd mingw-capture && rm -f *.o suika-capture.exe && make -j24 && cd ..
	cd mingw-replay && rm -f *.o suika-replay.exe && make -j24 && cd ..
