#!/bin/sh

rm -rf .gradle apps/build
./prepare-libs.sh
ANDROID_SDK_ROOT=/sdk/cmdline-tools/latest ./gradlew build --stacktrace
cp app/build/outputs/apk/debug/app-debug.apk suika.apk
