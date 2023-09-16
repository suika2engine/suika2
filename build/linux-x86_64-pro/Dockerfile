FROM ubuntu:22.04

# Install the host Qt libraries.
RUN apt-get update
RUN apt-get install -y \
    git \
    cmake \
    qt6-base-dev \
    qt6-multimedia-dev \
    qt6-shader-baker \
    libqt6shadertools6-dev \
    qt6-declarative-dev \
    qt6-declarative-dev-tools \
    qt6-declarative-private-dev \
    qt6-documentation-tools

# Checkout the Qt6 repository.
WORKDIR /root
RUN git clone git://code.qt.io/qt/qt5.git qt6

# Update the submodules and remove ones that we don' use.
WORKDIR /root/qt6
RUN perl init-repository
RUN rm -rf qtactiveqt \
       	   qt3d \
	   qtfeedback \
	   qtquick3d \
	   qttools \
	   qt5compat \
	   qtgamepad \
 	   qtquicktimeline \
	   qttranslations \
	   qtimageformats \
	   qtremoteobjects \
	   qtvirtualkeyboard \
	   qtlocation \
	   qtrepotools \
	   qtwayland \
	   qtcanvas3d \
	   qtlottie \
	   qtscxml \
	   qtwebchannel \
	   qtcharts \
	   qtmqtt \
	   qtsensors \
	   qtwebengine \
	   qtcoap \
	   qtnetworkauth \
	   qtserialbus \
	   qtwebglplugin \
	   qtconnectivity \
	   qtopcua \
	   qtserialport \
	   qtwebsockets \
	   qtdatavis3d \
	   qtpim \
	   qtspeech \
	   qtwebview \
	   qtdeclarative \
	   qtpositioning \
	   qtsvg \
	   qtxmlpatterns \
	   qtdoc \
	   qtqa \
	   qtsystems

# Ubuntu 22.04 quirk.
RUN qtchooser -install qt6 $(which qmake6)
RUN mkdir -p /usr/share/qtchooser
RUN echo '/usr/lib/qt6/bin' > /usr/share/qtchooser/qt6.conf
RUN echo '/usr/lib/x86_64-linux-gnu' >> /usr/share/qtchooser/qt6.conf
RUN mkdir -p /usr/lib/$(uname -p)-linux-gnu/qt-default/qtchooser
RUN ln -n /usr/share/qtchooser/qt6.conf /usr/lib/$(uname -p)-linux-gnu/qt-default/qtchooser/default.conf
ENV QT_SELECT=qt6

# Create a toolchain.cmake file for the mingw cross compilation.
RUN mkdir -p /root/qt6/qt6-build
RUN echo 'cmake_minimum_required(VERSION 3.18)' > /root/qt6/qt6-build/toolchain.cmake
RUN echo 'include_guard(GLOBAL)' >> /root/qt6/qt6-build/toolchain.cmake
RUN echo 'set(QT_HOST_PATH /home/tabata/Qt/6.2.4)' >> /root/qt6/qt6-build/toolchain.cmake
RUN echo 'set(Qt6HostInfo_DIR /usr/lib/x86_64-linux-gnu/cmake/Qt6HostInfo)' >> /root/qt6/qt6-build/toolchain.cmake
RUN echo 'set(Qt6ShaderToolsTools_DIR /usr/lib/qt6/bin)' >> /root/qt6/qt6-build/toolchain.cmake
RUN echo 'set(Qt6ToolsTools_DIR /usr/lib/qt6/bin)' >> /root/qt6/qt6-build/toolchain.cmake
RUN echo 'set(QT_FEATURE_linguist off)' >> /root/qt6/qt6-build/toolchain.cmake
RUN echo 'set(CMAKE_PREFIX_PATH /usr/lib/x86_64-linux-gnu/cmake)' >> /root/qt6/qt6-build/toolchain.cmake
RUN echo 'set(CMAKE_SYSTEM_NAME Windows)' >> /root/qt6/qt6-build/toolchain.cmake
RUN echo 'set(CMAKE_SYSTEM_PROCESSOR x86)' >> /root/qt6/qt6-build/toolchain.cmake
RUN echo 'set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)' >> /root/qt6/qt6-build/toolchain.cmake
RUN echo 'set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)' >> /root/qt6/qt6-build/toolchain.cmake
RUN echo 'include(CMakeInitializeConfigs)' >> /root/qt6/qt6-build/toolchain.cmake

# Run configure.
WORKDIR /root/qt6/qt6-build
RUN ../configure \
        --prefix=/opt/qt \
	-static \
	-static-runtime \
	CMAKE_TOOLCHAIN_FILE=/root/qt6/qt6-build/toolchain.cmake

# Do build.
RUN cmake --build . --parallel

# Do install.
RUN cmake --install .
