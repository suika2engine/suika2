#
# Change this line for your signature.
#
SIGNATURE='Developer ID Application: Keiichi TABATA'

#
# Build release main app
#
main: libroot
	@echo "Building..."
	@echo "Builing Engine" && xcodebuild -quiet -scheme engine-macos -project engine-macos.xcodeproj -configuration Release -archivePath `pwd`/build/Release/engine-macos.xcarchive archive
	@echo "Uploading Engine" && xcodebuild -quiet -exportArchive -archivePath `pwd`/build/Release/engine-macos.xcarchive -exportOptionsPlist export-options.plist
	@echo "Notarizing Engine" && rm -rf build/Release/engine-macos.app && until xcodebuild -quiet -exportNotarizedApp -archivePath `pwd`/build/Release/engine-macos.xcarchive -exportPath `pwd`/build/Release; do echo "Sleep..."; sleep 10; done
	@echo "Creating DMG for Engine"
	@rm -rf tmp suika.dmg && mkdir tmp && cp -Ra build/Release/engine-macos.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika2 suika.dmg && codesign --sign $(SIGNATURE) suika.dmg && rm -rf tmp

#
# This builds "libroot" directory.
#
libroot:
	./build-libs.sh

#
# Build check only
#
build-only:
	@xcodebuild -quiet -scheme engine-macos -project engine-macos.xcodeproj -configuration Release -archivePath `pwd`/build/Release/engine-macos.xcarchive build

#
# This phony target cleans up this directory.
#
clean:
	@rm -rf build suika.dmg libroot-mac.tar.gz libroot
