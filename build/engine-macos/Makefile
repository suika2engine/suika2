include ../common.mk

FILES_ENGINE_MACOS = \
	engine-macos/main.m \
	engine-macos/AppDelegate.h \
	engine-macos/AppDelegate.m \
	engine-macos/ViewController.h \
	engine-macos/ViewController.m \
	engine-macos/GameView.h \
	engine-macos/GameView.m

main: suika-mac.zip

suika-mac.zip: libroot $(SRCS_MAIN) $(HDRS_MAIN) $(HDRS_APPLE) $(SRCS_APPLE) $(FILES_ENGINE_MACOS)
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
	@rm -f suika-mac.zip
	@cd build/Release && zip -r ../../suika-mac.zip Suika.app && cd ../..

build-only: libroot
	@xcodebuild -target Suika -project engine-macos.xcodeproj -configuration Debug build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO CODE_SIGNING_ALLOWED=NO

libroot:
	./build-libs.sh

clean:
	@rm -rf build suika-mac.zip libroot-mac.tar.gz libroot
