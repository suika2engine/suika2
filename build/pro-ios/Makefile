build-only: $(SRCS_MAIN) $(HDRS_MAIN) $(FILES_MACOS)
	@curl -O https://suika2.com/dl/libroot-ios.tar.gz
	@tar xzf libroot-ios.tar.gz
	@xcodebuild -target "Suika2 Pro Mobile" -project pro-ios.xcodeproj -configuration Debug build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO CODE_SIGNING_ALLOWED=NO
