usage:
	@echo "This Makefile is for dev internal build test before a git-commit."
	@echo "  make docker ... build docker containers"
	@echo "  make all    ... build all"
	@echo "  make extra  ... build android and switch with docker"
	@echo ""

setup:
	cd android && docker build -t suika2-android . && cd ..
	docker pull devkitpro/devkita64

all:
	cd mingw && rm -f *.o suika.exe && make -j24 && cd ..
	cd mingw-pro && rm -f *.o suika-pro.exe && make -j24 && cd ..
	cd mingw-studio && rm -f *.o suika-studio.exe && make -j24 && cd ..
	cd mingw-64 && rm -f *.o suika-64.exe && make -j24 && cd ..
	cd mingw-arm64 && rm -f *.o suika-arm64.exe && make -j24 && cd ..
	cd mingw-capture && rm -f *.o suika-capture.exe && make -j24 && cd ..
	cd mingw-replay && rm -f *.o suika-replay.exe && make -j24 && cd ..
	if [ ! -z "`uname -a | grep Linux`" ]; then cd linux-x86_64 && rm -f *.o suika && make -j24 && cd ..; fi
	if [ ! -z "`uname -a | grep Linux`" ]; then cd linux-x86_64-clang && rm -f *.o suika-replay && make -j24 && cd ..; fi
	if [ ! -z "`uname -a | grep Linux`" ]; then cd linux-x86_64-capture && rm -f *.o suika-capture && make -j24 && cd ..; fi
	if [ ! -z "`uname -a | grep Linux`" ]; then cd linux-x86_64-replay && rm -f *.o suika-replay && make -j24 && cd ..; fi
	if [ ! -z "`uname -a | grep Darwin`" ]; then cd macos && make -j8 main build-only && cd ..; fi
	cd emscripten && make && cd ..

extra: android-docker console-docker

android-docker:
	cd android && docker run -it --rm -v `readlink -f ../../`:/project suika2-android bash -c "cd /project/build/android && ./build-on-docker.sh" && cd ..

console-docker:
	cd console && docker run -it --rm -v `readlink -f ../../`:/project devkitpro/devkita64 bash -c "cd /project/build/console && make" && cd ..
