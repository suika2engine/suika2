targets:
	@echo 'Welcome to Suika2!'
	@echo 'This is the build system of Suika2 for use with WSL2.'
	@echo
	@echo 'You can type following commands:'
	@echo '  make setup           ... install the dependency libraries'
	@echo '  make windows         ... build suika.exe'
	@echo '  make windows-64      ... build suika-64.exe'
	@echo '  make windows-arm64   ... build suika-arm64.exe'
	@echo '  make windows-pro     ... build suika-pro.exe'
	@echo '  make windows-capture ... build suika-capture.exe'
	@echo '  make windows-replay  ... build suika-replay.exe'
	@echo '  make linux           ... build suika-linux'
	@echo '  make linux-replay    ... build suika-linux-replay'
	@echo '  make test            ... run test without a window'
	@echo '  make gtest           ... run test with a window'
	@echo '  make do-release      ... build release files and upload them (dev internal)'
	@echo

	@# Check if we are on WSL2.
	@if [ -z "`grep -i WSL2 /proc/version`" ]; then \
		# Check if we are not on /mnt \
		case `pwd` in \
		/mnt/*)\
			echo 'Warning: You are under /mnt. Make sure to work on ~/ to avoid having Windows Security erase generated objects.'; \
			echo ; \
		esac; \
	fi

setup:
	@if [ -z "`which apt-get`" ]; then \
		echo 'Error: Your system lacks "apt-get" command.'; \
		exit 1; \
	fi

	@echo 'Are you sure you want to install the dependencies? (press enter)'
	@read str

	@echo Updating apt sources.
	@echo sudo apt-get update
	@sudo apt-get update

	@echo Installing dependencies for the Windows targets.
	@echo sudo apt-get install mingw-w64
	@sudo apt-get install mingw-w64

	@echo Installing dependencies for the Linux targets.
	@echo sudo apt-get install build-essential libasound2-dev libx11-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libxpm-dev mesa-common-dev xvfb lcov
	@sudo apt-get install \
		build-essential libasound2-dev libx11-dev \
		libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
		libxpm-dev mesa-common-dev xvfb lcov

	@echo Installing dependencies for the testing targets.
	@echo sudo apt-get install python3-pip
	@sudo apt-get install python3-pip
	@echo pip3 install opencv-python numpy
	@pip3 install opencv-python numpy

windows:
	@echo 'Building suika.exe'
	@cd build/mingw && \
	make libroot && \
	make -j8 && \
	make install

windows-64:
	@echo 'Building suika-64.exe'
	@cd build/mingw-64 && \
	make libroot && \
	make -j8 && \
	make install

windows-arm64:
	@echo 'Building suika-arm64.exe'
	@cd build/mingw-arm64 && \
	make libroot && \
	make -j8 && \
	make install

windows-pro:
	@echo 'Building suika-pro.exe'
	@cd build/mingw-pro && \
	make libroot && \
	make -j8 && \
	make install

windows-capture:
	@echo 'Building suika-capture.exe'
	@cd build/mingw-capture && \
	make libroot && \
	make -j8 && \
	make install

windows-replay:
	@echo 'Building suika-replay.exe'
	@cd build/mingw-capture && \
	make libroot && \
	make -j8 && \
	make install

linux:
	@echo 'Building a Linux game binary'
	@cd build/linux-x86_64 && \
	make libroot && \
	make -j8 && \
	make install

linux-replay:
	@echo 'Building a Linux replay binary'
	@cd build/linux-x86_64-replay && \
	make libroot && \
	make -j8 && \
	make install

test:
	@echo 'Running non-graphical tests...'

	@# Build suika-replay if not exists.
	@if [ ! -f build/linux-x86_64-replay/suika-replay ]; then \
		cd build/linux-x86_64-replay; \
		./build-libs.sh; make; \
	else \
		# Remove existing profile data. \
		rm -f build/linux-x86_64-replay/*.gcda; \
		rm -rf build/linux-x86_64-replay/sav; \
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
	xvfb-run --server-args=":99 -screen 0 1920x1080x24" ./run.sh && \
	cd ..

	@# Calculate the coverage.
	@cd build/linux-x86_64-replay && \
	lcov -d . --rc lcov_branch_coverage=1 -c -o app.info && \
	sed -i s+`pwd`+`readlink -f ../../src`+g app.info && \
	lcov -r app.info -o app.info --rc lcov_branch_coverage=1 '/usr/include/*' && \
	lcov --summary --rc lcov_branch_coverage=1 app.info | tail -n +2 > ../../summary && \
	genhtml -o lcovoutput -p `pwd` --num-spaces 4 --rc lcov_branch_coverage=1 -f app.info

gtest:
	@echo 'Running graphical tests...'

	@# Build suika-replay if not exists.
	@if [ ! -f build/linux-x86_64-replay/suika-replay ]; then \
		cd build/linux-x86_64-replay; \
		./build-libs.sh; make; \
		cd ../../; \
	else \
		# Remove existing profile data. \
		rm -f build/linux-x86_64-replay/*.gcda; \
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

	@# Calculate the coverage.
	@cd build/linux-x86_64-replay && \
	lcov -d . --rc lcov_branch_coverage=1 -c -o app.info && \
	sed -i s+`pwd`+`readlink -f ../../src`+g app.info && \
	lcov -r app.info -o app.info --rc lcov_branch_coverage=1 '/usr/include/*' && \
	lcov --summary --rc lcov_branch_coverage=1 app.info | tail -n +2 > ../../summary && \
	genhtml -o lcovoutput -p `pwd` --num-spaces 4 --rc lcov_branch_coverage=1 -f app.info

do-release:
	@echo 'Building release zip files.'
	@cd build && \
	./do-release.sh
