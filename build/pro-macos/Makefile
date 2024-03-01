include ../common.mk

FILES_MACOS = \
	pro-macos/main.m \
	pro-macos/AppDelegate.h \
	pro-macos/AppDelegate.m \
	pro-macos/ViewController.h \
	pro-macos/ViewController.m \
	pro-macos/GameView.h \
	pro-macos/GameView.m \
	../../src/apple/aunit.h \
	../../src/apple/aunit.c \
	../../src/apple/GameRenderer.h \
	../../src/apple/GameShaderTypes.h \
	../../src/apple/GameViewControllerProtocol.h \
	../../src/apple/GameRenderer.m \
	../../src/apple/GameShaders.metal

main: suika2.dmg

suika2.dmg: libroot $(SRCS_MAIN) $(HDRS_MAIN) $(FILES_MACOS)
	@echo "Building Suika2 Pro Desktop.app"
	@rm -rf build
	@xcodebuild -quiet -scheme pro-macos -project pro-macos.xcodeproj -configuration Release -archivePath `pwd`/build/Release/pro-macos.xcarchive archive
	@xcodebuild -quiet -exportArchive -archivePath `pwd`/build/Release/pro-macos.xcarchive -exportOptionsPlist export-options.plist
	@rm -rf "build/Release/Suika2 Pro Desktop.app"
	@until \
		xcodebuild -quiet -exportNotarizedApp -archivePath `pwd`/build/Release/pro-macos.xcarchive -exportPath `pwd`/build/Release; \
	do \
		echo "waiting..."; \
		sleep 10; \
	done
	@rm -rf tmp *.dmg
	@mkdir tmp
	@cp -Ra "build/Release/Suika2 Pro Desktop.app" "tmp/Suika2 Pro Desktop.app"
	@create-dmg \
		--volname "Suika2 Pro Desktop Installer" \
		--background "installer_background.png" \
		--window-pos 200 120 \
		--window-size 800 400 \
		--icon-size 100 \
		--icon "Suika2 Pro Desktop.app" 200 190 \
		--app-drop-link 600 185 \
		--format ULFO \
		"suika2.dmg" \
		"tmp"
	@codesign --sign 'Developer ID Application: Keiichi TABATA' suika2.dmg

libroot:
	./build-libs.sh

build-only:
	@curl -O https://suika2.com/dl/libroot-mac.tar.gz
	@tar xzf libroot-mac.tar.gz
	@touch ../engine-windows/suika.exe
	@touch ../engine-macos/suika-mac.dmg
	@mkdir -p ../engine-ios/ios-src
	@mkdir -p ../engine-android/android-src
	@mkdir -p ../engine-wasm/html
	@touch ../engine-wasm/html/index.html
	@touch ../engine-wasm/html/index.js
	@touch ../engine-wasm/html/index.wasm
	@if [ ! -z "`uname -a | grep arm64`" ]; then \
		xcodebuild -target "Suika2 Pro Desktop" -project pro-macos.xcodeproj -configuration Debug -destination "generic/platform=macOS,arch=armv8" build ONLY_ACTIVE_ARCH=YES CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO CODE_SIGNING_ALLOWED=NO; \
	else \
		echo 'We cannot build on a x86_64 host.'; \
	fi

clean:
	@rm -rf build suika2.dmg libroot-mac.tar.gz libroot
