<!-- Header -->
<div align="center">
  <img src="https://suika2.com/img/AppIcon.png" width="100" height="100"/>
  <h1>Suika2</h1>
</div>

<!-- Content -->
[![Release](https://img.shields.io/github/release/suika2engine/suika2?style=for-the-badge&color=dark-green)](https://github.com/suika2engine/suika2/releases/latest)

Suika2 is an open-source, cross-platform visual novel development engine that allows you to create a feature-rich visual novel with the care and attention to detail that your stories deserve. Featuring support and documentation in Japanese, Chinese, and English as well as active development and an accepting community, Suika2 could be the perfect start to your thrilling adventure, fawning romance, chilling epic, or anything else you could possibly imagine!

[Join our Discord server!](https://discord.gg/ZmvXxE8GFg)  ![Discord](https://discord.com/api/guilds/986623112617541677/widget.png)

Visit [the Website](https://suika2.com/en/) and [the Wiki](https://github.com/suika2engine/suika2/wiki), both of which are frequently updated!

Don't have a Discord account or would prefer to speak formally? Send an e-mail to `midori@suika2.com`.

**Note:** Replies will be sent from a Gmail address. 

## Demo

[Click here to start the Web demo](https://suika2.com/game-en/).

![demo](https://github.com/suika2engine/suika2/raw/master/doc/img/screenshot.jpg "screenshot")

## Portability

Official builds of Suika2 run on Windows, Mac, and Web browsers.
The source code can be compiled for iPhone, iPad, Android, Linux, FreeBSD, NetBSD, and Switch.
If a developer wishes, Windows and Mac versions can also be compiled from source.

Suika2 consists of a platform independent layer and a hardware abstraction layer (HAL).
The platform independent layer is written in ANSI C, the most portable language in the world.
HALs are currently written in C, C++, Objective-C, and Java.

If you want to port Suika2 to a new target platform, you are only required to write a HAL.

## Suika2 Pro for Creators

[`Suika2 Pro for Creators`](https://github.com/suika2engine/suika2/wiki/5.-Suika2-Pro-for-Creators) is Suika2's sister application, a visual development tool for Windows and Mac that provides developers with a debugger and package export function.

## Suika2 Kirara

[`Suika2 Kirara`](https://github.com/suika2engine/kirara) is Suika2's GUI application. If you are not familiar with scripting, please consider using Kirara.

## Contribution

Please submit issues light-heartedly as any and all feedback is welcome.

## Prebuilt Binaries

Download the latest prebuilt binaries from [the releases page](https://github.com/suika2engine/suika2/releases/latest).

**Note:** The provided ZIP files contain binaries for both Windows and Mac.

## Canary Build 

If you want to try our latest bug fixes and preview features, please see [Github Actions](https://github.com/suika2engine/suika2/actions).

## Building from Source

See `build/README.md` or visit the [Wiki](https://github.com/suika2engine/suika2/wiki/1.-Installation-and-Building-from-Source).

## License

This software is released under the MIT license.
You can redistribute this software commercially.
See `COPYING`.

It's important to include all licenses for assets that require them.

## Usage

* On Windows:
    * Download the zip file from [the releases page](https://github.com/suika2engine/suika2/releases) and extract it.
    * Open the `suika2` folder in Explorer.
    * Open the `suika.exe` application.

* On Mac:
    * Download the zip file from [the releases page](https://github.com/suika2engine/suika2/releases) and extract it.
    * Open the `suika2` folder in Finder.
    * Copy the `suika` application in `mac.dmg` to the `suika2` folder.
    * Open the `suika` application in the `suika2` folder.

* On iPhone/iPad:
    * Build from the source code using Xcode. See `build/README.md` or visit the [Wiki](https://github.com/suika2engine/suika2/wiki/1.-Installation-and-Building-from-Source#ios).

* On Android:
    * A sample game is available on Google Play. See [the official web site](https://suika2.com/en/).
    * To make your own game, you have to build from the source code using Android Studio. See `build/README.md` or visit the [Wiki](https://github.com/suika2engine/suika2/wiki/1.-Installation-and-Building-from-Source#android).

* On Linux/FreeBSD/NetBSD:
    * Build from the source code. See `build/README.md` or visit the [Wiki](https://github.com/suika2engine/suika2/wiki/1.-Installation-and-Building-from-Source#linux-binary-x86_64).

* On Switch Homebrew:  
    * Copy `suika.nro` to `yourgamedir` (See `build/README.md` if you want to build from source)
    * Copy `yourgamedir` to a Switch compatible sdcard (`/switch/yourgamedir`)
    * Install `hbmenu` and select an arbitrary Switch app, then press `R` to enter `hbmenu` (applet mode is not available as there is not enough memory)
    * select `swika` to play

## Manuals

* [Wiki](https://github.com/suika2engine/suika2/wiki/1.-Installation-and-Building-from-Source)
* [ドキュメント](https://suika2.com/wiki-jp/)
* [Tutorial](https://github.com/suika2engine/suika2/wiki/Tutorial)

## What does "suika" mean?

"Suika" means "watermelon" in Japanese.

### Trivia
Did you know...
* Midori wears a watermelon themed tie and pair of hair ribbons to stand out in her uniform? She also has 'Suika Orange' hair!
* Midori has an older sister, Rina, who's in university!
* Suika2 is the successor to 'Suika Studio' - you can find Suika Studio on the old English website with some deductive skills! (Suika Studio has long since reached its EOL.)
* For a 'History of Suika2' in update logs, visit [luxion.jp](https://luxion.jp/) or visit [suika2.com](https://suika2.com/en/) for the projects brief history.
