#
# Change this line for your signature.
#
SIGNATURE='Developer ID Application: Keiichi TABATA'

#
# Build the suika-pro.app
#
pro: libroot
	@echo "Building..."
	@echo "Builing suika-pro.app" && xcodebuild -quiet -scheme suika-pro -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika-pro.xcarchive archive
	@echo "Uploading suika-pro.app" && xcodebuild -quiet -exportArchive -archivePath `pwd`/build/Release/suika-pro.xcarchive -exportOptionsPlist export-options.plist
	@echo "Notarizing suika-pro.app" && rm -rf build/Release/suika-pro.app && until xcodebuild -quiet -exportNotarizedApp -archivePath `pwd`/build/Release/suika-pro.xcarchive -exportPath `pwd`/build/Release; do echo "Sleep..."; sleep 10; done
	@echo "Creating dmg files..."
	@rm -rf tmp mac-pro.dmg && mkdir tmp && cp -Ra build/Release/suika-pro.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac-pro mac-pro.dmg && codesign --sign $(SIGNATURE) mac-pro.dmg && rm -rf tmp

#
# This builds "libroot" directory.
#
libroot:
	./build-libs.sh

#
# Build check only
#
build-only:
	@xcodebuild -quiet -scheme suika-pro -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika-pro.xcarchive build

#
# This phony target cleans up this directory.
#
clean:
	@rm -rf build mac-pro.dmg libroot-mac.tar.gz libroot
