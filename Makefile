# This will show the usage of this Makefile.
targets:
	@echo 'Welcome to Suika2!'
	@echo 'This is the build system of Suika2.'
	@echo
	@echo 'You can type the following commands:'
	@echo '  make setup             ... install dependency tools'
	@echo '  make windows           ... build the main game engine for Windows'
	@echo '  make macos             ... build the main game engine for macOS'
	@echo '  make linux             ... build the main game engine for Linux'
	@echo '  make all-windows       ... build all Windows binaries'
	@echo '  make all-macos         ... build all macOS binaries'
	@echo '  make all-linux         ... build all Linux binaries'
	@echo '  make test              ... run tests without a window'
	@echo '  make gtest             ... run tests with a window (just for demo)'
	@echo '  make clean             ... cleanup'
	@echo '  make do-release-13     ... build v13 release files and upload them (dev internal)'
	@echo '  make do-release-kirara ... build Kirara release files and upload them (dev internal)'
	@echo ''
	@# Check for a situation that we are on WSL2 and not under /mnt
	@if [ ! -z "`uname | grep Linux`" ]; then \
		if [ -z "`grep -i WSL2 /proc/version`" ]; then \
			case `pwd` in \
			/mnt/*)\
				echo 'Warning: You are under /mnt. Make sure to work on ~/ to avoid having Windows Security erase generated objects.'; \
				echo ; \
			esac; \
		fi; \
	fi

# This will setup the compilers and the tools.
setup:
	@# For Linux including WSL2.
	@if [ ! -z "`uname | grep Linux`" ]; then \
		if [ -z "`which apt-get`" ]; then \
			echo 'Error: Your system lacks "apt-get" command.'; \
			exit 1; \
		fi; \
		echo 'Are you sure you want to install the dependencies? (press enter)'; \
		read str; \
		echo Updating apt sources.; \
		echo sudo apt-get update; \
		sudo apt-get update; \
		echo Installing dependencies for the Windows targets.; \
		echo sudo apt-get install mingw-w64; \
		sudo apt-get install mingw-w64; \
		echo Installing dependencies for the Linux targets.; \
		echo sudo apt-get install build-essential libasound2-dev libx11-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libxpm-dev mesa-common-dev xvfb lcov; \
		sudo apt-get install build-essential libasound2-dev libx11-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libxpm-dev mesa-common-dev xvfb lcov; \
		echo Installing dependencies for the testing targets.; \
		echo sudo apt-get install python3-pip; \
		sudo apt-get install python3-pip; \
		echo pip3 install opencv-python numpy; \
		pip3 install opencv-python numpy; \
	fi
	@# For macOS
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		if [ -z  "`which brew`" ]; then \
			echo 'Error: Your system lacks "brew" command.'; \
			exit 1; \
		fi; \
		echo 'Are you sure you want to install the dependencies? (press enter)'; \
		read str; \
		brew install mingw-w64; \
	fi

# A target for all Windows binaries.
all-windows: windows windows-64 windows-arm64 windows-pro windows-capture windows-replay

# A target for all macOS binaries.
all-macos:
	@echo 'Building macOS binaries'
	@cd build/macos && \
		make clean && \
		make release && \
		make install && \
		make clean && \
	cd ../..

# A target for all Linux binaries.
all-linux: linux linux-capture linux-replay

# suika.exe (the main game engine for 32-bit Windows)
windows:
	@echo 'Building suika.exe'
	@cd build/mingw && \
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

# suika-pro.exe (the debugger)
windows-pro:
	@echo 'Building suika-pro.exe'
	@cd build/mingw-pro && \
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

# A target for the main game engine for macOS.
macos:
	@echo 'Building macOS binary'
	@cd build/macos && \
		make clean && \
		make suika.dmg && \
		cp suika.dmg ../../ && \
		make clean && \
	cd ../..

# suika-linux (the main game engine for 64-bit Linux)
linux:
	@echo 'Building a Linux game binary'
	@cd build/linux-x86_64 && \
	make libroot && \
	make -j8 && \
	make install && \
	cd ../..

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

# Build apps and upload both Japanese and English zip files.
do-release:
	@# Check if we are running on WSL2.
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		echo "Warning: we are on macOS and we will make Windows binaries without code signing."; \
		echo ""; \
	elif [ -z "`grep -i WSL2 /proc/version`" ]; then \
		echo "Warning: we are on non-WSL2 Linux and we will make Windows binaries without code signing."; \
		echo ""; \
	fi
	@echo "Going to build release files and upload them."
	@cd build && \
	./do-release.sh && \
	cd ..

# Build apps and upload both Japanese and English zip files.
do-release-kirara:
	@# Check if we are running on WSL2.
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		echo "Warning: we are on macOS and we will make Windows binaries without code signing."; \
		echo ""; \
	elif [ -z "`grep -i WSL2 /proc/version`" ]; then \
		echo "Warning: we are on non-WSL2 Linux and we will make Windows binaries without code signing."; \
		echo ""; \
	fi
	@echo "Going to build release files and upload them."
	@cd build && \
	./do-release-kirara.sh && \
	cd ..

# Cleanup.
clean:
	rm -f suika.exe suika-64.exe suika-arm64.exe suika-pro.exe suika-capture.exe suika-replay.exe
	rm -f mac.dmg mac-pro.dmg mac-capture.dmg mac-replay.dmg mac.zip pack.mac
	rm -f suika-linux suika-linux-capture suika-linux-replay
