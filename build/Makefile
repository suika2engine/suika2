usage:
	@echo "This is an internal build test before a git-commit."
	@echo "  make setup ... install tools, build libraries and make docker containers"
	@echo "  make all   ... build all"
	@echo "  make extra ... build android and switch with docker"
	@echo ""

setup:
	sudo apt-get update
	sudo apt-get install mingw-w64 build-essential libasound2-dev libx11-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libxpm-dev mesa-common-dev xvfb lcov python3-pip
	pip3 install opencv-python numpy
	cd mingw && ./build-libs.sh && cd..
	cp -Ra mingw/libroot mingw-pro/
	cp -Ra mingw/libroot mingw-capture/
	cp -Ra mingw/libroot mingw-replay/
	cd mingw-64 && ./build-libs.sh && cd ..
	cd mingw-arm64 && ./build-libs.sh && cd ..
	cd linux-x86_64 && ./build-libs.sh && cd ..
	cp -Ra linux-x86_64/libroot linux-x86_64-capture/libroot
	cp -Ra linux-x86_64/libroot linux-x86_64-replay/libroot
	cd linux-x86_64-clang && ./build-libs.sh && cd ..
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
