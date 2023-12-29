include ../common.mk

FILES_APPLE = \
	engine-macos/main.m \
	engine-macos/AppDelegate.h \
	engine-macos/AppDelegate.m \
	engine-macos/ViewController.h \
	engine-macos/ViewController.m \
	engine-macos/GameView.h \
	engine-macos/GameView.m \
	../../src/apple/aunit.h \
	../../src/apple/aunit.c \
	../../src/apple/GameRenderer.h \
	../../src/apple/GameShaderTypes.h \
	../../src/apple/GameViewControllerProtocol.h \
	../../src/apple/GameRenderer.m \
	../../src/apple/GameShaders.metal

main: suika-mac.dmg

suika-mac.dmg: libroot $(SRCS_MAIN) $(HDRS_MAIN) $(FILES_APPLE)
	@echo "Building Suika.app"
	@xcodebuild -quiet -scheme engine-macos -project engine-macos.xcodeproj -configuration Release -archivePath `pwd`/build/Release/engine-macos.xcarchive archive
	@xcodebuild -quiet -exportArchive -archivePath `pwd`/build/Release/engine-macos.xcarchive -exportOptionsPlist export-options.plist
	@rm -rf build/Release/Suika.app
	@until \
		xcodebuild -quiet -exportNotarizedApp -archivePath `pwd`/build/Release/engine-macos.xcarchive -exportPath `pwd`/build/Release; \
	do \
		echo "Waiting..."; \
		sleep 10; \
	done
	@rm -f suika-mac.dmg
	@mkdir tmp
	@cp -Rv build/Release/Suika.app tmp/
	@hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac suika-mac.dmg
	@rm -rf tmp

build-only: libroot
	@xcodebuild -target Suika -project engine-macos.xcodeproj -configuration Debug build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO CODE_SIGNING_ALLOWED=NO

libroot:
	./build-libs.sh

clean:
	@rm -rf build suika-mac.dmg libroot-mac.tar.gz libroot
