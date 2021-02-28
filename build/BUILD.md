How to build
============

* Windows Binary
    * We use cross compiler to build Windows binary.
    * On Ubuntu 20.04, install following packages:
        * mingw-w64
        * wine-stable
    * In terminal, enter `build/mingw` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build Suika2 binary.
        * Run `make package` to create release zip file.
        * Run `make run` to run Suika2 by Wine.

* Mac Binary
    * On Mac OS 10.13, install command line developer tools.
    * In terminal, enter `build/cocoa` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build Suika2 binary.
        * Run `make package` to create release zip file.
    * In Finder, open `build/cocoa/suika2` folder.
        * Copy `Suika.app` to `/Applications`
        * Open `/Applications/Suika.app`

* Linux Binary
    * On Ubuntu 20.04, install following packages:
        * libasound2-dev
        * libX11-dev
        * libxpm-dev
    * In terminal, enter `build/linux` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build Suika2 binary.
        * Run `make package` to create release zip file.
        * Run `make run` to run Suika2.

* NetBSD Binary
    * On NetBSD 9.1, install following packages from pkgsrc:
        * gmake
        * alsa-lib
        * alsa-plugin-oss
    * In terminal, enter `build/netbsd` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `gmake` to build Suika2 binary.
        * Run `gmake run` to run Suika2.
    * To setup ALSA/OSS, create /etc/asound.conf
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
