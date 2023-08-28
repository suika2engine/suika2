How to build
============

This document provides instructions for building the Suika2 apps from the source code.

If you encounter any unexpected behaviour, please contact the development team for further instructions and assistance via GitHub or Discord.

FYI: We developers use WSL2 for the official builds.

## Getting Started

Firstly, you have to get the Suika2 repository using `Git`.
The way to do this depends on the operating system you use.

Alternatively, you can just download [the latest source code zip](https://github.com/suika2engine/suika2/archive/refs/heads/master.zip) and extract it.
If you don't need an Android App, this is the easiest way.
However, you need `Git Bash for Windows` to build an Android App on Windows.

* On Windows:
  * Install `Git for Windows`
  * Run `Git Bash`
    * `git clone https://github.com/suika2engine/suika2.git`
    * `cd suika2`

* On macOS 13:
  * From the terminal, run the following command:
    * `git clone https://github.com/suika2engine/suika2.git`
    * `cd suika2`

* On Ubuntu 22.04 (and WSL2):
  * From the terminal, run the following command:
    * `sudo apt-get update`
    * `sudo apt-get install -y git`
    * `git clone https://github.com/suika2engine/suika2.git`
    * `cd suika2`
    * `make`
      * This just shows help
      * You can build the binaries by `make windows`

## Instant Docker Build

Docker is optional for Suika2 build.
You can use it if you want, but we developers don't.

* You can build the following apps in a single step using Docker:
  * `suika.exe` ... Windows (32-bit)
  * `suika-64.exe` ... Windows (64-bit)
  * `suika-arm64.exe` ... Windows (Arm64)
  * `suika-pro.exe` ... Windows (Suika2 Pro for Creators)
  * `suika-capture.exe` ... Windows (Capture App)
  * `suika-replay.exe` ... Windows (Replay App)
  * `suika-linux` ... Linux (x86_64)
  * `suika-linux-replay` ... Linux (x86_64 Replay App)
  * `html` ... Web
  * `suika.apk` ... Android

* Procedure
  * On WSL2:
    * Install `Docker Desktop` on Windows
    * From the terminal, navigate to the `build/docker` directory and run the following command:
      * `./build.sh`
  * On Windows:
    * Install `Docker Desktop`
    * Double click `build/docker/build.bat`
  * On macOS 13:
    * Install `Docker Desktop`
    * From the terminal, navigate to the `build/docker` directory and run the following command:
      * `./build.sh`
  * On Ubuntu 22.04:
    * Install `docker.io`
    * From the terminal, navigate to the `build/docker` directory and run the following command:
      * `./build.sh`

## Windows 32-bit App

This method will build the `suika.exe` app separately.

* Prerequisites
  * On WSL2, install the following packages:
    * `mingw-w64`
  * On macOS 13, install `Homebrew` and the following package:
    * `mingw-w64`
  * On Ubuntu 22.04 (WSL2 is acceptable), install the following packages:
    * `mingw-w64`

* Procedure
  * From the terminal, navigate to the `build/mingw` directory and run the following commands:
    * `./build-libs.sh`
    * `make`
    * `make install`

## Windows 64-bit App

This method will build the `suika-64.exe` app separately.

* Prerequisites
  * On WSL2, install the following packages:
    * `mingw-w64`
  * On macOS 13, install `Homebrew` and the following package:
    * `mingw-w64`
  * On Ubuntu 22.04, install the following packages:
    * `mingw-w64`

* Procedure
  * From the terminal, navigate to the `build/mingw-64` directory and run the following commands:
    * `./build-libs.sh`
    * `make`
    * `make install`

## Windows Arm64 App

This method will build the `suika-arm64.exe` app separately.

* Prerequisites
  * WSL2 or Ubuntu 22.04.

* Procedure
  * From the terminal, navigate to the `build/mingw-arm64` directory and run the following commands:
    * `./build-libs.sh`
    * `make`
    * `make install`

## Windows Pro App

This method will build the `suika-pro.exe` app separately.

* Prerequisites
  * On WSL2, install the following packages:
    * `mingw-w64`
  * On macOS 13, install `Homebrew` and the following package:
    * `mingw-w64`
  * On Ubuntu 22.04 (WSL2 is acceptable), install the following packages:
    * `mingw-w64`

* Procedure
  * From the terminal, navigate to the `build/mingw-pro` directory and run the following commands:
    * `./build-libs.sh`
    * `make`
    * `make install`

## Windows Capture App

This method will build the `suika-capture.exe` app separately.

* Prerequisites
  * On WSL2, install the following packages:
    * `mingw-w64`
  * On macOS 13, install `Homebrew` and the following package:
    * `mingw-w64`
  * On Ubuntu 22.04 (WSL2 is acceptable), install the following packages:
    * `mingw-w64`

* Procedure
  * From the terminal, navigate to the `build/mingw-capture` directory and run the following commands:
    * `./build-libs.sh`
    * `make`
    * `make install`

`suika-capture.exe` records all user interactions and periodic/interactive screenshots.
Result data will be stored in the `record` folder.
One can make a test case with `suika-capture.exe` and replay them with `suika-replay.exe` on Windows or `suika-replay` on Linux.
In addition, recorded data can be used as a bug report that includes a strict reproduction procedure.

## macOS Apps (without Xcode GUI)

This method will utilise the terminal to build macOS binaries.
For macOS, we have 3 app variants (`suika`, `suika-pro`, `suika-capture` and `suika-replay`).

* On macOS 13, install Xcode 14 (we don't use the GUI of Xcode)
* From the terminal, navigate to the `build/macos` directory and run the following commands:
  * `./build-libs.sh`
    * Note: On Apple Silicon Mac, this script will build the libraries from the source codes
    * Note: On Intel Mac, this script will download the prebuilt libraries to avoid a build failure of `libpng`'s Universal Binary
  * `make noupload`
* Instead of `make upload`, you can use `make` for archiving, singing and notarization by a single command
  * Refer to `xcbuild-wrap.sh` and sign the apps with your Apple ID

## macOS Apps (with Xcode)

This method will utilise `Xcode` and terminal to build macOS binaries.
For macOS, we have 3 app variants (`suika`, `suika-pro`, `suika-capture` and `suika-replay`).

* On macOS 13, install Xcode 14
* From the terminal, navigate to the `build/macos` directory and run the following command:
  * `./build-libs.sh`
    * Note: On Apple Silicon Mac, this script will build the libraries from the source codes
    * Note: On Intel Mac, this script will download the prebuilt libraries to avoid a build failure of `libpng`'s Universal Binary
* From Xcode, open `build/macos/suika.xcodeproj`
  * For apps to use locally:
    * Navigate to the `Signing & Capabilities` tab and select `Automatically Manage Signing`
    * Build the app variants (`suika`, `suika-pro`, `suika-capture` and `suika-replay`)
  * For apps to distribute:
    * Set your development team (Apple ID)
    * Complete the following steps for each app variant (`suika`, `suika-pro`, `suika-capture` and `suika-replay`):
      * Archive the app
      * Press `Distribute App` button to notarize the app
      * Use `Developer ID`
      * Press `Export Notarized App` button and export the app to the `build/macos` folder
    * From the terminal, navigate to the `build/macos` directory and run the following command:
      * (Edit `make-dmg.sh` and set your `SIGNATURE`)
      * `./make-dmg.sh`
      * This will make `mac.dmg`

## iOS (iPhone and iPad) App

This method will utilise `Xcode` and terminal to build an iOS application.

* On macOS 13, install Xcode 14
* From the terminal, navigate to the `build/ios` directory and run the following command:
  * Run `./build-libs.sh` to build the libraries for use with actual devices
  * Alternatively, you can run `./build-libs-sim.sh` to build the libraries for use with simulators
* From Xcode, open `build/ios/suika.xcodeproj` and complete the following steps:
  * Navigate to the `Signing & Capabilities` tab and select `Automatically Manage Signing`
  * Connect the iOS device via cable and build the project for the device
  * Run the app from your iOS device
  * Replace `build/ios/suika/data01.arc` with your own `data01.arc` file if you like
* Distribute
  * You have to learn how to distribute apps on the `App Store`
  * This procedure has been changing frequently, so you have to search the latest information

## Android App (without Android Studio)

This method does not require `Android Studio`.

* On WSL2 or Ubuntu 22.04:
  * From the terminal, run the following command to install OpenJDK 17:
    * `sudo apt-get install -y openjdk-17-jdk-headless`
  * From the terminal, navigate to the `build/android` directory and run the following command:
    * `./build-on-linux.sh`
  * You will find `suika.apk` in the `build/android` directory.

## Android App (with Android Studio)

This method will utilise `Android Studio` to build the Android app.

* On Windows:
  * Install `Android Studio`
  * From the `Git Bash` terminal, navigate to the `build/android` directory and run the following command:
    * `./prepare-libs.sh`
  * Open the `build/android` project from `Android Studio`
  * Build the project

* On macOS or Ubuntu 22.04:
  * Install `Android Studio`
  * From the terminal, navigate to the `build/android` directory and run the following command:
    * `./prepare-libs.sh`
  * Open the `build/android` project from `Android Studio`
  * Build the project

Run the app from your device or an emulator.

## Web (Emscripten)

This method will build the Web version.

* Prerequisites
  * You need a UNIX-like environment such as WSL2, Ubuntu 22.04, MSYS2, or macOS 13
  * Ensure you can access `make` and `python3` commands
  * Install `Emscripten` using `emsdk`
    * `git clone https://github.com/emscripten-core/emsdk.git`
    * `cd emsdk && ./emsdk install latest`
    * `cd emsdk && ./emsdk activate latest`
  * Generate your `data01.arc`
    * To do this, run `Suika2 Pro for Creators` app, select `File` and `Export Package`

* Build instructions
  * From the terminal, navigate to the `build/emscripten` directory and run the following command:
    * `make`

* Testing instructions
  * Put your `data01.arc` into `build/emscripten/html/`
  * From the terminal, navigate to the `build/emscripten` directory and run the following command:
    * `make run`
  * From a browser, navigate to `http://localhost:8000/html/`

## Linux App (x86_64)

This method will build the Linux app.

* On WSL2 or Ubuntu 22.04, install the following packages:
  * `build-essential`
  * `libasound2-dev`
  * `libx11-dev`
  * `libxpm-dev`
  * `mesa-common-dev`
  * `libgstreamer1.0-dev`
  * `libgstreamer-plugins-base1.0-dev`
* From the terminal, navigate to the `build/linux-x86_64` directory and run the following commands:
  * `./build-libs.sh`
  * `make`
  * `make install`
* To check memory leaks, type the following command:
  * `make valgrind`
* To use static analysis of gcc, type the following command:
  * `make analyze`
* To check compilation warnings with LLVM/Clang, you can run the following commands in `build/linux-x86_64-clang`:
  * `sudo apt-get update`
  * `sudo apt-get install -y clang`
  * `./build-libs.sh`
  * `make`
  * `make analyze`

## Linux Capture App (x86_64)

This method will build the Linux Capture app.

* On WSL2 or Ubuntu 22.04, install the following packages:
  * `build-essential`
  * `libx11-dev`
  * `libxpm-dev`
  * `mesa-common-dev`
* From the terminal, navigate to the `build/linux-x86_64-capture` directory and run the following commands:
  * `./build-libs.sh`
  * `make`
  * `make install`

## Linux Replay App (x86_64)

This method will build the Linux Replay app.

* On WSL2 or Ubuntu 22.04, install the following packages:
  * `build-essential`
  * `libx11-dev`
  * `libxpm-dev`
  * `mesa-common-dev`
* From the terminal, navigate to the `build/linux-x86_64-replay` directory and run the following commands:
  * `./build-libs.sh`
  * `make`
  * `make install`

## Raspberry Pi App

This method will build a Raspberry Pi app.

* On Raspberry Pi OS, install the following packages:
  * `libasound2-dev`
  * `libx11-dev`
  * `libxpm-dev`
  * `mesa-common-dev`
  * `libgstreamer1.0-dev`
  * `libgstreamer-plugins-base1.0-dev`
* From the terminal, navigate to the `build/raspberrypi` directory and run the following commands:
  * `./build-libs.sh`
  * `make`
  * `make install`

## FreeBSD Binary

This method will build a FreeBSD binary.

* On FreeBSD 12 (amd64), install the following packages:
  * `gmake`
  * `alsa-lib`
  * `alsa-plugins`
* From the terminal, navigate to the `build/freebsd` directory and run the following commands:
  * `gmake`
  * `gmake install`

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
  * `export LD_LIBRARY_PATH=/usr/pkg/lib:/usr/X11R7/lib`
  * `gmake`
  * `gmake install`

## Switch App

This method will build a Switch app.

LEGAL NOTICE: The original author does not have a license for the official
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
  ``` shell
  docker pull devkitpro/devkita64
  docker run -d -it --rm -v $(pwd):/project --name devkita64_run devkitpro/devkita64
  docker exec -i devkita64_run bash -c "cd /project/build/switch && make"
  docker stop devkita64_run
  docker rm devkita64_run
  ```

## Release

This method will create release-ready ZIP files and upload them to the FTP server.

* Prerequisites
  * We use both WSL2 and macOS
    * This is because CI/CD is not available due to an issue on the cheap code signing of Certum
    * If we can get an expensive code signing certificate to export our private key to GitHub, we will move to CI/CD
  * We use Microsoft `OneDrive` to communicate between WSL2 and macOS
  * On WSL2:
    * Create a symbolic link named `build/cloud` that points to the folder on `OneDrive`
    ```
    ln -s /mnt/c/Users/username/OneDrive/suika-cloud build/cloud
    ```
    * Create a symbolic link named `build/ftplocal` that points to the local directory to put release files
    ```
    ln -s /home/username/Sites/suika2.com/dl build/ftplocal
    ```
    * Create a script `ftpupload.sh` somewhere and write code to upload a release file
    ```
    #!/bin/sh
    curl -T "$1" -u USERNAME:PASSWORD "ftp://ftp.your-ftp-server.com/your-upload-path/$1"
    ```
  * On macOS:
    * Create a symbolic link named `build/cloud` that points to the folder on `OneDrive`
    ```
    ln -s /Users/username/OneDrive/suika-cloud build/cloud
    ```

* Release Steps
  1. On WSL2:
    * Update the following documents:
      * `build/release/readme-jp.html`
      * `build/release/readme-en.html`
    * In the terminal, navigate to the `build/macos` directory and run the following command:
      * `enter-version.sh`
      * Enter version string and press return
    * In the terminal, navigate to the repository root and run the following commands:
      * `git add build/release/readme-jp.html build/release/readme-en.html build/macos/suika.xcodeproj/project.pbxproj`
      * `git commit -m "document: README for v2.x.y"` (write the version)
      * `git push origin master`
  2. On macOS:
    * In the terminal, navigate to the `build/macos` directory and run the following commands:
      * `git pull origin master`
      * `make` (this puts the dmg files to `OneDrive`)
  3. On WSL2:
    * Check if the new dmg files were uploaded to `OneDrive`
    * In the terminal, navigate to the repository root and run the following command:
      * `make do-release`
      * "Enter version e.g. 2.9.0" -> input version string and press return
      * "Are you sure you want to release 2.x.y? (press return)" -> press return
      * "Please build Mac apps and press return." -> press return
      * "Please sign the Windows apps and press return."
        * Sign the all Windows apps in `OneDrive` (`suika.exe`, `suika-64.exe`, `suika-arm64.exe`, `suika-pro.exe`, `suika-capture.exe` and `suika-replay.exe`)
        * Press return in the terminal
    * The release files will be uploaded to the Web server
    * Don't forget to update the Web site!
