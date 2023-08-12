targets:
	@echo 'Welcome to Suika2!'
	@echo 'This is the build system of Suika2 for use with WSL2.'
	@echo

	@# Check if we are on WSL2.
	@if [ -z "`grep -i WSL2 /proc/version`" ]; then \
		echo 'Error: You are not on WSL2.'; \
		exit 1; \
	fi;

	@# Check if we are not on /mnt
	@case `pwd` in \
	    /mnt/*) echo 'Error: You are under /mnt. Make sure work on ~/'; exit 1; \
	esac

	@# Print usage
	@echo 'You can type followwing commands:'
	@echo '  make setup           ... install dependency libraries'
	@echo '  make windows         ... build suika.exe'
	@echo '  make windows-64      ... build suika-64.exe'
	@echo '  make windows-arm64   ... build suika-arm64.exe'
	@echo '  make windows-pro     ... build suika-pro.exe'
	@echo '  make windows-capture ... build suika-capture.exe'
	@echo '  make linux           ... build suika-linux'
	@echo '  make linux-replay    ... build suika-linux-replay'
	@echo '  make test            ... run test without a window'
	@echo '  make gtest           ... run test with a window'
	@echo '  make do-release      ... build release files and upload them (dev internal)'
	@echo

setup:
	@sudo apt-get update
	@sudo apt-get install -y \
		mingw-w64 build-essential libasound2-dev libx11-dev \
		libxpm-dev mesa-common-dev xvfb lcov
	@pip3 install opencv-python numpy

windows:
	@cd build/mingw && \
	make libroot && \
	make -j8 && \
	make install

windows-64:
	@cd build/mingw-64 && \
	make libroot && \
	make -j8 && \
	make install

windows-arm64:
	@cd build/mingw-arm64 && \
	make libroot && \
	make -j8 && \
	make install

windows-pro:
	@cd build/mingw-pro && \
	make libroot && \
	make -j8 && \
	make install

windows-capture:
	@cd build/mingw-capture && \
	make libroot && \
	make -j8 && \
	make install

linux:
	@cd build/linux-x86_64 && \
	make libroot && \
	make -j8 && \
	make install

linux-replay:
	@cd build/linux-x86_64-replay && \
	make libroot && \
	make -j8 && \
	make install

test:
	@echo Running non-graphical test...

	@# Build suika-replay if not exists.
	@if [ ! -f build/linux-x86_64-replay/suika-replay ]; then \
		cd build/linux-x86_64-replay; \
		./build-libs.sh; make; \
		cd ../../; \
	else \
		# Remove existing profile data. \
		rm -f build/linux-x86_64-replay/*.gcda; \
	fi

	@# Fetch testcases repository.
	@if [ ! -d testcases ]; then \
		git clone https://github.com/suika2engine/testcases.git; \
	else \
		cd testcases; \
		git pull origin master; \
		cd ..; \
	fi

	@# Run testcases.
	@cd testcases && \
	xvfb-run --server-args=":99 -screen 0 1920x1080x24" ./run.sh && \
	cd ..

	@# Calculate coverage.
	@cd build/linux-x86_64-replay && \
	lcov -d . --rc lcov_branch_coverage=1 -c -o app.info && \
	sed -i s+`pwd`+`readlink -f ../../src`+g app.info && \
	lcov -r app.info -o app.info --rc lcov_branch_coverage=1 '/usr/include/*' && \
	lcov --summary --rc lcov_branch_coverage=1 app.info | tail -n +2 > ../../summary && \
	genhtml -o lcovoutput -p `pwd` --num-spaces 4 --rc lcov_branch_coverage=1 -f app.info

gtest:
	@echo Running graphical test...

	@# Build suika-replay if not exists.
	@if [ ! -f build/linux-x86_64-replay/suika-replay ]; then \
		cd build/linux-x86_64-replay; \
		./build-libs.sh; make; \
		cd ../../; \
	else \
		# Remove existing profile data. \
		rm -f build/linux-x86_64-replay/*.gcda; \
	fi

	@# Fetch testcases repository.
	@if [ ! -d testcases ]; then \
		git clone https://github.com/suika2engine/testcases.git; \
	else \
		cd testcases; \
		git pull origin master; \
		cd ..; \
	fi

	@# Run testcases.
	@cd testcases && \
	./run.sh && \
	cd ..

	@# Calculate coverage.
	@cd build/linux-x86_64-replay && \
	lcov -d . --rc lcov_branch_coverage=1 -c -o app.info && \
	sed -i s+`pwd`+`readlink -f ../../src`+g app.info && \
	lcov -r app.info -o app.info --rc lcov_branch_coverage=1 '/usr/include/*' && \
	lcov --summary --rc lcov_branch_coverage=1 app.info | tail -n +2 > ../../summary && \
	genhtml -o lcovoutput -p `pwd` --num-spaces 4 --rc lcov_branch_coverage=1 -f app.info

do-release:
	@cd build && \
	./do-release.sh
