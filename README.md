![icon](https://github.com/ktabata/suika2/raw/master/doc/icon.png "icon") Suika 2
=================================================================================

Suika 2 is a portable game engine that runs on both Windows and Mac. You can enjoy producing impressive visual novels using Suika 2.

## Demo

![demo](https://github.com/ktabata/suika2/raw/master/doc/screenshot-en.jpg "screenshot")

## Portability

In a strict sense, Suika 2 runs on Windows, Mac, Android, Linux, FreeBSD and NetBSD.

Suika 2 consists of a platform independent part and a hardware abstraction layer (HAL). The platform independent part is written in ANSI C with some GCC pragmas. HALs are currently written in C, Objective-C and Java.

If you want to port Suika 2 to a new target platform, it is only necessary to write a HAL.

## Contribution

Please write an issue with a light heart. Any and all feedback is welcome.
Pull requests that correct my English are especially welcome.

## Prebuilt Binaries

Download the binaries from [the official web site.](https://luxion.jp/s2/)

## Build from Source

See build/BUILD.md

## License

This software is released under the MIT license.
You can redistribute this software commercially.
See game/COPYING.

## Usage

* On Windows:
    * Download the zip file from [the official web site](https://luxion.jp/s2/) and extract it.
    * Open `suika2` folder in Explorer.
    * Open `suika.exe` application.

* On Mac:
    * Download the zip file from [the official web site](https://luxion.jp/s2/) and extract it.
    * Open `suika2` folder in Finder.
    * Copy `suika` application in `mac.dmg` to `suika2` folder.
    * Open `suika` application in `suika2` folder.

* On Android:
    * Sample game is available on Google Play. See [the official web site](https://luxion.jp/s2/)
    * To make your own game, you should build from source code using Android Studio.

* On Linux/FreeBSD/NetBSD:
    * Build from source code. See build/BUILD.md

## Enable English mode

* Open `conf/config.txt` by a text editor other than Notepad.
* You can find the following line:
```
#language=English
```
* Delete the heading character `#` and save the file.
* You'll get the messages in English.

## Manuals

* [Tutorial](https://github.com/ktabata/suika2/blob/master/doc/tutorial.md)
* [Command Reference](https://github.com/ktabata/suika2/blob/master/doc/reference.md)

## What does "suika" mean?

It means "watermelon" in Japanese.
