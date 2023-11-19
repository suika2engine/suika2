FROM ubuntu:22.04
RUN apt-get -y update
RUN apt-get -y dist-upgrade
RUN apt-get install -y openjdk-17-jdk-headless wget unzip

RUN mkdir /sdk
WORKDIR /sdk
RUN wget https://dl.google.com/android/repository/commandlinetools-linux-10406996_latest.zip
RUN unzip commandlinetools-linux-10406996_latest.zip

ENV JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64

ENV ANDROID_SDK_ROOT=/sdk/cmdline-tools
RUN yes | ./cmdline-tools/bin/sdkmanager "cmdline-tools;latest" --sdk_root=$ANDROID_SDK_ROOT

ENV ANDROID_SDK_ROOT=/sdk/cmdline-tools/latest
RUN yes | ./cmdline-tools/bin/sdkmanager --licenses --sdk_root=$ANDROID_SDK_ROOT
