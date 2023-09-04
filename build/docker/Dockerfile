FROM ubuntu:20.04

# Use Japanese mirror
RUN sed -i.bak -r 's!(deb|deb-src) \S+!\1 http://ftp.riken.jp/Linux/ubuntu/!' /etc/apt/sources.list

# Upgrade
RUN apt-get -y update
RUN apt-get -y dist-upgrade

# Common
RUN apt-get -y install build-essential

# For mingw
RUN apt-get -y install mingw-w64

# For Linux
RUN apt-get -y install libasound2-dev libx11-dev libxpm-dev mesa-common-dev
RUN apt-get -y remove libunwind-dev libunwind-14-dev
RUN apt-get -y install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev

# For Web
RUN apt-get -y install git
RUN git clone https://github.com/emscripten-core/emsdk.git
RUN cd emsdk && ./emsdk install latest
RUN cd emsdk && ./emsdk activate latest

# For Android
RUN apt-get install -y openjdk-17-jdk-headless wget
