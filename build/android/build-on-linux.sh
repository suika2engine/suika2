#!/bin/sh

sudo apt-get update
sudo apt-get install -y openjdk-17-jdk-headless

export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64

./prepare-libs.sh

echo Downloading Android SDK command line tools.
wget https://dl.google.com/android/repository/commandlinetools-linux-10406996_latest.zip

echo Extracting SDK.
unzip commandlinetools-linux-10406996_latest.zip

unset ANDROID_HOME
unset ANDROID_SDK_ROOT_HOME

yes | ./cmdline-tools/bin/sdkmanager "cmdline-tools;latest" --sdk_root=$ANDROID_SDK_ROOT

export ANDROID_SDK_ROOT_HOME=`pwd`/cmdline-tools/latest

yes | ./cmdline-tools/bin/sdkmanager --licenses --sdk_root=$ANDROID_SDK_ROOT

./gradlew build
