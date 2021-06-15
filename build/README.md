How to build
============

* Windows App
    * We use cross compiler to build Windows binary.
    * On Ubuntu 20.04, install following packages:
        * `build-essential`
        * `mingw-w64`
    * In terminal, enter `build/mingw` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build `suika.exe`.
        * Run `make install` to copy `suika.exe` to `game` directory.
    * To run Suika 2, double click `suika.exe` in `game` folder on Windows.

* Mac App
    * On macOS 11.4, install Xcode 12.5.
    * In terminal, enter `build/macos` directory.
        * Run `./build-libs.sh` to build libraries.
    * In Xcode, open `build/macos`.
        * Set your development team.
        * Build project.
        * Archive project.
        * Notarize application by `Distribute App` button.
        * Press `Export Notarized App` to export app to `game` folder.
    * To run Suika2, double click `suika.app` in `game` folder.

* Release File
    * On macOS 11.4, in terminal, enter `build/release` directory.
       * Modify `SIGNATURE` in `Makefile` to sign `mac.dmg` file.
       * Run `make` to create ZIP file.
       * Rename `suika-2.x.x.zip`.

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
        * Run `make install` to copy binary `suika` to `game` directory.
    * In terminal, enter `game` directory.
	    * Run `./suika`

* Raspberry Pi Binary
    * On Raspberry Pi OS, install following packages:
        * `libasound2-dev`
        * `libx11-dev`
        * `libxpm-dev`
    * In terminal, enter `build/linux-arm` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build Suika2 binary.
        * Run `make install` to copy binary `suika` to `game` directory.
    * In terminal, enter `game` directory.
	    * Run `./suika`

* FreeBSD Binary
    * On FreeBSD 12.2, install following packages:
        * `gmake`
        * `alsa-lib`
        * `alsa-plugins`
    * In terminal, enter `build/freebsd` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `gmake` to build Suika2 binary.
        * Run `gmake install` to copy binary `suika` to `game` directory.
    * In terminal, enter `game` directory.
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
        * Run `gmake install` to copy binary `suika` to `game` directory.
    * In terminal, enter `game` directory.
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
