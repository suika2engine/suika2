#!/bin/sh

export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64

# Extract the libraries and copy the Suika2 files into the project tree.
./prepare-libs.sh

echo Downloading Android SDK command line tools.
wget https://dl.google.com/android/repository/commandlinetools-linux-10406996_latest.zip

echo Extracting SDK.
unzip commandlinetools-linux-10406996_latest.zip

unset ANDROID_HOME

export ANDROID_SDK_ROOT=`pwd`/cmdline-tools
yes | ./cmdline-tools/bin/sdkmanager "cmdline-tools;latest" --sdk_root=$ANDROID_SDK_ROOT

export ANDROID_SDK_ROOT=`pwd`/cmdline-tools/latest
yes | ./cmdline-tools/bin/sdkmanager --licenses --sdk_root=$ANDROID_SDK_ROOT

./gradlew build

cp build/android/app/build/outputs/apk/debug/app-debug.apk suika.apk
