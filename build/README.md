How to build
============

* Windows App
    * We'll use cross compiler to build Windows binary.
    * On Ubuntu 20.04, install following packages:
        * `build-essential`
        * `mingw-w64`
    * Alternatively, on macOS 11, install Homebrew and following package:
        * `mingw-w64`
    * In terminal, enter `build/mingw` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build `suika.exe`.
        * Run `make install` to copy `suika.exe` to `suika2` directory.
    * To run Suika2, copy `suika.exe` to `game-en` folder or `game-jp` folder, then double click it.

* Mac App
    * On macOS 11, install Xcode 12.
    * In terminal, enter `build/macos` directory.
        * Run `./build-libs.sh` to build libraries.
    * In Xcode, open `build/macos`.
        * Set your development team.
        * Build project.
        * Archive project.
        * Notarize application by `Distribute App` button.
        * Press `Export Notarized App` to export app to `suika2` folder.
    * To run Suika2, copy `suika.app` to `game-en` folder or `game-jp` folder, then double click it.

* Web App
    * See `emscripten/README.md`

* Release Files
    * On macOS 11, in terminal, enter `build/release` directory.
       * Modify `SIGNATURE` in `Makefile` to sign `mac.dmg` file.
       * Run `make` to create ZIP files.
       * Rename `suika-2.x.x-en.zip` and `suika-2.x.x-jp.zip`.

* Web Kit Distribution Files
    * Run `make` in `build/emscripten` first. (See `emscripten/README.md`.)
    * In terminal, enter `build/web-kit` directory.
        * Run `make` to create ZIP files.
        * Rename `suika2-web-kit-2.x-x-en.zip` and `suika2-web-kit-2.x-x-jp.zip`.

* Android App
    * Install Android Studio 3.3.1 from Android Studio Archive.
    * Select `Configure` on initial screen.
        * Install `CMake`, `LLDB`, `NDK`.
    * Open Suika2 project (`build/android`).
    * Build project.
    * Run app on device or emulator.

* Linux Binary
    * On Ubuntu 20.04, install following packages:
        * `build-essential`
        * `libasound2-dev`
        * `libx11-dev`
        * `libxpm-dev`
    * In terminal, enter `build/linux` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build Suika2 binary.
        * Run `make install` to copy binary `suika` to `suika2` directory.
        * Copy `suika` to `game-en` directory or `game-jp` directory.
    * In terminal, enter `game-en` directory or `game-jp` directory.
        * Run `./suika`

* Raspberry Pi Binary
    * On Raspberry Pi OS, install following packages:
        * `libasound2-dev`
        * `libx11-dev`
        * `libxpm-dev`
    * In terminal, enter `build/linux-arm` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build Suika2 binary.
        * Run `make install` to copy binary `suika` to `suika2` directory.
        * Copy `suika` to `game-en-small` directory or `game-jp-small` directory.
    * In terminal, enter `game-en-small` or `game-jp-small` directory.
	    * Run `./suika`

* FreeBSD Binary
    * On FreeBSD 12.2, install following packages:
        * `gmake`
        * `alsa-lib`
        * `alsa-plugins`
    * In terminal, enter `build/freebsd` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `gmake` to build Suika2 binary.
        * Run `gmake install` to copy binary `suika` to `suika2` directory.
        * Copy `suika` to `game-en` directory or `game-jp` directory
    * In terminal, enter `game-en` directory or `game-jp` directory.
        * Run `./suika`.

* NetBSD Binary
    * On NetBSD 9.1, install following packages:
        * `gmake`
        * `alsa-lib`
        * `alsa-plugins-oss`
    * `export LD_LIBRARY_PATH=/usr/pkg/lib:/usr/X11R7/lib`
    * In terminal, enter `build/netbsd` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `gmake` to build Suika2 binary.
        * Run `gmake install` to copy binary `suika` to `suika2` directory.
        * Copy `suika` to `game-en` directory or `game-jp` directory
    * In terminal, enter `game-en` directory or `game-jp` directory.
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
