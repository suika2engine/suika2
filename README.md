![icon](https://github.com/ktabata/suika2/raw/master/doc/icon.png "icon") Suika2
=================================================================================

Suika2 is a portable game engine that runs on both Windows and Mac. You can enjoy producing impressive visual novels using Suika2.

## Demo

![demo](https://github.com/ktabata/suika2/raw/master/doc/screenshot-en.jpg "screenshot")

## Portability

In a strict sense, Suika2 runs on Windows, Mac, Web browsers, iPhone, iPad, Android, Linux, FreeBSD and NetBSD.

Suika2 consists of platform independent part and hardware abstraction layer (HAL). The platform independent part is written in ANSI C. HALs are currently written in C, Objective-C and Java.

If you want to port Suika2 to a new target platform, it is only necessary to write a HAL.

## Contribution

Please write an issue with a light heart. Any and all feedback is welcome.
Pull requests that correct my English are especially welcome.

## Prebuilt Binary

Download the binary from [the official web site.](https://suika2.com/en/)

## Build from Source

See `build/README.md`

## License

This software is released under the MIT license.
You can redistribute this software commercially.
See `COPYING`.

## Usage

* On Windows:
    * Download the zip file from [the official web site](https://suika2.com/en/) and extract it.
    * Open `suika2` folder in Explorer.
    * Open `suika.exe` application.

* On Mac:
    * Download the zip file from [the official web site](https://suika2.com/en/) and extract it.
    * Open `suika2` folder in Finder.
    * Copy `suika` application in `mac.dmg` to `suika2` folder.
    * Open `suika` application in `suika2` folder.

* On iPhone/iPad:
    * Build from source code using Xcode. See `build/README.md`

* On Android:
    * Sample game is available on Google Play. See [the official web site](https://suika2.com/en/)
    * To make your own game, you have to build from source code using Android Studio. See `build/README.md`

* On Linux/FreeBSD/NetBSD:
    * Build from source code. See `build/README.md`

## Manuals

* [Tutorial](https://suika2.com/en/tutorial.html)
* [Command Reference](https://suika2.com/en/reference.html)

## What does "suika" mean?

It means "watermelon" in Japanese.
