usage:
	@echo "This is an internal build test before a git-commit."
	@echo "  make setup ... build libraries and make docker containers"
	@echo "  make all   ... build all"
	@echo "  make extra ... build android and switch with docker"
	@echo ""

setup:
	cd android && docker build -t suika2-android . && cd ..
	docker pull devkitpro/devkita64

all:
	cd mingw && rm -f *.o suika.exe && make -j24 && cd ..
	cd mingw-pro && rm -f *.o suika-pro.exe && make -j24 && cd ..
	cd mingw-64 && rm -f *.o suika-64.exe && make -j24 && cd ..
	cd mingw-arm64 && rm -f *.o suika-arm64.exe && make -j24 && cd ..
	cd mingw-capture && rm -f *.o suika-capture.exe && make -j24 && cd ..
	cd mingw-replay && rm -f *.o suika-replay.exe && make -j24 && cd ..
	cd linux-x86_64 && rm -f *.o suika && make -j24 && cd ..
	cd linux-x86_64-clang && rm -f *.o suika-replay && make -j24 && cd ..
	cd linux-x86_64-capture && rm -f *.o suika-capture && make -j24 && cd ..
	cd linux-x86_64-replay && rm -f *.o suika-replay && make -j24 && cd ..
	cd emscripten && make && cd ..

extra:
	cd android && docker run -it --rm -v `readlink -f ../../`:/project suika2-android bash -c "cd /project/build/android && ./build-on-docker.sh" && cd ..
	cd switch && docker run -it --rm -v `readlink -f ../../`:/project devkitpro/devkita64 bash -c "cd /project/build/switch && make" && cd ..
