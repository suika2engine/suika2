How to build
============

* Windows Binary
    * On Ubuntu 18.04, install following package.
	    * mingw-w64
    * Enter build/mingw directory.
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build Suika2 binary.
        * Run `make install` to deploy Suika2 binary.
        * Run `make run` to run Suika2.
    * Visual C++ project helps you to debug this software, but Microsoft's compiler does not do AVX optimization

* Mac Binary
    * On Mac OS 10.11, install Xcode7 and command line tools.
    * Enter build/cocoa directory
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build Suika2 binary.
        * Run `make install` to deploy Suika2 binary.
    * Enter mac directory and run Suika.app.
    * Xcode7 project helps you to debug this software.

* Linux Binary
    * On Ubuntu 18.04, install following packages.
        * libasound2-dev
        * libX11-dev
        * libxpm-dev
    * Enter `build/linux`
        * Run `./build-libs.sh` to build libraries.
        * Run `make` to build Suika2 binary.
        * Run `make install` to deploy Suika2 binary.
        * Run `make run` to run Suika2.
    * gdb helps you to debug this software.

* Android NDK
    * Open build/ndk on Android Studio
