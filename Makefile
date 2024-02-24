DESTDIR=/usr/local

build: suika-linux suika-pro

suika-linux:
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		echo 'Run on Linux.'; \
		exit 1; \
	fi;
	@cd build/engine-linux && \
		make -f Makefile.linux -j8 && \
		make -f Makefile.linux install && \
		cd ../..

suika-pro:
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		echo 'Run on Linux.'; \
		exit 1; \
	fi;
	@cd build/pro-linux && \
		./make-deps.sh && \
		rm -rf build && \
		mkdir build && \
		cd build && \
		cmake .. && \
		make && \
		cp suika-pro ../../../ && \
		cd ../../..
	@cd build/engine-ios && \
		make src && \
		cd ../../
	@cd build/engine-android && \
		make src && \
		cd ../../

install: build
	@install -v -d $(DESTDIR)/bin
	@install -v suika-linux $(DESTDIR)/bin/suika-runtime
	@install -v suika-pro $(DESTDIR)/bin/suika2

	@install -v -d $(DESTDIR)/share
	@install -v -d $(DESTDIR)/share/suika2

	@install -v -d $(DESTDIR)/share/suika2/export-linux
	@install -v suika-linux $(DESTDIR)/share/suika2/export-linux/suika-runtime

	@install -v -d $(DESTDIR)/share/suika2/export-ios
	@cd build/engine-ios/ios-src && find . -type d -exec install -v -d "$(DESTDIR)/share/suika2/export-ios/{}" ';' && cd ../../..
	@cd build/engine-ios/ios-src && find . -type f -exec install -v "{}" "$(DESTDIR)/share/suika2/export-ios/{}" ';' && cd ../../..

	@install -v -d $(DESTDIR)/share/suika2/export-android
	@cd build/engine-android/android-src && find . -type d -exec install -v -d "$(DESTDIR)/share/suika2/export-android/{}" ';' && cd ../../..
	@cd build/engine-android/android-src && find . -type f -exec install -v "{}" "$(DESTDIR)/share/suika2/export-android/{}" ';' && cd ../../..

	@install -v -d $(DESTDIR)/share/suika2/export-web
	@install -v build/engine-wasm/html/index.html $(DESTDIR)/share/suika2/export-web
	@install -v build/engine-wasm/html/index.js $(DESTDIR)/share/suika2/export-web
	@install -v build/engine-wasm/html/index.wasm $(DESTDIR)/share/suika2/export-web

	@install -v -d $(DESTDIR)/share/suika2/english-adv
	@cd games/english && find . -type d -exec install -v -d "$(DESTDIR)/share/suika2/english-adv/{}" ';' && cd ../..
	@cd games/english && find . -type f -exec install -v "{}" "$(DESTDIR)/share/suika2/english-adv/{}" ';' && cd ../..

	@install -v -d $(DESTDIR)/share/suika2/english-nvl
	@cd games/nvl-en && find . -type d -exec install -v -d "$(DESTDIR)/share/suika2/english-nvl/{}" ';' && cd ../..
	@cd games/nvl-en && find . -type f -exec install -v "{}" "$(DESTDIR)/share/suika2/english-nvl/{}" ';' && cd ../..

	@install -v -d $(DESTDIR)/share/suika2/japanese-adv
	@cd games/japanese && find . -type d -exec install -v -d "$(DESTDIR)/share/suika2/japanese-adv/{}" ';' && cd ../..
	@cd games/japanese && find . -type f -exec install -v "{}" "$(DESTDIR)/share/suika2/japanese-adv/{}" ';' && cd ../..

	@install -v -d $(DESTDIR)/share/suika2/japanese-nvl
	@cd games/nvl && find . -type d -exec install -v -d "$(DESTDIR)/share/suika2/japanese-nvl/{}" ';' && cd ../..
	@cd games/nvl && find . -type f -exec install -v "$(DESTDIR)/share/suika2/japanese-nvl/{}" "{}" ';' && cd ../..

	@install -v -d $(DESTDIR)/share/suika2/japanese-nvl-vertical
	@cd games/nvl-tategaki && find . -type d -exec install -v -d "$(DESTDIR)/share/suika2/japanese-nvl-vertical/{}" ';' && cd ../..
	@cd games/nvl-tategaki && find . -type f -exec install -v "{}" "$(DESTDIR)/share/suika2/japanese-nvl-vertical/{}" ';' && cd ../..

clean:
	rm -f suika-linux suika-pro

##
## dev internal
##

do-release:
	@cd build && ./do-release.sh && cd ..

setup:
	@# For macOS:
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		if [ -z  "`which brew`" ]; then \
			echo 'Installing Homebrew...'; \
			/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"; \
		fi; \
		echo 'Installing build tools...'; \
		brew install mingw-w64 gsed cmake coreutils gsed wget makensis create-dmg; \
		echo "Building libraries..."; \
		cd build/engine-windows && ./build-libs.sh && cd ../..; \
		cp -Ra build/engine-windows/libroot build/pro-windows/; \
	fi
	@# For Linux:
	@if [ ! -z "`uname | grep Linux`" ]; then \
		echo 'Installing dependencies...'; \
		sudo apt-get update; \
		sudo apt-get install build-essential cmake libasound2-dev libx11-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libxpm-dev mesa-common-dev zlib1g-dev libpng-dev libjpeg-dev libwebp-dev libbz2-dev libogg-dev libvorbis-dev libfreetype-dev cmake qt6-base-dev qt6-multimedia-dev libqt6core6 libqt6gui6 libqt6widgets6 libqt6opengl6-dev libqt6openglwidgets6 libqt6multimedia6 libqt6multimediawidgets6 mingw-w64; \
	fi
	@# For WSL2:
	@if [ ! -z "`uname | grep WSL2`" ]; then \
		echo "Disabling EXE file execution."; \
		echo 0 | sudo tee /proc/sys/fs/binfmt_misc/WSLInterop; \
		cd build/engine-windows && ./build-libs.sh && cd ../..; \
		cp -Ra build/engine-windows/libroot build/pro-windows/; \
		echo "Re-enabling EXE file execution."; \
		echo 1 | sudo tee /proc/sys/fs/binfmt_misc/WSLInterop; \
	fi

engine-windows:
	cd build/engine-windows && make && cd ../..

engine-windows-64:
	cd build/engine-windows-64 && make && cd ../..

engine-windows-arm64:
	cd build/engine-windows-arm64 && make && cd ../..

pro-windows:
	cd build/pro-windows && make && cd ../..

engine-macos:
	cd build/engine-macos && make && cd ../..

pro-macos:
	cd build/pro-macos && make && cd ../..

engine-wasm:
	cd build/engine-wasm  && make && cd ../..

pro-wasm:
	cd build/pro-wasm  && make && cd ../..

engine-linux: suika-linux

pro-linux: suika-pro
