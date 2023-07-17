targets:
	@echo Hello! Welcome to Suika2!
	@echo
	@echo Try \"make setup\" then \"make windows\" to build suika.exe on WSL2.
	@echo Make sure you are under WSL2 home, not /mnt/c.
	@echo
	@echo targets: setup windows windows-pro linux-x86 do-release
	@echo

setup:
	sudo apt-get install mingw-w64

windows:
	cd build/mingw && \
	make && \
	make install

windows-pro:
	cd build/mingw-pro && \
	make && \
	make install

do-release:
	cd build && \
	./do-release.sh

test:
	echo Running test...
	if [ ! -f build/linux-x86_64-replay/suika-replay ]; then cd build/linux-x86_64-replay; ./build-libs.sh; make; else rm -f build/linux-x86_64-replay/suika-replay/*.gcda; fi && \
	if [ ! -d testcases ]; then git clone https://github.com/suika2engine/testcases.git; else cd testcases; git pull origin master; cd ..; fi && \
	cd testcases && \
	xvfb-run --server-args=":99 -screen 0 1920x1080x24" ./run.sh && \
	cd ../build/linux-x86_64-replay && \
	echo C0 coverage: `gcov -n *.gcda | tail -n1 | grep -o '[0-9.]*%'` && \
	lcov -d . --rc lcov_branch_coverage=1 -c -o app.info && sed -i s+`pwd`+`readlink -f ../../src`+g app.info && lcov -r app.info -o app.info --rc lcov_branch_coverage=1 '/usr/include/*' && \
	lcov --summary --rc lcov_branch_coverage=1 app.info | tail -n +2 > ../../summary && \
	genhtml -o lcovoutput -p `pwd` --num-spaces 4 -f app.info

gtest:
	echo Running test...
	if [ ! -f build/linux-x86_64-replay/suika-replay ]; then cd build/linux-x86_64-replay; ./build-libs.sh; make; else rm -f build/linux-x86_64-replay/suika-replay/*.gcda; fi && \
	if [ ! -d testcases ]; then git clone https://github.com/suika2engine/testcases.git; else cd testcases; git pull origin master; cd ..; fi && \
	cd testcases && \
	./run.sh && \
	cd ../build/linux-x86_64-replay && \
	echo C0 coverage: `gcov -n *.gcda | tail -n1 | grep -o '[0-9.]*%'` && \
	lcov -d . --rc lcov_branch_coverage=1 -c -o app.info && sed -i s+`pwd`+`readlink -f ../../src`+g app.info && lcov -r app.info -o app.info --rc lcov_branch_coverage=1 '/usr/include/*' && \
	lcov --summary --rc lcov_branch_coverage=1 app.info | tail -n +2 > ../../summary && \
	genhtml -o lcovoutput -p `pwd` --num-spaces 4 -f app.info

#
# The following targets have to be executed on macOS device.
#

mac:
	cd build/macos && \
	make suika

mac-pro:
	cd build/macos && \
	make suika-pro
