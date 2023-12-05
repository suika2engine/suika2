#
# Change this line for your signature.
#
SIGNATURE='Developer ID Application: Keiichi TABATA'

#
# Build release main app
#
main: libroot
	@echo "Building..."
	@echo "Builing suika.app" && xcodebuild -quiet -scheme suika -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika.xcarchive archive
	@echo "Uploading suika.app" && xcodebuild -quiet -exportArchive -archivePath `pwd`/build/Release/suika.xcarchive -exportOptionsPlist export-options.plist
	@echo "Notarizing suika.app" && rm -rf build/Release/suika.app && until xcodebuild -quiet -exportNotarizedApp -archivePath `pwd`/build/Release/suika.xcarchive -exportPath `pwd`/build/Release; do echo "Sleep..."; sleep 10; done
	@echo "Creating dmg files..."
	@rm -rf tmp mac.dmg && mkdir tmp && cp -Ra build/Release/suika.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac mac.dmg && codesign --sign $(SIGNATURE) mac.dmg && rm -rf tmp

#
# This builds "libroot" directory.
#
libroot:
	./build-libs.sh

#
# Build check only
#
build-only:
	@xcodebuild -quiet -scheme suika -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika.xcarchive build

#
# This phony target cleans up this directory.
#
clean:
	@rm -rf build mac.dmg libroot-mac.tar.gz libroot

