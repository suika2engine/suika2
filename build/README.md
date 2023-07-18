How to build
============

This document provides instructions for building the Suika2 apps from the source code.

If you encounter any unexpected behaviour, please contact the development team for further instructions and assistance via GitHub or Discord.

## Getting Started

Firstly, you have to get the Suika2 repository using `Git`.
The way to do this depends on the operating system you use.

* On Ubuntu 22.04 (WSL2 is accepted):
  * From the terminal, run the following command:
    * `sudo apt-get update`
    * `sudo apt-get install -y git`
    * `git clone https://github.com/suika2engine/suika2.git`
    * `cd suika2`
    * `ls -lha`
    * `make` (this shows help)
    * `make setup` (this installs dependencies)
    * `make windows` (this builds `suika.exe`)
    * `make windows-64` (this builds `suika-64.exe`)
    * `make windows-arm64` (this builds `suika-arm64.exe`)
    * `make windows-pro` (this builds `suika-pro.exe`)
    * `make windows-capture` (this builds `suika-capture.exe`)

* On macOS 13:
  * From the terminal, run the following command:
    * `git clone https://github.com/suika2engine/suika2.git`
    * `cd suika2`
    * `ls -lha`

* On Windows:
  * Install `Git for Windows`
    * Check `Enable symbolic links` during the installation
  * Run `Git Bash` as **admin**
    * (If you do not run as admin, the symbolic links in the Android project will be broken)
    * `git clone -c core.symlinks=true https://github.com/suika2engine/suika2.git`
    * `cd suika2`
    * `ls -lha`

## Instant Docker Build

Docker is optional for Suika2 build.
You can use it if you want, but we developers don't.

* You can build the following apps in a single step using Docker:
  * `suika.exe` ... Windows (32-bit)
  * `suika-64.exe` ... Windows (64-bit)
  * `suika-arm64.exe` ... Windows (Arm64)
  * `suika-pro.exe` ... Windows (Suika2 Pro for Creators)
  * `suika-capture.exe` ... Windows (Capture App)
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
  * On Ubuntu 22.04 (WSL2 is acceptable), install the following packages:
    * `build-essential`
    * `mingw-w64`
  * On macOS 13, install `Homebrew` and the following package:
    * `mingw-w64`

* Procedure
  * From the terminal, navigate to the `build/mingw` directory and run the following commands:
    * `make`
    * `make install`

## Windows 64-bit App

This method will build the `suika-64.exe` app separately.

* Prerequisites
  * On Ubuntu 22.04 (WSL2 is acceptable), install the following packages:
    * `build-essential`
    * `mingw-w64`
  * On macOS 13, install `Homebrew` and the following package:
    * `mingw-w64`

* Procedure
  * From the terminal, navigate to the `build/mingw-64` directory and run the following commands:
    * `make`
    * `make install`

## Windows Arm64 App

This method will build the `suika-arm64.exe` app separately.

* Prerequisites
  * Ubuntu 22.04 (WSL2 is acceptable)

* Procedure
  * From the terminal, navigate to the `build/mingw-arm64` directory and run the following commands:
    * `make`
    * `make install`

## Windows Pro App

This method will build the `suika-pro.exe` app separately.

* Prerequisites
  * On Ubuntu 22.04 (WSL2 is acceptable), install the following packages:
    * `build-essential`
    * `mingw-w64`
  * On macOS 13, install `Homebrew` and the following package:
    * `mingw-w64`

* Procedure
  * From the terminal, navigate to the `build/mingw-pro` directory and run the following commands:
    * `make`
    * `make install`

## Windows Capture App

This method will build the `suika-capture.exe` app separately.

* Prerequisites
  * On Ubuntu 22.04 (WSL2 is acceptable), install the following packages:
    * `build-essential`
    * `mingw-w64`
  * On macOS 13, install `Homebrew` and the following package:
    * `mingw-w64`

* Procedure
  * From the terminal, navigate to the `build/mingw-capture` directory and run the following commands:
    * `make`
    * `make install`

`suika-capture.exe` records all user interactions and periodic/interactive screenshots.
Result data will be stored in the `record` folder.
One can make a test case with `suika-capture.exe` and replay them with `suika-replay` on Linux.
In addition, recorded data can be used as a bug report that includes a strict reproduction procedure.

## macOS Apps

This method will utilise `Xcode` and terminal to build macOS binaries.
For macOS, we have 3 app variants (`suika`, `suika-pro` and `suika-capture`).

* On macOS 13, install Xcode 14
* From the terminal, navigate to the `build/macos` directory and run the following command:
  * Run `./build-libs.sh` to build the libraries
  * On Intel Mac, this script downloads prebuilt libraries and extract them to avoid `libpng` build failure
* From Xcode, open `build/macos`
  * For debug apps:
    * Navigate to the `Signing & Capabilities` tab and select `Automatically Manage Signing`
    * Build the app variants (`suika`, `suika-pro` and `suika-capture`)
  * For distribution apps:
    * Set your development team (you need Apple Developer Program account)
    * Complete the following steps for each app variant (`suika`, `suika-pro` and `suika-capture`):
      * Archive the app
      * Press `Distribute App` button to notarize the app
      * Use `Developer ID`
      * Press `Export Notarized App` button and export the app to the `build/macos` folder
    * From the terminal, navigate to the `build/macos` directory and run the following command:
      * (Edit `make-dmg.sh` and set your `SIGNATURE`)
      * `./make-dmg.sh`

## iOS (iPhone and iPad) App

This method will utilise `Xcode` and terminal to build an iOS application.

* On macOS 13, install Xcode 14
* From the terminal, navigate to the `build/ios` directory and run the following command:
  * Run `./build-libs.sh` to build the libraries for use with actual devices
  * Alternatively, you can run `./build-libs-sim.sh` to build the libraries for use with simulators
* From Xcode, open `build/ios` and complete the following steps:
  * Navigate to the `Signing & Capabilities` tab and select `Automatically Manage Signing`
  * Connect the iOS device via cable and build the project for the device
  * Run the app from your iOS device
  * Replace `build/ios/suika/data01.arc` with your own `data01.arc` file if you like
* Distribute
  * You have to learn how to distribute apps on App Store
  * This procedure has been changing frequently, so you have to search the latest information

## Android App

This method will utilise `Android Studio` to build the Android app.

* On Windows:
  * Install `Android Studio`
  * From the `Git Bash` terminal, navigate to the `build/android` directory and run the following command:
    * `./prepare-libs.sh` (this decompresses the libraries)
  * Open the `build/android` project from `Android Studio`
  * Build the project

* On macOS or Ubuntu 22.04:
  * Install `Android Studio`
  * From the terminal, navigate to the `build/android` directory and run the following command:
    * `./prepare-libs.sh` (this decompresses the libraries)
  * Open the `build/android` project from `Android Studio`
  * Build the project

Run the app from your device or an emulator.

## Web

This method will utilise the Emscripten framework to build the web distribution.

* Prerequisites
  * You need a UNIX-like environment such as WSL2, Ubuntu 22.04, MSYS2, or macOS 13
  * Ensure you can access `make` and `python3` commands
  * Install `Emscripten` using `emsdk`
    * `git clone https://github.com/emscripten-core/emsdk.git`
    * `cd emsdk && ./emsdk install latest`
    * `cd emsdk && ./emsdk activate latest`
  * Generate your `data01.arc` using `suika-pro.exe`
    * In the `Suika2 Pro for Creators` app, select `File` and `Export Package`

* Instructions
  * From the terminal, navigate to the `build/emscripten` directory and run the following commands:
    * `make`
    * Copy your `data01.arc` into `build/emscripten/html/`
    * `make run`
  * From a browser, navigate to `http://localhost:8000/html/`

## Linux App (x86_64)

This method will build the Linux app.

* On Ubuntu 22.04 (WSL2 is acceptable), install the following packages:
  * `build-essential`
  * `libasound2-dev`
  * `libx11-dev`
  * `libxpm-dev`
  * `mesa-common-dev`
  * `libgstreamer1.0-dev`
  * `libgstreamer-plugins-base1.0-dev`
* From the terminal, navigate to the `build/linux-x86_64` directory and run the following commands:
  * `make`
  * `make install`
  * `make analyze` (this runs static analysis)
  * `make valgrind` (this checks memory leaks)
* To check compilation warnings with LLVM/Clang, you can run the following commands in `build/linux-x86_64-clang`:
  * `sudo apt-get update`
  * `sudo apt-get install -y clang`
  * `make`
  * `make analyze`

## Linux Replay App (x86_64)

This method will build the Linux Replay app.

* On Ubuntu 22.04 (WSL2 is acceptable), install the following packages:
  * `build-essential`
  * `libx11-dev`
  * `libxpm-dev`
  * `mesa-common-dev`
* From the terminal, navigate to the `build/linux-x86_64-replay` directory and run the following commands:
  * `make`
  * `make install`

## Raspberry Pi App

This method will build the Raspberry Pi app.

* On Raspberry Pi OS, install the following packages:
  * `libasound2-dev`
  * `libx11-dev`
  * `libxpm-dev`
  * `mesa-common-dev`
  * `libgstreamer1.0-dev`
  * `libgstreamer-plugins-base1.0-dev`
* From the terminal, navigate to the `build/raspberrypi` directory and run the following commands:
  * `make`
  * `make install`

## FreeBSD Binary

This method will build a FreeBSD app.

* On FreeBSD 12 (amd64), install the following packages:
  * `gmake`
  * `alsa-lib`
  * `alsa-plugins`
* From the terminal, navigate to the `build/freebsd` directory and run the following commands:
  * `gmake`
  * `gmake install`

## NetBSD Binary

This method will build a NetBSD app.

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

This method will build the Switch app.

* Manual Steps
  * Install [devkitpro](https://devkitpro.org/wiki/Getting_Started)
  * Add env `DEVKITPRO`, e.g., `export DEVKITPRO=/opt/devkitpro`
  * Run `sudo dkp-pacman -S switch-dev switch-portlibs`
  * In the `build/switch` directory:
    * `make swika.nro` (this builds nro file that can be loaded by hbmenu)
    * `make debug SWITH_IP=192.168.xx.xx` (this runs the app for debug)
* Docker Steps
  * You can use docker for the compililation:
  ``` shell
  docker pull devkitpro/devkita64
  docker run -d -it --rm -v $(pwd):/project --name devkita64_run devkitpro/devkita64
  docker exec -i devkita64_run bash -c "cd /project/build/switch && make"
  docker stop devkita64_run
  docker rm devkita64_run
  ```

## Release

This method will create release-ready ZIP files and upload them to FTP server.

* Prerequisites
  * Use both WSL2 and macOS
  * Use `OneDrive` to communicate between WSL2 and macOS
  * WSL2
    * Create a symbolic link `build/cloud` that points to the folder on `OneDrive`
    ```
    ln -s /mnt/c/Users/username/OneDrive/suika-cloud build/cloud
    ```
    * Create a symbolic lonk `build/ftplocal` that points to the local directory to put release files
    ```
    ln -s /home/username/Sites/suika2.com/dl build/ftplocal
    ```
    * Create a script `ftpupload.sh` somewhere and write code to upload a release file
    ```
    #!/bin/sh
    curl -T "$1" -u USERNAME:PASSWORD "ftp://ftp.your-ftp-server.com/your-upload-path/$1"
    ```
  * macOS
    * Create a symbolic link `build/cloud` that points to the folder on `OneDrive`
    ```
    ln -s /Users/username/OneDrive/suika-cloud build/cloud
    ```

* Release Steps
  1. On WSL2
    * Update the following documents:
      * `build/release/readme-jp.html`
      * `build/release/readme-en.html`
      * `build/web-kit/readme-jp.html`
      * `build/web-kit/readme-en.html`
    * In terminal, navigate to the `build/macos` directory and run the following commands:
      * `enter-version.sh`
        * Enter version string and press return
    * In terminal, navigate to the repository root
      * `git add -u`
      * `git commit -m "v2.x.y"` (write the version)
      * `git push origin master`
  2. On macOS
    * In terminal, navigate to the `build/macos` directory and run the following commands:
      * `git pull origin master`
      * `make` (this puts dmg files to `OneDrive`)
  3. On WSL2
    * In terminal, navigate to the repository root and run `make do-release`
      * "Enter version e.g. 2.9.0" -> input version string and press return
      * "Are you sure you want to release 2.x.y? (press return)" -> press return
      * "Please build Mac apps and press return." -> press return
      * "Please sign the Windows apps and press return."
        * Sign the apps in the cloud folder (`suika.exe`, `suika-64.exe`, `suika-arm64.exe`, `suika-pro.exe` and `suika-capture.exe`)
        * Press return in the terminal
      * "Stop if this is a pre-release." -> press return
      * The release files will be uploaded to your Web server
