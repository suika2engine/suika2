How to build
============

* Windows App
    * We'll use the cross compiler to build Windows binary.
    * On Ubuntu 22.04 (WSL2 is OK), install the following packages:
        * `build-essential`
        * `mingw-w64`
    * Alternatively, on macOS 12, install Homebrew and the following package:
        * `mingw-w64`
    * In the terminal, enter `build/mingw` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build `suika.exe`.
        * Run `make install` to copy `suika.exe` to `suika2` directory.
        * Sign `suika.exe` using your certificate if you need.
    * To run Suika2, copy `suika.exe` to `game-en` folder or `game-jp` folder, then double click it.

* Mac App
    * On macOS 12, install Xcode 13.
    * In the terminal, enter `build/macos` directory.
        * Run `./build-libs.sh` to build libraries.
    * In Xcode, open `build/macos`.
        * Set your development team.
        * Build the project.
        * Archive then project.
        * Notarize the application by `Distribute App` button.
        * Press `Export Notarized App` to export the app to `suika2` folder.
    * To run Suika2, copy `suika.app` to `game-en` folder or `game-jp` folder, then double click it.

* iOS App
    * On macOS 12, install Xcode 13.
    * In the terminal, enter `build/ios` directory.
        * Run `./build-libs.sh` to build libraries.
        * Alternatively, you can run `./build-libs-sim.sh` for use with simulators on Apple Silicon Mac.
    * In Xcode, open `build/ios`.
        * In "Signing & Capabilities" tab, check `Automatically manage signing`.
        * Plug in the iOS device and build the project for the device.
        * Run on the iOS device.
        * Replace `build/ios/suika/data01.arc` and enjoy your game.

* Android App
    * Install Android Studio.
    * In Tools -> SDK Manager -> SDK Tools:
        * Install CMake (3.18.1)
        * Install NDK (24.0.8215888)
    * In the terminal, enter `build/android` directory.
        * Run `./prepare-libs.sh` to decompress libraries.
    * Open Suika2 project (`build/android`).
    * Build thenproject.
    * Run the app on your device or emulator.

* Web App
    * See `build/emscripten/README.md`

* Release Files
    * On macOS 12, in terminal, enter `build/release` directory.
       * Modify `SIGNATURE` in `Makefile` to sign dmg files.
       * Run `make` to create ZIP files.
       * Rename `suika-2.x.x-en.zip` and `suika-2.x.x-jp.zip`.

* Web Kit Distribution Files
    * Run `make` in `build/emscripten` first. (See `emscripten/README.md`.)
    * In the terminal, enter `build/web-kit` directory.
        * Run `make` to create ZIP files.
        * Rename `suika2-web-kit-2.x-x-en.zip` and `suika2-web-kit-2.x-x-jp.zip`.

* Linux Binary (x86_64)
    * On Ubuntu 22.04, install the following packages:
        * `build-essential`
        * `libasound2-dev`
        * `libx11-dev`
        * `libxpm-dev`
        * `mesa-common-dev`
    * In the terminal, enter `build/linux-x86_64` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build Suika2 binary.
        * Run `make install` to copy binary `suika` to `suika2` directory.
        * Copy `suika` to `game-en` directory or `game-jp` directory.
    * In the terminal, enter `game-en` directory or `game-jp` directory.
        * Run `./suika`

* Raspberry Pi Binary
    * On Raspberry Pi OS, install following packages:
        * `libasound2-dev`
        * `libx11-dev`
        * `libxpm-dev`
        * `mesa-common-dev`
    * In the terminal, enter `build/linux-arm` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build Suika2 binary.
        * Run `make install` to copy binary `suika` to `suika2` directory.
        * Copy `suika` to `game-en` directory or `game-jp` directory.
    * In the terminal, enter `game-en` or `game-jp` directory.
        * Run `./suika`

* FreeBSD Binary
    * On FreeBSD 12, install the following packages:
        * `gmake`
        * `alsa-lib`
        * `alsa-plugins`
    * In the terminal, enter `build/freebsd` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `gmake` to build Suika2 binary.
        * Run `gmake install` to copy binary `suika` to `suika2` directory.
        * Copy `suika` to `game-en` directory or `game-jp` directory
    * In the terminal, enter `game-en` directory or `game-jp` directory.
        * Run `./suika`.

* NetBSD Binary
    * On NetBSD 9, install the following packages:
        * `gmake`
        * `alsa-lib`
        * `alsa-plugins-oss`
    * `export LD_LIBRARY_PATH=/usr/pkg/lib:/usr/X11R7/lib`
    * In the terminal, enter `build/netbsd` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `gmake` to build Suika2 binary.
        * Run `gmake install` to copy binary `suika` to `suika2` directory.
        * Copy `suika` to `game-en` directory or `game-jp` directory
    * In the terminal, enter `game-en` directory or `game-jp` directory.
        * Run `./suika`.
    * FYI: To setup ALSA/OSS, create /etc/asound.conf
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
