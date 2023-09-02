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
	@rm -rf build mac.dmg mac-pro.dmg mac-capture.dmg mac-replay.dmg ../../mac.zip ../../pack.mac
	@echo "Builing suika.app" && security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -scheme suika -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika.xcarchive archive && security lock-keychain login.keychain
	@echo "Uploading suika.app" && security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -exportArchive -archivePath `pwd`/build/Release/suika.xcarchive -exportOptionsPlist export-options.plist && security lock-keychain login.keychain
	@echo "Notarizing suika.app" && rm -rf build/Release/suika.app && until security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -exportNotarizedApp -archivePath `pwd`/build/Release/suika.xcarchive -exportPath `pwd`/build/Release && security lock-keychain login.keychain; do echo "Sleep..."; sleep 10; done
	@echo "Builing suika-pro.app" && security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -scheme suika-pro -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika-pro.xcarchive archive && security lock-keychain login.keychain
	@echo "Uploading suika-pro.app" && security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -exportArchive -archivePath `pwd`/build/Release/suika-pro.xcarchive -exportOptionsPlist export-options.plist && security lock-keychain login.keychain
	@echo "Notarizing suika-pro.app" && rm -rf build/Release/suika-pro.app && until security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && xcodebuild -exportNotarizedApp -archivePath `pwd`/build/Release/suika-pro.xcarchive -exportPath `pwd`/build/Release && security lock-keychain login.keychain; do echo "Sleep..."; sleep 10; done
	@echo "Creating dmg files..."
	@rm -rf tmp mac.dmg && mkdir tmp && cp -Ra build/Release/suika.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac mac.dmg && security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && codesign --sign $(SIGNATURE) mac.dmg && security lock-keychain login.keychain && rm -rf tmp
	@rm -rf tmp mac-pro.dmg && mkdir tmp && cp -Ra build/Release/suika-pro.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac-pro mac-pro.dmg && security unlock-keychain -p "$(MACOS_PASSWORD)" login.keychain && codesign --sign $(SIGNATURE) mac-pro.dmg && security lock-keychain login.keychain && rm -rf tmp
	@mv mac.dmg mac-pro.dmg ../../
	@echo "Creating a zip file for use with Kirara..."
	@rm -rf zip mac.zip && mkdir zip
	@cp -Rv build/Release/suika.app zip/
	@cp -Rv build/Release/suika-pro.app zip/
	@cd zip && zip -r ../../../mac.zip . && cd .. && rm -rf zip
	@echo "Building pack.mac for use with Kirara..."
	@cd ../../tools/pack && make pack.mac && cp pack.mac ../../ && cd ../../build/macos

#
# This phony target copies dmg files to the project root.
#
install:
	cp mac.dmg mac-pro.dmg mac-capture.dmg mac-replay.dmg pack.mac ../../

#
# This phony target cleans up this directory.
#
clean:
	rm -rf build mac.dmg mac-pro.dmg mac-capture.dmg mac-replay.dmg tmp zip mac.zip pack.mac ../../mac.zip ../../pack.mac
