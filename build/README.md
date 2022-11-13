How to build
============

This document provides instructions for building the Suika2 application from its source. If you encounter any unexpected behaviour, please contact the
Team for further instructions and assistance. 


Windows App
============

This method uses a cross compiler to build a Windows binary.

* On Ubuntu 22.04 (WSL2 is OK), install the following packages:
   * `build-essential`
   * `mingw-w64`
* Alternatively, on macOS 13, install Homebrew and the following package:
   * `mingw-w64`
* From the terminal, enter the `build/mingw` directory.
   * Run `./build-libs.sh` to build libraries.
   * Run `make` to build `suika.exe`.
   * Run `make install` to copy `suika.exe` to `suika2` directory.
   * Sign `suika.exe` using your certificate if you need.
* To run Suika2, copy `suika.exe` to the `game-en` folder or the `game-jp` folder then double click it.


Mac App
============

* On macOS 13, install Xcode 14.
* From the terminal, enter the `build/macos` directory.
   * On Apple Silicon Mac:
      * Run `./build-libs.sh` to build the libraries.
   * On Intel Mac:
      * You can't build a `Universal Binary` library for `libpng`, so use the prebuilt libraries.
      * Download these from `https://suika2.com/dl/libroot-mac.tar.gz` and extract it.
* In Xcode, open `build/macos`.
   * Set your development team.
   * Build the project.
   * Archive then project.
   * Notarize the application by `Distribute App` button.
   * Press `Export Notarized App` to export the app to `build/macos` folder.
* To run Suika2, copy `suika.app` to the `game-en` folder or the `game-jp` folder then double click it.
* To distribute the app, follow these steps:
   * From the terminal, enter the `build/macos` directory.
   * Modify `SIGNATURE` in `make-dmg.sh` to sign dmg files.
   * Run `./make-dmg.sh` to create the dmg files.
   * Distribute the `mac.dmg` file.

**Note:** In some cases, building the application may fail when using an Intel-based Mac. In these cases, please build from an Apple Silicon machine.
If you are unable to do this, please contact the team for the up-to-date result of `build-libs.sh`.


iOS App
============

* On macOS 13, install Xcode 14.
* From the terminal, enter the `build/ios` directory.
   * Run `./build-libs.sh` to build libraries.
   * Alternatively, you can run `./build-libs-sim.sh` for use with simulators on Apple Silicon Mac.
* In Xcode, open `build/ios`.
   * In "Signing & Capabilities" tab, check `Automatically manage signing`.
   * Plug in the iOS device and build the project for the device.
   * Run on the iOS device.
   * Replace `build/ios/suika/data01.arc` and enjoy your game.


Android App
============

* Install Android Studio.
* From Tools -> SDK Manager -> SDK Tools:
   * Install CMake (3.18.1)
   * Install NDK (24.0.8215888)
* From the terminal, enter the `build/android` directory.
   * Run `./prepare-libs.sh` to decompress libraries.
* Open the Suika2 project (`build/android`).
   * Build the project.
   * Run the app on your device or emulator.


Web App
============

* See `build/emscripten/README.md`


Release Files
============

* On Ubuntu 22.04 (WSL2 is OK), from the terminal, run `./do-release.sh`.


Web Kit Distribution Files
============

* Run `make` in `build/emscripten` first. (See `emscripten/README.md`.)
* From the terminal, enter the `build/web-kit` directory.
   * Run `make` to create ZIP files.
   * Rename `suika2-web-kit-2.x-x-en.zip` and `suika2-web-kit-2.x-x-jp.zip`.


Linux Binary (x86_64)
============

* On Ubuntu 22.04, install the following packages:
   * `build-essential`
   * `libasound2-dev`
   * `libx11-dev`
   * `libxpm-dev`
   * `mesa-common-dev`
   * `libgstreamer1.0-dev`
   * `libgstreamer-plugins-base1.0-dev`
* From the terminal, enter the `build/linux-x86_64` directory.
   * Run `./build-libs.sh` to build libraries.
   * Run `make` to build Suika2 binary.
   * Run `make install` to copy binary `suika` to `suika2` directory.
   * Copy `suika` to `game-en` directory or `game-jp` directory.
* Before running the game, you may need to install the following package:
   * `gstreamer1.0-plugins-ugly`
* From the terminal, enter the `game-en` directory or the `game-jp` directory.
   * Run `./suika`


Raspberry Pi Binary
============

* On Raspberry Pi OS, install the following packages:
   * `libasound2-dev`
   * `libx11-dev`
   * `libxpm-dev`
   * `mesa-common-dev`
   * `libgstreamer1.0-dev`
   * `libgstreamer-plugins-base1.0-dev`
* From the terminal, enter the `build/linux-arm` directory.
   * Run `./build-libs.sh` to build libraries.
   * Run `make` to build Suika2 binary.
   * Run `make install` to copy binary `suika` to `suika2` directory.
   * Copy `suika` to `game-en` directory or `game-jp` directory.
* Before running the game, you may need to install the following package:
   * `gstreamer1.0-plugins-ugly`
* From the terminal, enter the `game-en` or the `game-jp` directory.
   * Run `./suika`


FreeBSD Binary
============

* On FreeBSD 12, install the following packages:
   * `gmake`
   * `alsa-lib`
   * `alsa-plugins`
* From the terminal, enter the `build/freebsd` directory.
   * Run `./build-libs.sh` to build libraries.
   * Run `gmake` to build Suika2 binary.
   * Run `gmake install` to copy binary `suika` to `suika2` directory.
   * Copy `suika` to `game-en` directory or `game-jp` directory
* From the terminal, enter the `game-en` directory or the `game-jp` directory.
   * Run `./suika`.


NetBSD Binary
============

* On NetBSD 9, install the following packages:
   * `gmake`
   * `alsa-lib`
   * `alsa-plugins-oss`
* `export LD_LIBRARY_PATH=/usr/pkg/lib:/usr/X11R7/lib`
* From the terminal, enter the `build/netbsd` directory.
   * Run `./build-libs.sh` to build libraries.
   * Run `gmake` to build Suika2 binary.
   * Run `gmake install` to copy binary `suika` to `suika2` directory.
   * Copy `suika` to `game-en` directory or `game-jp` directory
* From the terminal, enter the `game-en` directory or the `game-jp` directory.
   * Run `./suika`.

**Note:** To set up ALSA/OSS, create `/etc/asound.conf` and include the following:
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
