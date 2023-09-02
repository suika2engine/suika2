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
	@echo "Type one of the following commands:"
	@echo "  make suika.dmg"
	@echo "  make release"

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
# Build all apps for a release
#
release: libroot
	@echo "Building..."
	@security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -scheme suika -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika.xcarchive archive
	@security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -scheme suika-pro -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika-pro.xcarchive archive
	@security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -scheme suika-capture -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika-capture.xcarchive archive
	@security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -scheme suika-replay -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika-replay.xcarchive archive
	@echo "Uploading..."
	@security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -exportArchive -archivePath `pwd`/build/Release/suika.xcarchive -exportOptionsPlist export-options.plist
	@security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -exportArchive -archivePath `pwd`/build/Release/suika-pro.xcarchive -exportOptionsPlist export-options.plist
	@security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -exportArchive -archivePath `pwd`/build/Release/suika-capture.xcarchive -exportOptionsPlist export-options.plist
	@security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -exportArchive -archivePath `pwd`/build/Release/suika-replay.xcarchive -exportOptionsPlist export-options.plist
	@echo "Notarizing..."
	@rm -rf build/Release/suika.app build/Release/suika-pro.app build/Release/suika-capture.app build/Release/suika-replay.app
	@until security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -exportNotarizedApp -archivePath `pwd`/build/Release/suika.xcarchive -exportPath `pwd`/build/Release; do echo "Sleep..."; sleep 10; done
	@until security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -exportNotarizedApp -archivePath `pwd`/build/Release/suika-pro.xcarchive -exportPath `pwd`/build/Release; do echo "Sleep..."; sleep 10; done
	@until security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -exportNotarizedApp -archivePath `pwd`/build/Release/suika-capture.xcarchive -exportPath `pwd`/build/Release; do echo "Sleep..."; sleep 10; done
	@until security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -exportNotarizedApp -archivePath `pwd`/build/Release/suika-replay.xcarchive -exportPath `pwd`/build/Release; do echo "Sleep..."; sleep 10; done
	@echo "Creating dmg files..."
	@rm -rf tmp mac.dmg && mkdir tmp && cp -Rv build/Release/suika.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac mac.dmg && security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && codesign --sign $(SIGNATURE) mac.dmg && rm -rf tmp
	@rm -rf tmp mac-pro.dmg && mkdir tmp && cp -Rv build/Release/suika-pro.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac-pro mac-pro.dmg && security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && codesign --sign $(SIGNATURE) mac-pro.dmg && rm -rf tmp
	@rm -rf tmp mac-capture.dmg && mkdir tmp && cp -Rv build/Release/suika-capture.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac-capture mac-capture.dmg && security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && codesign --sign $(SIGNATURE) mac-capture.dmg && rm -rf tmp
	@rm -rf tmp mac-replay.dmg && mkdir tmp && cp -Rv build/Release/suika-replay.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac-replay mac-replay.dmg && security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && codesign --sign $(SIGNATURE) mac-replay.dmg && rm -rf tmp
	@echo "Creating a zip file for use with Kirara..."
	@rm -rf zip mac.zip && mkdir zip
	@cp -Rv build/Release/suika.app zip/
	@cp -Rv build/Release/suika-pro.app zip/
	@cd zip && zip -r ../mac.zip . && cd .. && rm -rf zip
	@echo "Building pack.mac for use with Kirara..."
	@cd ../../tools/pack && make pack.mac && cp pack.mac ../../build/macos/pack.mac && cd ../../build/macos

#
# This phony target copies dmg files to the project root.
#
install:
	cp mac.dmg mac-pro.dmg mac-capture.dmg mac-replay.dmg pack.mac ../../

#
# This phony target cleans up this directory.
#
clean:
	rm -rf mac.dmg mac-pro.dmg mac-capture.dmg mac-replay.dmg tmp zip mac.zip pack.mac
