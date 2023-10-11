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
	@echo '  make setup   ... install development tools'
	@echo '  make windows ... build the main game engine for Windows'
	@echo '  make macos   ... build the main game engine for macOS'
	@echo '  make linux   ... build the main game engine for Linux'
	@echo '  make test    ... run tests without a window'
	@echo ''
	@if [ ! -z "`uname | grep Linux`" ]; then \
		echo 'On Linux, you can also type the following commands:'; \
		echo '  make build'; \
		echo '  sudo make install INSTALL_DIR=/usr'; \
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

# This will setup the compilers and the tools.
setup:
	@# For Linux including WSL2.
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
		echo 'Installing dependencies for Windows targets.'; \
		echo 'sudo apt-get install mingw-w64'; \
		sudo apt-get install mingw-w64; \
		echo 'Installing dependencies for Linux targets.'; \
		echo 'sudo apt-get install build-essential libasound2-dev libx11-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libxpm-dev mesa-common-dev xvfb lcov cmake qt6-base-dev qt6-multimedia-dev'; \
		sudo apt-get install build-essential libasound2-dev libx11-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libxpm-dev mesa-common-dev xvfb lcov cmake qt6-base-dev qt6-multimedia-dev'; \
		echo 'Installing dependencies for testing targets.'; \
		echo 'sudo apt-get install python3-pip'; \
		echo 'pip3 install opencv-python numpy'; \
		sudo apt-get install python3-pip; \
		pip3 install opencv-python numpy; \
	fi
	@# For macOS
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		if [ -z  "`which brew`" ]; then \
			echo 'Error: Your system lacks "brew" command.'; \
			exit 1; \
		fi; \
		echo 'Are you sure you want to install all dependencies? (press enter)'; \
		read str; \
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
		make capture && \
		cp suika-capture.dmg ../../ && \
		make clean && \
	cd ../..

##
## Linux
##

# A target for all Linux binaries.
all-linux: linux linux-pro linux-capture linux-replay

# suika-linux (the main game engine for 64-bit Linux)
linux:
	@echo 'Building a Linux game binary'
	@cd build/linux-x86_64 && \
	make libroot && \
	make -j8 && \
	make install && \
	cd ../..

# suika-linux (the main game engine for 64-bit Linux)
linux-shared:
	@echo 'Building a Linux game binary'
	@cd build/linux-x86_64-shared && \
	make -j8 && \
	make install && \
	cd ../..

# suika-pro (the debugger for Linux)
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

# suika-pro (the debugger for Linux)
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

# Non-graphical automatic tests.
test:
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

# Make a main release file and upload it.
do-release:
	@# Check if we are running on WSL2.
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		echo "Warning: we are on macOS and we will make Windows binaries without code signing."; \
		echo ""; \
	elif [ -z "`grep -i WSL2 /proc/version`" ]; then \
		echo "Warning: we are on non-WSL2 Linux and we will make Windows binaries without code signing."; \
		echo ""; \
	fi
	@echo "Going to build a main release file and upload it."
	@cd build && \
	./do-release.sh && \
	cd ..

# Make Kirara release files and upload them.
do-release-kirara:
	@# Check if we are running on WSL2.
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		echo "Warning: we are on macOS and we will make Windows binaries without code signing."; \
		echo ""; \
	elif [ -z "`grep -i WSL2 /proc/version`" ]; then \
		echo "Warning: we are on non-WSL2 Linux and we will make Windows binaries without code signing."; \
		echo ""; \
	fi
	@echo "Going to build Kirara release files and upload them."
	@cd build && \
	./do-release-kirara.sh && \
	cd ..

# Update template games.
do-release-templates:
	@echo "Going to update template games."
	@cd build && \
	./do-release-templates.sh && \
	cd ..

##
## POSIX Conventions
##

# Build for Linux
build: linux-shared linux-pro-shared

# Install for Linux
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

# Cleanup.
clean:
	rm -f suika.exe suika-pro.exe suika-64.exe suika-arm64.exe suika-capture.exe suika-replay.exe
	rm -f mac.dmg mac-pro.dmg mac-capture.dmg mac-replay.dmg mac.zip pack.mac
	rm -f suika-linux suika-pro suika-linux-capture suika-linux-replay
