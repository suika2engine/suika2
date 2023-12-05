#
# Change this line for your signature.
#
SIGNATURE='Developer ID Application: Keiichi TABATA'

#
# Build suika-capture.app
#
capture: libroot
	@echo "Building..."
	@rm -rf build mac-capture.dmg
	@echo "Builing suika-capture.app" && xcodebuild -quiet -scheme suika-capture -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika-capture.xcarchive archive
	@echo "Uploading suika-capture.app" && xcodebuild -quiet -exportArchive -archivePath `pwd`/build/Release/suika-capture.xcarchive -exportOptionsPlist export-options.plist
	@echo "Notarizing suika-capture.app" && rm -rf build/Release/suika-capture.app && until xcodebuild -quiet -exportNotarizedApp -archivePath `pwd`/build/Release/suika-capture.xcarchive -exportPath `pwd`/build/Release; do echo "Sleep..."; sleep 10; done
	@echo "Creating suika-capture.dmg" && rm -rf tmp mac-capture.dmg && mkdir tmp && cp -Ra build/Release/suika-capture.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac-capture mac-capture.dmg && codesign --sign $(SIGNATURE) mac-capture.dmg && rm -rf tmp

#
# This builds "libroot" directory.
#
libroot:
	./build-libs.sh

#
# Build check only
#
build-only:
	@xcodebuild -quiet -scheme suika-capture -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika-capture.xcarchive build

#
# This phony target cleans up this directory.
#
clean:
	@rm -rf build mac-capture.dmg libroot-mac.tar.gz libroot
