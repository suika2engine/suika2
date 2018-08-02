How to build
============

* Windows Binary
    * We use cross compiler to build Windows binary.
    * On Ubuntu 18.04, install following packages.
        * mingw-w64
        * wine-stable
    * In terminal, enter `build/mingw` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build Suika2 binary.
        * Run `make install` to deploy Suika2 binary.
        * Run `make run` to run Suika2 by Wine.

* Mac Binary
    * On Mac OS 10.13, install Xcode and command line tools.
    * In terminal, enter `build/cocoa` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build Suika2 binary.
        * Run `make install` to deploy Suika2 binary.
    * In Finder, open `suika2-mac` folder.
        * Copy `Suika.app` to `/Applications`
		* Open `/Applications/Suika.app`

* Linux Binary
    * On Ubuntu 18.04, install following packages.
        * libasound2-dev
        * libX11-dev
        * libxpm-dev
    * In terminal, enter `build/linux` directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build Suika2 binary.
        * Run `make install` to deploy Suika2 binary.
        * Run `make run` to run Suika2.
