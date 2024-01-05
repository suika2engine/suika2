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
	@cd engine-windows && rm -f *.o suika.exe && make -j24 && cd ..
	@cd engine-windows-64 && rm -f *.o suika-64.exe && make -j24 && cd ..
	@cd engine-windows-arm64 && rm -f *.o suika-arm64.exe && make -j24 && cd ..
	@cd pro-windows && rm -f *.o suika-pro.exe && make -j24 && cd ..
	@cd engine-wasm && make && cd ..
	@cd pro-wasm && make && cd ..
	@if [ ! -z "`uname -a | grep Linux`" ]; then \
		cd engine-linux && rm -f *.o suika && make -f Makefile.linux -j12 && cd ..; \
		cd engine-linux && rm -f *.o suika && CC=clang make -f Makefile.linux -j12 && cd ..; \
	fi
	@if [ ! -z "`uname -a | grep Darwin`" ]; then \
		cd engine-macos && make build-only && cd ..; \
		cd engine-ios && make build-only && cd ..; \
		cd pro-macos && make build-only && cd ..; \
		cd pro-ios && make build-only && cd ..; \
	fi

extra:
	cd engine-android && docker run -it --rm -v `readlink -f ../../`:/project suika2-android bash -c "cd /project/build/engine-android && ./build-on-docker.sh" && cd ..
