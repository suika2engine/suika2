HOW TO BUILD
============
This document provides instructions for building the Suika2 apps from the source code.

FYI: The original author uses WSL2 and macOS for the official builds.

## Getting Started
Firstly, you have to get the Suika2 repository using `Git`.

* From the terminal, run the following command:
```
git clone https://github.com/suika2engine/suika2.git
```

## Suika2 for Windows (main engine)
This method will build a Windows binary on WSL2, Linux or macOS.

* Prerequisite
  * Use WSL2, Ubuntu or macOS
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make setup
  ```

* Build
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make windows
  ```

## Suika2 for macOS (main engine)
This method will utilize `Xcode` and terminal to build macOS main engine binary.

* Steps
  * Use macOS 13
  * Install Xcode 14
  * From the terminal:
    * Navigate to the `build/macos` directory and run the following command:
    ```
    ./build-libs.sh
    ```
    * Note:
      * On an Apple Silicon Mac, `build-libs.sh` will build the libraries from the source codes
      * On an Intel Mac, `build-libs.sh` will download the prebuilt libraries to avoid a build failure of `libpng`
  * From Xcode, open `build/macos/suika.xcodeproj`
    * Select the `suika` target
    * Navigate to the `Signing & Capabilities` tab
    * Select `Automatically Manage Signing`
    * Build the `suika` target

* Additional steps for when you want to build apps for distribution
  * Select `suika` target
  * Navigate to the `Signing & Capabilities` tab
  * Set your development team (Apple ID)
  * Archive the app
  * Press `Distribute App` button to notarize the app
  * Use `Developer ID`
  * Press `Export Notarized App` button
  * Export the app to the `build/macos` folder
  * Edit `build/macos/make-dmg.sh` and set your `SIGNATURE`
  * From the terminal, navigate to the `build/macos` directory and run the following command to make `mac.dmg`:
  ```
  ./make-dmg.sh
  ```

## Suika2 for Web (main engine)
This method will build the Web version of Suika2.

* Build instructions
  * From the terminal, navigate to the `build/emscripten` directory
  * Run the following command to create the `build/emscripten/html/index.*` files:
  ```
  make
  ```
  * Note that `emsdk` will be installed when the first call of `make`

* Test instructions
  * Copy your `data01.arc` to `build/emscripten/html/`
  * `make run`
  * Open `http://localhost:8000/html/` by a browser

## Suika2 for Linux (main engine, x86_64)
This method will build a Linux app.

* Prerequisite
  * Use Ubuntu including WSL2
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make setup
  ```

* Build
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make linux
  ```

## Suika2 for Android (Android app)
This method requires `Android Studio` to build the Android app.

* Install `Android Studio`
* Run `Suika2 Pro` and export Android source code
  * Note that the Android export function is currently supported on only Windows version of `Suika2 Pro`
* Open the exported project from `Android Studio`
* Build the project
* Run the app from your device or an emulator.

## Suika2 for iOS (iPhone and iPad app)
This method will utilize `Xcode` and terminal to build an iOS app.

* Use macOS 13
* Install Xcode 14
* Run `Suika2 Pro` and export iOS source code
  * Note that the iOS export function is currently supported on only Windows version of `Suika2 Pro`
* From Xcode, open the exported project
* Complete the following steps:
  * Navigate to the `Signing & Capabilities` tab
  * Select `Automatically Manage Signing`
  * Connect your iOS device via USB cable
  * Build the project for the device
  * Run the app from your iOS device

## Suika2 Pro for Windows
This method will build a Suika2 Pro for Windows binary on WSL2, Linux or macOS.

* Prerequisite
  * Use WSL2, Ubuntu or macOS
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make setup
  ```

* Build
  * From the terminal, navigate to the source code directory and run the following command:
  ```
  make windows-pro
  ```

## Suika2 Pro for macOS
This method will utilize `Xcode` and terminal to build macOS main engine binary.

* Steps
  * Use macOS 13
  * Install Xcode 14
  * From the terminal:
    * Navigate to the `build/macos` directory and run the following command:
    ```
    ./build-libs.sh
    ```
    * Note:
      * On an Apple Silicon Mac, this script will build the libraries from the source codes
      * On an Intel Mac, this script will download the prebuilt libraries to avoid a build failure of `libpng`
  * From Xcode, open `build/macos/suika.xcodeproj`
    * Select `suika-pro` target
    * Navigate to the `Signing & Capabilities` tab
    * Select `Automatically Manage Signing`
    * Build `suika-pro` target

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
  make linux-pro
  ```

## Suika2 for Raspberry Pi (main engine)
This method will build a Raspberry Pi app.

* Steps
  * Use Raspberry Pi OS
  * From the terminal, run the following commands:
  ```
  sudo apt-get update
  sudo apt-get install libasound2-dev libx11-dev libxpm-dev mesa-common-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
  ```
  * From the terminal, navigate to the `build/raspberrypi` directory and run the following command:
  ```
  make
  ```

## Misc

### GCC Static Analysis
To use static analysis of `gcc`, type the following commands:
```
cd build/linux-x86_64
make analyze
```

### LLVM Static Analysis
To use static analysis of LLVM/Clang, you can run the following commands:
```
sudo apt-get install -y clang
cd build/linux-x86_64-clang
make
make analyze
```

### Memory Leak Profiling
We use `valgrind` to detect memory leaks and keep memory-related bugs at zero.

To use memory leak checks on Linux, type the following commands:
```
cd build/linux-x86_64
make valgrind
```

### FreeBSD (main engine)
This method will build a FreeBSD binary.

* On FreeBSD 12 (amd64), install the following packages:
  * `gmake`
  * `alsa-lib`
  * `alsa-plugins`
* From the terminal, navigate to the `build/freebsd` directory and run the following commands:
```
gmake
gmake install
```

### NetBSD (main engine)
This method will build a NetBSD binary.

* On NetBSD 9 (amd64), install the following packages:
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
* From the terminal, navigate to the `build/netbsd` directory and run the following commands:
```
export LD_LIBRARY_PATH=/usr/pkg/lib:/usr/X11R7/lib
gmake
gmake install
```
