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

#
# The following targets have to be executed on macOS device.
#

mac:
	cd build/macos && \
	make suika

mac-pro:
	cd build/macos && \
	make suika-pro
