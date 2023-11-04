<!-- Big Banner -->
<img src="https://github.com/suika2engine/suika2/raw/master/doc/img/main_repo.png" width="100%">

## About

Suika2 is a visual novel development suite with an open-source philosophy, a true cross-platform design, and production-ready quality.

## Discord

<a href="https://discord.gg/ZmvXxE8GFg"><img src="https://discordapp.com/api/guilds/986623112617541677/widget.png?style=banner1"></a>

## Web Demo

<a href="https://suika2.com/run/sample/"><img src="https://github.com/suika2engine/suika2/raw/master/doc/img/screenshot.jpg"></a>

## Portability

Suika2 runs on Windows, macOS, Web browsers, iPhone, iPad, Android, Linux, FreeBSD, NetBSD, and some consoles.

Suika2 consists of the platform independent layer (PIL) and the hardware abstraction layer (HAL).
PIL is written in ANSI C, the most portable language in the world.
HAL is currently written in C, C++, Objective-C and Java.

If we would like to port Suika2 to a new target platform, we are only required to write a thin HAL.

Suika2 never depends on SDK/frameworks such as Unity, Godot, or SDL2.
In fact, it takes less than a week for us to port Suika2 to a platform that do not support such SDK/frameworks.
In other words, HAL, our compatibility layer, is very complete in its API design.

## Suika2 Pro

Suika2 Pro is a developer tool for Suika2, available for Windows, macOS and Linux.
It provides developers with a debugging capability and an export function.

## Binary Distribution

<!-- Release Banner -->
<img src="https://img.shields.io/github/release/suika2engine/suika2?style=for-the-badge&color=dark-green">

Download the latest release from [the official website](https://suika2.com/en/).

**Note:** We provide a zip file that contains all binaries for Windows, macOS, Web, and Linux.

## Building from Source

See [build/README.md](https://github.com/suika2engine/suika2/blob/master/build/README.md).

## CI

We do CI. [Check artifacts here.](https://github.com/ktabata/suika2/actions/workflows/suika-windows.yml)

Due to a code signing issue, we don't create automatic releases.
The `Certum code signing certificate in the cloud` we use cannot be used with GitHub Actions.

## License

This software is released under the MIT license.
You can redistribute this software commercially.
See [COPYING](https://github.com/suika2engine/suika2/blob/master/doc/COPYING) for all licenses including dependency libraries.

## Usage

* On Windows:
  * Download a zip file from [the official site](https://suika2.com/en/) and extract it.
  * Open the `suika2` folder in Explorer.
  * Start the `suika.exe` app.

* On Mac:
  * Download a zip file from [the official site](https://suika2.com/en/) and extract it.
  * Open the `suika2` folder in Finder.
  * Open the `mac.dmg` file and copy the `suika` app inside `mac.dmg` to the `suika2` folder.
  * Start the `suika` app in the `suika2` folder.

* On Linux:
  * You can use the `apt` command on Ubuntu:
  ```
  sudo add-apt-repository ppa:ktabata/ppa
  sudo apt update
  sudo apt install suika2
  suika2
  ```

* On iPhone/iPad (iOS):
  * Export your game from `Suika2 Pro for Windows`
  * Build your game using Xcode

* On Android:
  * Export your game from `Suika2 Pro for Windows`
  * Build your game using `Android Studio`
  * FYI: A sample game is available on [Google Play](https://play.google.com/store/apps/details?id=jp.luxion.suika&pcampaignid=MKT-Other-global-all-co-prtnr-py-PartBadge-Mar2515-1).

* Other platforms
  * Build from the source code.
  * See [build/README.md](https://github.com/suika2engine/suika2/blob/master/build/README.md).

## Documentation

* [Japanese Documentation](https://suika2.com/wiki/?%E3%83%89%E3%82%AD%E3%83%A5%E3%83%A1%E3%83%B3%E3%83%88)
* [English Documentation](https://suika2.com/en/doc/)

## Contribution

Please feel free to submit any issues and pull requests as any and all feedback is appreciated!

## Suika Studio

Suika2 is the successor to "Suika Studio":
* [See the 2002 version here](https://github.com/ktabata/suika-studio-2002-gpl)
* [See the 2003 version here](https://github.com/ktabata/suika-studio-2003-gpl)

The author is a pioneer in GUI editor field of visual novel creation.

## Trivia

Did you know that...
* Midori wears a watermelon themed tie and pair of hair ribbons to stand out in her uniform!
* "Suika" means "watermelon" in Japanese.

## This software is sponsored by the following

<a href="https://luxion.co.jp/index_en.html"><img src="https://luxion.co.jp/luxion-soft-llc.png" width="256" alt="sponsored-luxion-soft-llc"></a>

Here is the place where you put your banner!
