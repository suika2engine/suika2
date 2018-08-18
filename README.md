![icon](https://github.com/ktabata/suika2/raw/master/doc/icon.png "icon") Suika 2
=================================================================================

Suika 2 is a portable game engine that runs on both Windows and Mac. You can enjoy producing impressive visual novels using Suika 2.

## Demo

![demo](https://github.com/ktabata/suika2/raw/master/doc/screenshot.jpg "screenshot")

## Portability

In a strict sense, Suika 2 runs on Windows, Mac, Linux, and Android.

Suika 2 consists of a platform independent part and a hardware abstraction layer (HAL). The platform independent part is written in ANSI C with some GCC pragmas. HALs are currently written in C, Objective-C, and Java.

If you want to port Suika 2 to a new target platform, it is only necessary to write a HAL.

## Contribution

Please write an issue with a light heart. Any and all feedback is welcome.

## Prebuilt Binaries

Download the binaries from [the official web site.](http://luxion.jp/s2/dl.html)

## Build from Source

See build/BUILD.md

## License

This software is released under the MIT license. See suika2/COPYING.

## Usage

* On Windows:
    * `git clone` the repository or download the zip file.
    * Open `suika2` folder in Explorer.
    * Double click `suika.exe`

* On Mac:
    * `git clone` the repository or download the zip file.
    * Open `suika2-mac` folder in Finder.
    * Copy `Suika` to the `/Applications` folder. This step is necessary.
    * Double click `Suika` in the `/Application` folder.

* On Linux:
    * `git clone` the repository or download the zip file.
    * Enter `suika2` directory in terminal.
    * run `./suika-linux`
