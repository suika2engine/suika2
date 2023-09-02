#
# Change this line for your signature.
#
SIGNATURE='Developer ID Application: Keiichi TABATA'

#
# Password
#
MACOS_PASSWORD=`cat .passwd`

#
# Print the usage.
#
usage:
	@echo "Hello, this is a Makefile for macOS build."
	@echo ""
	@echo "Type one of the following commands:"
	@echo "  make suika.dmg ... build the main game engine"
	@echo "  make release   ... build apps for a release"
	@echo "  make cap-rep   ... build suika-capture.app and suika-replay.app"

#
# Build the main app only
#
suika.dmg: libroot
	@echo "Building..."
	@xcodebuild -scheme suika -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika.xcarchive archive
	@echo "Uploading..."
	@xcodebuild -exportArchive -archivePath `pwd`/build/Release/suika.xcarchive -exportOptionsPlist export-options.plist
	@echo "Waiting for notarization..."
	@rm -rf build/Release/suika.app
	@until \
		xcodebuild -exportNotarizedApp -archivePath `pwd`/build/Release/suika.xcarchive -exportPath `pwd`/build/Release; \
	do \
	    echo "Sleep..."; \
	    sleep 10; \
	done
	@rm -rf tmp mac.dmg && mkdir tmp && cp -Rv build/Release/suika.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac mac.dmg && codesign --sign $(SIGNATURE) mac.dmg && rm -rf tmp

#
# This builds "libroot" directory.
#
libroot:
	./build-libs.sh

#
# Build release apps
#
release: libroot
	@echo "Building..."
	@rm -rf build mac.dmg mac-pro.dmg mac.zip pack.mac
	@security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain
	@echo "Builing suika.app" && xcodebuild -scheme suika -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika.xcarchive archive
	@echo "Uploading suika.app" && xcodebuild -exportArchive -archivePath `pwd`/build/Release/suika.xcarchive -exportOptionsPlist export-options.plist
	@echo "Notarizing suika.app" && rm -rf build/Release/suika.app && until xcodebuild -exportNotarizedApp -archivePath `pwd`/build/Release/suika.xcarchive -exportPath `pwd`/build/Release; do echo "Sleep..."; sleep 10; done
	@echo "Builing suika-pro.app" && xcodebuild -scheme suika-pro -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika-pro.xcarchive archive
	@echo "Uploading suika-pro.app" && xcodebuild -exportArchive -archivePath `pwd`/build/Release/suika-pro.xcarchive -exportOptionsPlist export-options.plist
	@echo "Notarizing suika-pro.app" && rm -rf build/Release/suika-pro.app && until xcodebuild -exportNotarizedApp -archivePath `pwd`/build/Release/suika-pro.xcarchive -exportPath `pwd`/build/Release; do echo "Sleep..."; sleep 10; done
	@echo "Creating dmg files..."
	@rm -rf tmp mac.dmg && mkdir tmp && cp -Ra build/Release/suika.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac mac.dmg && codesign --sign $(SIGNATURE) mac.dmg && rm -rf tmp
	@rm -rf tmp mac-pro.dmg && mkdir tmp && cp -Ra build/Release/suika-pro.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac-pro mac-pro.dmg && codesign --sign $(SIGNATURE) mac-pro.dmg && rm -rf tmp
	@echo "Creating a zip file for use with Kirara..."
	@rm -rf zip mac.zip && mkdir zip
	@cp -Rv build/Release/suika.app zip/
	@cp -Rv build/Release/suika-pro.app zip/
	@cd zip && zip -r ../mac.zip . && cd .. && rm -rf zip
	@echo "Building pack.mac for use with Kirara..."
	@cd ../../tools/pack && make pack.mac && cp pack.mac ../../build/macos/ && cd ../../build/macos

#
# Build capture-and-replay apps
#
cap-rep: libroot
	@if [ -z "$SSH_CLIENT" ]; then echo "Cannot run in an ssh session due to a codesign issue."; exit 1; fi
	@echo "Building..."
	@rm -rf build mac-capture.dmg mac-replay.dmg
	@echo "Builing suika-capture.app" && xcodebuild -scheme suika-capture -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika-capture.xcarchive archive
	@echo "Uploading suika-capture.app" && xcodebuild -exportArchive -archivePath `pwd`/build/Release/suika-capture.xcarchive -exportOptionsPlist export-options.plist
	@echo "Notarizing suika-capture.app" && rm -rf build/Release/suika-capture.app && until xcodebuild -exportNotarizedApp -archivePath `pwd`/build/Release/suika-capture.xcarchive -exportPath `pwd`/build/Release; do echo "Sleep..."; sleep 10; done
	@echo "Creating suika-capture.dmg" && rm -rf tmp mac-capture.dmg && mkdir tmp && cp -Ra build/Release/suika-capture.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac-capture mac-capture.dmg && codesign --sign $(SIGNATURE) mac-capture.dmg && rm -rf tmp
	@echo "Builing suika-replay.app" && xcodebuild -scheme suika-replay -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika-replay.xcarchive archive
	@echo "Uploading suika-replay.app" && xcodebuild -exportArchive -archivePath `pwd`/build/Release/suika-replay.xcarchive -exportOptionsPlist export-options.plist
	@echo "Notarizing suika-replay.app" && rm -rf build/Release/suika-replay.app && until xcodebuild -exportNotarizedApp -archivePath `pwd`/build/Release/suika-replay.xcarchive -exportPath `pwd`/build/Release; do echo "Sleep..."; sleep 10; done
	@echo "Creating suika-replay.dmg" && rm -rf tmp mac-replay.dmg && mkdir tmp && cp -Ra build/Release/suika-replay.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac-replay mac-replay.dmg && codesign --sign $(SIGNATURE) mac-replay.dmg && rm -rf tmp

#
# This phony target copies dmg files to the project root.
#
install:
	@cp mac.dmg mac-pro.dmg mac.zip pack.mac ../../
	@if [ -e mac-capture.dmg ]; then cp mac-capture.dmg ../../; fi
	@if [ -e mac-replay.dmg ]; then cp mac-replay.dmg ../../; fi

#
# This phony target cleans up this directory.
#
clean:
	@rm -rf build mac.dmg mac-pro.dmg mac-capture.dmg mac-replay.dmg tmp zip mac.zip pack.mac
