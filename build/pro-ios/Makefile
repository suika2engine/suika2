include ../common.mk

FILES_IOS= \
	pro-ios/main.m \
	pro-ios/AppDelegate.h \
	pro-ios/AppDelegate.m \
	pro-ios/SceneDelegate.h \
	pro-ios/SceneDelegate.m \
	pro-ios/ViewController.h \
	pro-ios/ViewController.m \
	pro-ios/GameView.h \
	pro-ios/GameView.m \
	pro-ios/ScriptTextView.h \
	pro-ios/ScriptTextView.m \
	../../src/apple/aunit.h \
	../../src/apple/aunit.c \
	../../src/apple/GameRenderer.h \
	../../src/apple/GameShaderTypes.h \
	../../src/apple/GameViewControllerProtocol.h \
	../../src/apple/GameRenderer.m \
	../../src/apple/GameShaders.metal

build-only: $(SRCS_MAIN) $(HDRS_MAIN) $(FILES_IOS)
	@curl -O https://suika2.com/dl/libroot-ios.tar.gz
	@tar xzf libroot-ios.tar.gz
	@if [ ! -z "`uname -a | grep arm64`" ]; then \
	    xcodebuild -target "Suika2 Pro Mobile" -project pro-ios.xcodeproj -configuration Debug build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO CODE_SIGNING_ALLOWED=NO;
	else \
		echo 'We cannot build on a x86_64 host.'; \
	fi
