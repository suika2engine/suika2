How to build
============

This document provides instructions for building the Suika2 apps from the source code.

If you encounter any unexpected behaviour, please make an issue or report it on Discord server.

FYI: The original author uses WSL2 and macOS for the official builds.

## Getting Started
Firstly, you have to get the Suika2 repository using `Git`.
The way to do this depends on the operating system you use.

Alternatively, you can just download [the latest source code zip](https://github.com/suika2engine/suika2/archive/refs/heads/master.zip) and extract it.
If you don't need an Android App, this is the easiest way.
However, you need `Git for Windows` to build an Android App on Windows.

* On Windows:
  * Install `Git for Windows`
  * Run `Git Bash` and type the following command:
  ```
  git clone https://github.com/suika2engine/suika2.git
  ```

* On macOS 13:
  * From the terminal, run the following command:
  ```
  git clone https://github.com/suika2engine/suika2.git
  ```

* On Ubuntu 22.04 (and WSL2):
  * From the terminal, run the following commands:
  ```
  git clone https://github.com/suika2engine/suika2.git
  ```

## All Windows Apps
This method will build all Windows binaries on WSL2, Linux or macOS.
For Windows, we have 6 app variants (`suika.exe`, `suika-pro.exe`, `suika-64.exe`, `suika-arm64.exe`, `suika-capture.exe` and `suika-replay.exe`)

* Steps
  * Use WSL2, Ubuntu or macOS
  * From the terminal, navigate to the source code directory
    * Run the following commands:
    ```
    make setup
    make all-windows
    ```

## All macOS Apps (without Xcode GUI)
This method will utilize the terminal to build all macOS binaries.
For macOS, we have 4 app variants (`suika.app`, `suika-pro.app`, `suika-capture.app` and `suika-replay.app`).

* Steps
  * Use macOS 13
  * Install Xcode 14 since we need command line tools of Xcode
  * From the terminal, navigate to the source code directory
    * Run the following command:
    ```
    make all-macos
    ```

## macOS main engine app (with Xcode GUI)
This method will utilize `Xcode` and terminal to build macOS main engine binary.

* Steps
  * Use macOS 13
  * Install Xcode 14
  * From the terminal, navigate to the `build/macos` directory
    * Run the following command:
    ```
    ./build-libs.sh
    ```
    * Note:
      * On an Apple Silicon Mac, this script will build the libraries from the source codes
      * On an Intel Mac, this script will download the prebuilt libraries to avoid a build failure of `libpng`'s Universal Binary
  * From Xcode, open `build/macos/suika.xcodeproj`
  * If you want to build apps for local usage:
    * Select `suika` target
    * Navigate to the `Signing & Capabilities` tab
    * Select `Automatically Manage Signing`
    * Build the app variants (`suika`, `suika-pro`, `suika-capture` and `suika-replay`)
  * If you want to build apps for distribution:
    * Select `suika` target
    * Navigate to the `Signing & Capabilities` tab
    * Set your development team (Apple ID)
    * Archive the app
    * Press `Distribute App` button to notarize the app
    * Use `Developer ID`
    * Press `Export Notarized App` button
    * Export the app to the `build/macos` folder
    * From the terminal, navigate to the `build/macos` directory
      * Run the following command to make `mac.dmg`:
      ```
      # Edit make-dmg.sh and set your `SIGNATURE`
      ./make-dmg.sh
      ```

## iOS (iPhone and iPad) App
This method will utilize `Xcode` and terminal to build an iOS application.

* Steps
  * Use macOS 13
  * Install Xcode 14
  * From the terminal, navigate to the `build/ios` directory
    * Run the following command:
    ```
    ./build-libs.sh
    ```
    * Alternatively, you can run `./build-libs-sim.sh` to build the libraries for use with simulators
  * From Xcode, open `build/ios/suika.xcodeproj`
  * Complete the following steps:
    * Navigate to the `Signing & Capabilities` tab
    * Select `Automatically Manage Signing`
    * Connect the iOS device via cable
    * Build the project for the device
    * Run the app from your iOS device
    * Replace `build/ios/suika/data01.arc` with your own `data01.arc` file if you like
    * Rebuild and run

* Distribution
  * You have to learn how to distribute apps on the `App Store`
  * This procedure has been changing frequently, so you have to search the latest information

## Android App (without Android Studio)
This method requires WSL2 or Ubuntu, but doesn't require `Android Studio`.

* Steps
  * Use WSL2 or Ubuntu 22.04
  * From the terminal, run the following command to install OpenJDK 17:
  ```
  sudo apt-get install -y openjdk-17-jdk-headless
  ```
  * From the terminal, navigate to the `build/android` directory
    * Run the following command:
    ```
    ./build-on-linux.sh
    ```
  * You will find `suika.apk` in the `build/android` directory

## Android App (with Android Studio)
This method requires `Android Studio` to build the Android app.

* On Windows:
  * Install `Android Studio`
  * From the `Git Bash` terminal, navigate to the `build/android` directory
    * Run the following command:
    ```
    ./prepare-libs.sh
    ```
  * Open the `build/android` project from `Android Studio`
  * Build the project

* On macOS or Ubuntu 22.04:
  * Install `Android Studio`
  * From the terminal, navigate to the `build/android` directory
    * Run the following command:
    ```
    ./prepare-libs.sh
    ```
  * Open the `build/android` project from `Android Studio`
  * Build the project

Run the app from your device or an emulator.

## Web (Emscripten)
This method will build the Web version.

* Prerequisites
  * You need a UNIX-like environment such as WSL2, Ubuntu, MSYS2 or macOS
  * Ensure you can access `make` and `python3` commands
  * Install `Emscripten` using `emsdk`
  ```
  git clone https://github.com/emscripten-core/emsdk.git
  cd emsdk && ./emsdk install latest && cd ..
  cd emsdk && ./emsdk activate latest && cd ..
  ```
  * Generate your `data01.arc`
    * To do this, run `Suika2 Pro` app, select `File` and `Export Package`

* Build instructions
  * From the terminal, navigate to the `build/emscripten` directory
    * Run the following command:
    ```
    make
    ```

* Testing instructions
  * Put your `data01.arc` into `build/emscripten/html/`
  * From the terminal, navigate to the `build/emscripten` directory
    * Run the following command:
    ```
    make run
    ```
  * From a browser, navigate to `http://localhost:8000/html/`

## All Linux Apps (x86_64)
This method will build all Linux apps.

* Steps
  * Use WSL2, Ubuntu
  * From the terminal, navigate to the source code directory
    * Run the following command:
    ```
    make setup
    make all-linux
    ```

* Static analysis
  * To check memory leaks, type the following commands:
  ```
  cd build/linux-x86_64
  make valgrind
  ```
  * To use static analysis of gcc, type the following commands:
  ```
  cd build/linux-x86_64
  make analyze
  ```
  * To check compilation warnings with LLVM/Clang, you can run the following commands:
  ```
  sudo apt-get install -y clang
  cd build/linux-x86_64-clang
  make
  make analyze
  ```

## Raspberry Pi App
This method will build a Raspberry Pi app.

* Steps
  * Use Raspberry Pi OS
  * From the terminal, run the following commands:
  ```
  sudo apt-get update
  sudo apt-get install libasound2-dev libx11-dev libxpm-dev mesa-common-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
  ```
  * From the terminal, navigate to the `build/raspberrypi` directory
    * Run the following commands:
    ```
    make
    make install
    ```

## FreeBSD Binary
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

## NetBSD Binary
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

## Switch App
This method will build a Switch app.

**LEGAL NOTICE**: The original author does not have a license for the official
Switch SDK, so he just wrote a code that might work with SDL. After that,
a volunteer made it work with Switch homebrew. However, the original author
was not involved in this porting, and he is trying to obtain an official
license.

* Manual Steps
  * Install [devkitpro](https://devkitpro.org/wiki/Getting_Started)
  * Add env `DEVKITPRO`, e.g., `export DEVKITPRO=/opt/devkitpro`
  * Run `sudo dkp-pacman -S switch-dev switch-portlibs`
  * In the `build/switch` directory:
    * `make swika.nro` (this builds nro file that can be loaded by hbmenu)
    * `make debug SWITH_IP=192.168.xx.xx` (this runs the app for debug)

* Docker Steps
  * You can use a docker container for the compililation:
  ```
  docker pull devkitpro/devkita64
  docker run -d -it --rm -v $(pwd):/project --name devkita64_run devkitpro/devkita64
  docker exec -i devkita64_run bash -c "cd /project/build/switch && make"
  docker stop devkita64_run
  docker rm devkita64_run
  ```

* Run Steps
  * Copy `suika.nro` to `yourgamedir`.
  * Copy `yourgamedir` to a Switch compatible sdcard (`/switch/yourgamedir`)
  * Install `hbmenu` and select an arbitrary Switch app, then press `R` to enter `hbmenu`.
    * Note: applet mode is not available as there is not enough memory.
  * Select `swika` to play

## Instant Docker Build
Docker is optional for Suika2 build.
You can use it if you would like, but the original author doesn't.

* You can build the following apps in a single step using Docker:
  * `suika.exe` ... Windows (32-bit)
  * `suika-64.exe` ... Windows (64-bit)
  * `suika-arm64.exe` ... Windows (Arm64)
  * `suika-pro.exe` ... Windows (Suika2 Pro for Creators)
  * `suika-capture.exe` ... Windows (Capture App)
  * `suika-replay.exe` ... Windows (Replay App)
  * `suika-linux` ... Linux (x86_64)
  * `suika-linux-capture` ... Linux (x86_64 Capture App)
  * `suika-linux-replay` ... Linux (x86_64 Replay App)
  * `html` ... Web
  * `suika.apk` ... Android

* Procedure
  * On WSL2:
    * Install `Docker Desktop` on Windows
    * From the terminal, navigate to the `build/docker` directory and run the following command:
    ```
    ./build.sh
    ```
  * On Windows:
    * Install `Docker Desktop`
    * Double click `build/docker/build.bat`
  * On macOS 13:
    * Install `Docker Desktop`
    * From the terminal, navigate to the `build/docker` directory and run the following command:
    ```
    ./build.sh
    ```
  * On Ubuntu 22.04:
    * Install `docker.io`
    * From the terminal, navigate to the `build/docker` directory and run the following command:
    ```
    ./build.sh
    ```

## Release
This method will create release zip files and upload them to a FTP server.

* Prerequisites
  * You need both WSL2 and macOS hosts
    * This is because the original author requires Windows to sign exe files
    * He uses "Certum Open Source Code Signing in the Cloud" product
    * He thinks it cannot be used in GitHub CI because the private key is not extractable in a normal way
  * On macOS:
    * Turn on the ssh server
    * Add your public key to `~/.ssh/authorized_keys`
    * Edit `SIGNATURE` in `build/macos/Makefile`
  * On WSL2:
    * Create `build/.env` file for credentials:
    ```
    WINDOWS_USER=your-windows-user-name
    MACOS_HOST=your-mac-host-name.local
    MACOS_USER=your-mac-user-name
    FTP_LOCAL=~/Sites/suika2.com/dl
    FTP_USER=your-ftp-account
    FTP_PASSWORD=your-ftp-password
    FTP_URL=ftp://ftp.your-web-server.com/suika2.com/dl
    ```

* Release Steps
  * On WSL2:
    * In the terminal, navigate to the `build/macos` directory and run the following command:
    ```
    ./update-version.sh '12.38'
    ```
    * Update the following documents:
      * `doc/readme-jp.html`
      * `doc/readme-en.html`
    * In the terminal, navigate to the repository root and run the following commands:
    ```
    git add doc/readme-jp.html doc/readme-en.html build/macos/suika.xcodeproj/project.pbxproj
    git commit -m "document: update README for x.y"
    git push github master
    make do-release
    ```
    * Release files will be built and uploaded to a Web server you specified in `.env`
