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

test: replay
	rm -rf testcases && \
	git clone https://github.com/suika2engine/testcases.git && \
	cd testcases && \
	xvfb-run --server-args=":99 -screen 0 1920x1080x24" ./run.sh

replay: build/linux-x86_64-replay/suika-replay
	cd build/linux-x86_64-replay && \
	./build-libs.sh && \
	make

build/linux-x86_64-replay/suika-replay:

#
# The following targets have to be executed on macOS device.
#

mac:
	cd build/macos && \
	make suika

mac-pro:
	cd build/macos && \
	make suika-pro
