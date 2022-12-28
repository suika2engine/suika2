all: libroot

libroot:
	curl -O https://suika2.com/dl/libroot-mac.tar.gz
	tar xzf libroot-mac.tar.gz

suika: libroot
	xcodebuild -target suika -project suika.xcodeproj -configuration Debug build

suika-pro: libroot
	xcodebuild -target suika-pro -project suika.xcodeproj -configuration Debug build
