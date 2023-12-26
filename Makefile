DESTDIR=/usr/local

build: suika-linux suika-pro

suika-linux:
	@cd build/engine-linux && \
		make -f Makefile.shared -j8 && \
		make -f Makefile.shared install && \
		cd ../..

suika-pro:
	@cd build/pro-linux && \
		./make-deps-shared.sh && \
		rm -rf build && \
		mkdir build && \
		cd build && \
		cmake .. && \
		make && \
		cp suika-pro ../../../ && \
		cd ../../..

install: build
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
	@install -v suika-pro $(DESTDIR)/bin
	@install -v build/pro-linux/suika2 $(DESTDIR)/bin
	@install -v -t $(DESTDIR)/share/suika2/game/anime games/english/anime/*
	@install -v -t $(DESTDIR)/share/suika2/game/bg games/english/bg/*
	@install -v -t $(DESTDIR)/share/suika2/game/bgm games/english/bgm/*
	@install -v -t $(DESTDIR)/share/suika2/game/ch games/english/ch/*
	@install -v -t $(DESTDIR)/share/suika2/game/cg games/english/cg/*
	@install -v -t $(DESTDIR)/share/suika2/game/conf games/english/conf/*
	@install -v -t $(DESTDIR)/share/suika2/game/cv games/english/cv/*
	@install -v -t $(DESTDIR)/share/suika2/game/gui games/english/gui/*
	@install -v -t $(DESTDIR)/share/suika2/game/txt games/english/txt/*
	@install -v -t $(DESTDIR)/share/suika2/game/font games/english/font/*
	@install -v -t $(DESTDIR)/share/suika2/game/rule games/english/rule/*
	@install -v -t $(DESTDIR)/share/suika2/game/se games/english/se/*
	@install -v -t $(DESTDIR)/share/suika2/game/wms games/english/wms/*

clean:
	rm -f suika-linux suika-pro

##
## dev internal
##

do-release:
	@cd build && ./scripts/do-release.sh && cd ..

setup:
	@# For macOS:
	@if [ ! -z "`uname | grep Darwin`" ]; then \
		if [ -z  "`which brew`" ]; then \
			echo 'Installing Homebrew...'; \
			/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"; \
		fi; \
		echo 'Installing build tools...'; \
		brew install mingw-w64 gsed coreutils gsed wget makensis create-dmg; \
		echo "Building libraries..."; \
		cd build/engine-windows && ./build-libs.sh && cd ../..; \
		cp -Ra build/engine-windows/libroot build/pro-windows/; \
	fi
	@# For Linux:
	@if [ ! -z "`uname | grep Linux`" ]; then \
		echo 'Installing dependencies...'; \
		sudo apt-get update; \
		sudo apt-get install build-essential libasound2-dev libx11-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libxpm-dev mesa-common-dev zlib1g-dev libpng-dev libjpeg-dev libwebp-dev libbz2-dev libogg-dev libvorbis-dev libfreetype-dev cmake qt6-base-dev qt6-multimedia-dev libqt6core6 libqt6gui6 libqt6widgets6 libqt6opengl6-dev libqt6openglwidgets6 libqt6multimedia6 libqt6multimediawidgets6 mingw-w64; \
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

pro-windows:
	cd build/pro-windows && make && cd ../..

engine-macos:
	cd build/engine-macos && make && cd ../..

pro-macos:
	cd build/pro-macos && make && cd ../..

engine-wasm:
	cd build/engine-wasm  && make && cd ../..

engine-linux:
	cd build/engine-linux  && make && cd ../..

pro-linux:
	cd build/engine-linux  && make && cd ../..
