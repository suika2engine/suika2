###
### The Build System of Suika2
###

DESTDIR=/usr/local

all: targets

# This will show the usage of this Makefile if no target is specified.
targets:
	@echo 'Welcome to Suika2! This is the build system of Suika2.'
	@echo
	@echo 'You can type the following commands:'
	@echo '  make setup   ... setup a build environment'
	@echo '  make windows ... build the main game engine for Windows'
	@echo '  make macos   ... build the main game engine for macOS'
	@echo '  make linux   ... build the main game engine for Linux'
	@echo '  make test    ... run tests without a window'
	@echo ''
	@if [ ! -z "`uname | grep Linux`" ]; then \
		echo 'On Linux, you can also type the following commands:'; \
		echo '  ./configure --prefix=/usr/local'; \
		echo '  make'; \
		echo '  sudo make install'; \
		echo ''; \
		if [ ! -z "`grep -i WSL2 /proc/version`" ]; then \
			case `pwd` in \
			/mnt/*)\
				echo 'Warning: You are on /mnt. Make sure to work on ~/ to avoid having Windows Security erase generated objects.'; \
				echo ; \
			esac; \
		fi; \
	fi

##
## Setup
##

# This will setup a build environment.
setup:
	@# For Linux including WSL2, install packages.
	@if [ ! -z "`uname | grep Linux`" ]; then \
		if [ -z "`which apt-get`" ]; then \
			echo 'Error: Your system lacks "apt-get" command.'; \
			exit 1; \
		fi; \
		echo 'Are you sure you want to install all dependencies? (press enter)'; \
		read str; \
		echo 'Updating apt sources.'; \
		echo 'sudo apt-get update'; \
		sudo apt-get update; \
		echo 'Installing dependencies.'; \
		sudo apt-get install mingw-w64 build-essential libasound2-dev libx11-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libxpm-dev mesa-common-dev xvfb lcov python3-pip debhelper-compat zlib1g-dev libpng-dev libjpeg9-dev libogg-dev libvorbis-dev libfreetype-dev cmake qt6-base-dev qt6-multimedia-dev libqt6core6 libqt6gui6 libqt6widgets6 libqt6opengl6-dev libqt6openglwidgets6 libqt6multimedia6 libqt6multimediawidgets6; \
		pip3 install opencv-python numpy; \
		cd build/linux-x86_64 && make libroot && cd ../..; \
		cd build/linux-x86_64-clang && make libroot && cd ../..; \
		cd build/linux-x86_64-capture && cp -Ra ../linux-x86_64/libroot . && cd ../..; \
		cd build/linux-x86_64-replay && cp -Ra ../linux-x86_64/libroot . && cd ../..; \
	fi
	@# For WSL2, build libraries with EXE execution turned off.
	@if [ ! -z "`uname -a | grep WSL2`" ]; then \
		echo "Disabling EXE file execution."; \
		echo 0 | sudo tee /proc/sys/fs/binfmt_misc/WSLInterop; \
		echo "Building libraries."; \
		cd build/mingw && make libroot && cd ../..; \
		cd build/mingw-64 && make libroot && cd ../..; \
		cd build/mingw-arm64 && make libroot && cd ../..; \
		cp -Ra build/mingw/libroot build/mingw-pro/; \
		cp -Ra build/mingw/libroot build/mingw-capture/; \
		cp -Ra build/mingw/libroot build/mingw-replay/; \
		echo "Re-enabling EXE file execution."; \
		echo 1 | sudo tee /proc/sys/fs/binfmt_misc/WSLInterop; \
	fi
	@# For macOS
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		if [ -z  "`which brew`" ]; then \
			echo 'Error: Your system lacks "brew" command.'; \
			exit 1; \
		fi; \
		echo 'Are you sure you want to install all dependencies? (press enter)'; \
		read str; \
		echo 'brew install mingw-w64'; \
		brew install mingw-w64; \
	fi

##
## Windows
##

# A target for all Windows binaries.
all-windows: windows windows-pro windows-64 windows-arm64 windows-capture windows-replay

# suika.exe (the main game engine for 32-bit Windows)
windows:
	@echo 'Building suika.exe'
	@cd build/mingw && \
	make libroot && \
	make -j8 && \
	make install && \
	cd ../..

# suika-pro.exe (the debugger for 32-bit Windows)
windows-pro:
	@echo 'Building suika-pro.exe'
	@cd build/mingw-pro && \
	make libroot && \
	make -j8 && \
	make install && \
	cd ../..

# suika-64.exe (the main game engine for 64-bit Windows)
windows-64:
	@echo 'Building suika-64.exe'
	@cd build/mingw-64 && \
	make libroot && \
	make -j8 && \
	make install && \
	cd ../..

# suika-arm64.exe (the main game engine for Arm64 Windows)
windows-arm64:
	@echo 'Building suika-arm64.exe'
	@cd build/mingw-arm64 && \
	make libroot && \
	make -j8 && \
	make install && \
	cd ../..

# suika-capture.exe (the caputure app)
windows-capture:
	@echo 'Building suika-capture.exe'
	@cd build/mingw-capture && \
	make libroot && \
	make -j8 && \
	make install && \
	cd ../..

# suika-replay.exe (the replay app)
windows-replay:
	@echo 'Building suika-replay.exe'
	@cd build/mingw-capture && \
	make libroot && \
	make -j8 && \
	make install && \
	cd ../..

##
## macOS
##

# A target for all macOS binaries.
all-macos: macos-main macos-pro macos-capture macos-replay

# A target for the main game engine for macOS.
macos:
	@echo 'Building macOS app'
	@cd build/macos && \
		make main && \
		cp suika.dmg ../../ && \
		make clean && \
	cd ../..

# A target for the debugger for macOS.
macos-pro:
	@echo 'Building macOS debugger app'
	@cd build/macos && \
		make pro && \
		cp suika-pro.dmg ../../ && \
		make clean && \
	cd ../..

# A target for the capture app for macOS.
macos-capture:
	@echo 'Building macOS capture app'
	@cd build/macos && \
		make capture && \
		cp suika-capture.dmg ../../ && \
		make clean && \
	cd ../..

# A target for the replay app for macOS.
macos-replay:
	@echo 'Building macOS replay app'
	@cd build/macos && \
		make replay && \
		cp suika-replay.dmg ../../ && \
		make clean && \
	cd ../..

##
## Linux
##

# A target for all Linux binaries.
all-linux: linux linux-pro linux-capture linux-replay

# suika-linux (the main game engine for 64-bit Linux, static link)
linux:
	@echo 'Building a Linux game binary'
	@cd build/linux-x86_64 && \
	make libroot && \
	make -j8 && \
	make install && \
	cd ../..

# suika-linux (the main game engine for 64-bit Linux, dynamic link)
linux-shared:
	@echo 'Building a Linux game binary'
	@cd build/linux-x86_64-shared && \
	make -j8 && \
	make install && \
	cd ../..

# suika-pro (the debugger for Linux, static link)
linux-pro:
	@echo 'Building for Linux'
	@cd build/linux-x86_64-pro && \
	./make-deps.sh && \
	rm -rf build && \
	mkdir build && \
	cd build && \
	cmake .. && \
	make && \
	cp suika-pro ../../../ && \
	cd ../../..

# suika-pro (the debugger for Linux, dynamic link)
linux-pro-shared:
	@echo 'Building for Linux'
	@cd build/linux-x86_64-pro && \
	./make-deps-shared.sh && \
	rm -rf build && \
	mkdir build && \
	cd build && \
	cmake .. && \
	make && \
	cp suika-pro ../../../ && \
	cd ../../..

# suika-linux-capture (the capture app for 64-bit Linux)
linux-capture:
	@echo 'Building a Linux capture binary'
	@cd build/linux-x86_64-capture && \
	make libroot && \
	make -j8 && \
	make install && \
	cd ../..

# suika-linux-replay (the replay app for 64-bit Linux)
linux-replay:
	@echo 'Building a Linux replay binary'
	@cd build/linux-x86_64-replay && \
	make libroot && \
	make -j8 && \
	make install && \
	cd ../..

##
## Tests
##

test:
	@echo "Use 'ctest' for CUI and 'gtest' for GUI"

# Non-graphical automatic tests.
ctest:
	@echo 'Running non-graphical tests...'
	@# Check if we are running on Linux including WSL2.
	@if [ -z "`uname | grep Linux`" ]; then \
		echo "Error: this target needs Linux."; \
		exit 1; \
	fi
	@# Fetch the testcase repository.
	@if [ ! -d testcases ]; then \
		git clone https://github.com/suika2engine/testcases.git; \
	else \
		cd testcases; \
		git pull origin master; \
		cd ..; \
	fi
	@# Run the testcases in a virtual X server.
	@cd testcases && \
	./run.sh --no-x11 && \
	cd ..

# Graphical automatic tests.
gtest:
	@echo 'Running graphical tests...'
	@# Check if we are running on Linux including WSL2.
	@if [ -z "`uname | grep Linux`" ]; then \
		echo "Error: this target needs Linux."; \
		exit 1; \
	fi
	@# Fetch the testcase repository.
	@if [ ! -d testcases ]; then \
		git clone https://github.com/suika2engine/testcases.git; \
	else \
		cd testcases; \
		git pull origin master; \
		cd ..; \
	fi
	@# Run the testcases on a real X server.
	@cd testcases && \
	./run.sh && \
	cd ..

##
## Release (dev internal)
##

# Make a main release file and update the Web site.
do-release:
	@# Check if we are running on WSL2.
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		echo "Warning: we are on macOS and we will make Windows binaries without code signing."; \
		echo ""; \
	elif [ -z "`grep -i WSL2 /proc/version`" ]; then \
		echo "Warning: we are on non-WSL2 Linux and we will make Windows binaries without code signing."; \
		echo ""; \
	fi
	@cd build && ./scripts/release-main.sh && cd ..

# Make Kirara release files and upload them.
kirara:
	@# Check if we are running on WSL2.
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		echo "Warning: we are on macOS and we will make Windows binaries without code signing."; \
		echo ""; \
	elif [ -z "`grep -i WSL2 /proc/version`" ]; then \
		echo "Warning: we are on non-WSL2 Linux and we will make Windows binaries without code signing."; \
		echo ""; \
	fi
	@cd build && ./scripts/release-kirara.sh && cd ..

# Update template games.
update-templates:
	@echo "Going to update template games."
	@cd build && ./scripts/release-templates.sh && cd ..

##
## POSIX Convention
##

# Build for Linux/BSD
build: linux-shared linux-pro-shared

# Install for Linux/BSD
install:
	@echo 'Installing Suika2'
	@install -v -d $(DESTDIR)/bin
	@install -v -d $(DESTDIR)/share/suika2/game
	@install -v -d $(DESTDIR)/share/suika2/game/anime
	@install -v -d $(DESTDIR)/share/suika2/game/bg
	@install -v -d $(DESTDIR)/share/suika2/game/bgm
	@install -v -d $(DESTDIR)/share/suika2/game/ch
	@install -v -d $(DESTDIR)/share/suika2/game/cg
	@install -v -d $(DESTDIR)/share/suika2/game/conf
	@install -v -d $(DESTDIR)/share/suika2/game/cv
	@install -v -d $(DESTDIR)/share/suika2/game/gui
	@install -v -d $(DESTDIR)/share/suika2/game/txt
	@install -v -d $(DESTDIR)/share/suika2/game/font
	@install -v -d $(DESTDIR)/share/suika2/game/rule
	@install -v -d $(DESTDIR)/share/suika2/game/se
	@install -v -d $(DESTDIR)/share/suika2/game/wms
	@install -v -d $(DESTDIR)/share/suika2/game/mov
	@install -v suika-linux $(DESTDIR)/bin
	@install -v build/linux-x86_64-pro/build/suika-pro $(DESTDIR)/bin
	@install -v build/linux-x86_64-pro/suika2 $(DESTDIR)/bin
	@install -v -t $(DESTDIR)/share/suika2/game/anime game/anime/*
	@install -v -t $(DESTDIR)/share/suika2/game/bg game/bg/*
	@install -v -t $(DESTDIR)/share/suika2/game/bgm game/bgm/*
	@install -v -t $(DESTDIR)/share/suika2/game/ch game/ch/*
	@install -v -t $(DESTDIR)/share/suika2/game/cg game/cg/*
	@install -v -t $(DESTDIR)/share/suika2/game/conf game/conf/*
	@install -v -t $(DESTDIR)/share/suika2/game/cv game/cv/*
	@install -v -t $(DESTDIR)/share/suika2/game/gui game/gui/*
	@install -v -t $(DESTDIR)/share/suika2/game/txt game/txt/*
	@install -v -t $(DESTDIR)/share/suika2/game/font game/font/*
	@install -v -t $(DESTDIR)/share/suika2/game/rule game/rule/*
	@install -v -t $(DESTDIR)/share/suika2/game/se game/se/*
	@install -v -t $(DESTDIR)/share/suika2/game/wms game/wms/*

##
## Other
##

# Cleanup.
clean:
	rm -f suika.exe suika-pro.exe suika-64.exe suika-arm64.exe suika-capture.exe suika-replay.exe
	rm -f mac.dmg mac-pro.dmg mac-capture.dmg mac-replay.dmg mac.zip pack.mac
	rm -f suika-linux suika-pro suika-linux-capture suika-linux-replay
