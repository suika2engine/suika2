include ../common.mk

FILES_IOS= \
	engine-ios/main.m \
	engine-ios/AppDelegate.h \
	engine-ios/AppDelegate.m \
	engine-ios/SceneDelegate.h \
	engine-ios/SceneDelegate.m \
	engine-ios/ViewController.h \
	engine-ios/ViewController.m \
	engine-ios/GameView.h \
	engine-ios/GameView.m \
	../../src/apple/aunit.h \
	../../src/apple/aunit.c \
	../../src/apple/GameRenderer.h \
	../../src/apple/GameShaderTypes.h \
	../../src/apple/GameViewControllerProtocol.h \
	../../src/apple/GameRenderer.m \
	../../src/apple/GameShaders.metal

build-only: libroot $(SRCS_MAIN) $(HDRS_MAIN) $(FILES_MACOS)
	@xcodebuild -quiet -target Suika2 -project engine-ios.xcodeproj -configuration Debug build

libroot:
	./build-libs.sh
