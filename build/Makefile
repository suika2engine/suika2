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
	cd engine-windows-x86 && rm -f *.o suika.exe && make -j24 && cd ..
	cd engine-windows-x86_64 && rm -f *.o suika-64.exe && make -j24 && cd ..
	cd engine-windows-arm64 && rm -f *.o suika-arm64.exe && make -j24 && cd ..
	cd engine-wasm && make && cd ..
	cd studio-windows-x86 && rm -f *.o suika-studio.exe && make -j24 && cd ..
	cd studio-wasm && make && cd ..
	cd pro-windows-x86 && rm -f *.o suika-pro.exe && make -j24 && cd ..
	cd capture-windows-x86 && rm -f *.o suika-capture.exe && make -j24 && cd ..
	cd replay-windows-x86 && rm -f *.o suika-replay.exe && make -j24 && cd ..
	if [ ! -z "`uname -a | grep Linux`" ]; then cd engine-linux-x86_64 && rm -f *.o suika && make -j24 && cd ..; fi
	if [ ! -z "`uname -a | grep Linux`" ]; then cd engine-linux-x86_64-clang && rm -f *.o suika-replay && make -j24 && cd ..; fi
	if [ ! -z "`uname -a | grep Linux`" ]; then cd capture-linux-x86_64 && rm -f *.o suika-capture && make -j24 && cd ..; fi
	if [ ! -z "`uname -a | grep Linux`" ]; then cd replay-x86_64 && rm -f *.o suika-replay && make -j24 && cd ..; fi
	if [ ! -z "`uname -a | grep Darwin`" ]; then cd all-macos && make -j8 main build-only && cd ..; fi

extra: android-docker console-docker

android-docker:
	cd engine-android && docker run -it --rm -v `readlink -f ../../`:/project suika2-android bash -c "cd /project/build/engine-android && ./build-on-docker.sh" && cd ..

console-docker:
	cd engine-console && docker run -it --rm -v `readlink -f ../../`:/project devkitpro/devkita64 bash -c "cd /project/build/engine-console && make" && cd ..
