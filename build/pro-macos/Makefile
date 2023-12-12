#
# Change this line for your signature.
#
SIGNATURE='Developer ID Application: Keiichi TABATA'

#
# Build release main app
#
main: libroot
	@echo "Building..."
	@echo "Builing Pro" && xcodebuild -quiet -scheme pro-macos -project pro-macos.xcodeproj -configuration Release -archivePath `pwd`/build/Release/pro-macos.xcarchive archive
	@echo "Uploading Pro" && xcodebuild -quiet -exportArchive -archivePath `pwd`/build/Release/pro-macos.xcarchive -exportOptionsPlist export-options.plist
	@echo "Notarizing Pro" && rm -rf build/Release/pro-macos.app && until xcodebuild -quiet -exportNotarizedApp -archivePath `pwd`/build/Release/pro-macos.xcarchive -exportPath `pwd`/build/Release; do echo "Sleep..."; sleep 10; done
	@echo "Creating DMG for Pro"
	@rm -rf tmp suika2.dmg && mkdir tmp && cp -Ra build/Release/pro-macos.app tmp/ && hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika2 suika2.dmg && codesign --sign $(SIGNATURE) suika2.dmg && rm -rf tmp

#
# This builds "libroot" directory.
#
libroot:
	./build-libs.sh

#
# Build check only
#
build-only:
	@xcodebuild -quiet -scheme pro-macos -project pro-macos.xcodeproj -configuration Release -archivePath `pwd`/build/Release/pro-macos.xcarchive build

#
# This phony target cleans up this directory.
#
clean:
	@rm -rf build suika2.dmg libroot-mac.tar.gz libroot
