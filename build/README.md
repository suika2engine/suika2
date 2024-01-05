HOW TO BUILD
============

This document provides instructions for building various Suika2 apps from the source code.

FYI: The author uses macOS for the official builds because he only needs one MacBook Pro.

## Getting Started

Firstly, you have to get the Suika2 repository using `Git`.

* From the terminal, run the following command:
```
git clone https://github.com/suika2engine/suika2.git
```

# The Main Engines

## The Suika2 Main Engine for Windows

This method will build a Windows app on Ubuntu or macOS.

* Prerequisite
  * Use Ubuntu or macOS
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make setup
  ```

* Build (x86, 32-bit, recommended)
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make engine-windows
  ```

* Alternative build (x86_64, 64-bit)
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make engine-windows-64
  ```

* Alternative build (Arm64)
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make engine-windows-arm64
  ```

## The Suika2 Main Engine for macOS

This method will utilize `Xcode` and terminal to build macOS main engine binary.

* Use macOS 14
* Install Xcode 15
* From Xcode, open `build/engine-macos/suika.xcodeproj`
* Build the `suika` target

## The Suika2 Main Engine for Wasm

This method will build the Wasm version of Suika2.

* Build instructions
  * From the terminal, navigate to the source code directory
  * Run the following command to create the `build/engine-wasm/html/index.*` files:
  ```
  make engine-wasm
  ```
  * Note that `emsdk` will be installed when the first call of `make`

* Test instructions
  * Copy your `data01.arc` to `build/engine-wasm/html/`
  * Do `make run` in the `build/engine-wasm/` directory
  * Open `http://localhost:8000/html/` by a browser

## The Suika2 Main Engine for Linux

This method will build a Linux app.

* Prerequisite
  * Use Ubuntu
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make setup
  ```

* Build
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make engine-linux
  ```

## The Suika2 Main Engine for iOS (iPhone and iPad)

This method will utilize `Xcode` and terminal to build an iOS app.

* Use macOS 14
* Install Xcode 15
* Run `Suika2 Pro Desktop` and export iOS source code
* From Xcode, open the exported project
* Complete the following steps:
  * Connect your iOS device via USB cable
    * First time only: Enable debug mode on iOS and reboot the device
    * Reconnect the device
  * Register your device to Xcode
  * Set the build target to your device
  * Build the project for the device
    * First time only: Wait for a finish of transfer of iOS debug information
  * Run the app from your iOS device

## The Suika2 Main Engine for Android

This method requires `Android Studio` to build the Android app.

* Install `Android Studio`
* Run `Suika2 Pro Desktop` and export Android source code
* Open the exported project from `Android Studio`
* Build the project
* Run the app from your device or an emulator.

# Suika2 Pro

## Suika2 Pro Desktop for Windows

This method will build a Suika2 Pro for Windows app on Ubuntu or macOS.

* Prerequisite
  * Use Ubuntu or macOS
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make setup
  ```

* Build
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make pro-windows
  ```

## Suika2 Pro Desktop for macOS

This method will utilize `Xcode` and terminal to build a macOS Pro app.

* Use macOS 14
* Install Xcode 15
* From Xcode, open `build/pro-macos/pro-macos.xcodeproj`
* Build.

## Suika2 Pro Mobile for iOS

This method will utilize `Xcode` and terminal to build an iOS Pro app.

* Steps
  * Use macOS 14
  * Install Xcode 15
  * From Xcode, open `build/pro-ios/pro-ios.xcodeproj`
  * Build.

## Suika2 Pro for Linux

This method will build a Linux version of Suika2 Pro using Qt6.

* Prerequisite
  * Use Ubuntu including WSL2
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make setup
  ```

* Build
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make pro-linux
  ```

## Suika2 Studio 2024 Wasm

This method will build Suika2 Studio 2024 Wasm, a Web variation of Suika2 Pro, on Ubuntu or macOS.

* Use Ubuntu or macOS
* From the terminal, navigate to the source code directory and run the following command:
```
make pro-wasm
```
* Upload the `build/pro-wasm/html/*` files to your Web server
* Access the uploaded `index.html` via `https` (Note that `http` is not allowed)

# Misc

## GCC Static Analysis

To use static analysis of `gcc`, type the following commands:
```
cd build/engine-linux-x86_64
make analyze
```

## LLVM Static Analysis

To use static analysis of LLVM/Clang, you can run the following commands:
```
sudo apt-get install -y clang
cd build/engine-linux-x86_64-clang
make
make analyze
```

## Memory Leak Profiling

We use `valgrind` to detect memory leaks and keep memory-related bugs at zero.

To use memory leak checks on Linux, type the following commands:
```
cd build/engine-linux-x86_64
make valgrind
```

## The Suika2 Main Engine for Raspberry Pi

This method will build a Raspberry Pi app.

* Use Raspberry Pi OS
* From the terminal, run the following commands:
```
sudo apt-get update
sudo apt-get install libasound2-dev libx11-dev libxpm-dev mesa-common-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
```
* From the terminal, navigate to the source code directory and run the following command:
```
make engine-linux
```

## The Main Engine for FreeBSD

This method will build a FreeBSD binary.

* On FreeBSD 12 (amd64), install the following packages:
  * `gmake`
  * `alsa-lib`
  * `alsa-plugins`
* From the terminal, navigate to the `build/engine-linux` directory and run the following commands:
```
gmake -f Makefile.freebsd
gmake -f Makefile.freebsd install
```

## The Main Engine for NetBSD

This method will build a NetBSD binary.

* On NetBSD, install the following packages:
  * `gmake`
  * `alsa-lib`
  * `alsa-plugins-oss`
* Prerequisite
  * To setup `ALSA/OSS`, create `/etc/asound.conf` and copy the following snippet to the file:
  ```
  pcm.!default {
    type oss
    device /dev/audio
  }
  ctl.!default {
    type oss
    device /dev/mixer
  }
  ```
* From the terminal, navigate to the `build/engine-netbsd` directory and run the following commands:
```
export LD_LIBRARY_PATH=/usr/pkg/lib:/usr/X11R7/lib
gmake -f Makefile.netbsd
gmake -f Makefile.netbsd install
```

## The Main Engine for OpenBSD

This method will build an OpenBSD binary without a sound output.

* On OpenBSD, install the following package:
  * `gmake`
* From the terminal, navigate to the `build/engine-linux` directory and run the following commands:
```
gmake -f Makefile.openbsd
gmake -f Makefile.openbsd install
```
