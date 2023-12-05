#
# Change this line for your signature.
#
SIGNATURE='Developer ID Application: Keiichi TABATA'

#
# Build suika-replay.app
#
replay: libroot
	@echo "Building..."
	@rm -rf build mac-replay.dmg
	@echo "Builing suika-replay.app" && xcodebuild -quiet -scheme suika-replay -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika-replay.xcarchive archive
	@echo "Uploading suika-replay.app" && xcodebuild -quiet -exportArchive -archivePath `pwd`/build/Release/suika-replay.xcarchive -exportOptionsPlist export-options.plist
	@echo "Notarizing suika-replay.app" && rm -rf build/Release/suika-replay.app && until xcodebuild -quiet -exportNotarizedApp -archivePath `pwd`/build/Release/suika-replay.xcarchive -exportPath `pwd`/build/Release; do echo "Sleep..."; sleep 10; done
	@echo "Creating suika-replay.dmg" && rm -rf tmp mac-replay.dmg && mkdir tmp && cp -Ra build/Release/suika-replay.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac-replay mac-replay.dmg && codesign --sign $(SIGNATURE) mac-replay.dmg && rm -rf tmp

#
# This builds "libroot" directory.
#
libroot:
	./build-libs.sh

#
# Build check only
#
build-only:
	@xcodebuild -quiet -scheme suika-replay -project suika.xcodeproj -configuration Release -archivePath `pwd`/build/Release/suika-replay.xcarchive build

#
# This phony target cleans up this directory.
#
clean:
	@rm -rf build mac-replay.dmg libroot-mac.tar.gz libroot
