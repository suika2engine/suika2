<!-- Header -->
<div align="center">
  <img src="https://suika2.com/img/AppIcon.png" width="100" height="100"/>
  <h1>Suika2</h1>
</div>

<!-- Content -->
[![Release](https://img.shields.io/github/release/suika2engine/suika2?style=for-the-badge&color=dark-green)](https://github.com/suika2engine/suika2/releases/latest)

Suika2 is an open-source, cross-platform visual novel development engine that allows you to create a feature-rich visual novel with the care and attention to detail that your stories deserve. Featuring support and documentation in Japanese, Chinese, and English as well as active development and an accepting community, Suika2 could be the perfect start to your thrilling adventure, fawning romance, chilling epic, or anything else you could possibly imagine!

[Join our Discord server!](https://discord.gg/ZmvXxE8GFg)  ![Discord](https://discord.com/api/guilds/986623112617541677/widget.png)

Visit [the Website](https://suika2.com/en/) and [the Wiki](https://github.com/suika2engine/suika2/wiki).

Don't have a Discord account or would prefer to speak formally? Send an e-mail to `midori@suika2.com`.

## Demo

[Click here to start the Web demo](https://suika2.com/game-en/).

![demo](https://github.com/suika2engine/suika2/raw/master/doc/img/screenshot.jpg "screenshot")

## Portability

Official builds of Suika2 run on Windows, Mac, Linux and Web browsers.

The source code can be compiled for Windows, Mac, iPhone, iPad, Android, Linux, FreeBSD, NetBSD and Switch.

Suika2 consists of the platform independent layer (PIL) and the hardware abstraction layer (HAL).
PIL is written in ANSI C, the most portable language in the world.
HAL is currently written in C, C++, Objective-C and Java.

If you would like to port Suika2 to a new target platform, you are only required to write a HAL.

## Suika2 Pro

[Suika2 Pro](https://github.com/suika2engine/suika2/wiki/5.-Suika2-Pro-for-Creators) is a Suika2's sister app.
This is a visual tool for Windows and Mac that provides developers with a debugging capability and a package export function.

## Suika2 Kirara

[Suika2 Kirara](https://github.com/suika2engine/kirara) is a Suika2's sister app too.
This tool provides a capability to edit Suika2 scripts in a window with mouse operations.
If you are not familiar with scripting, please consider using Kirara.

**Note**: Kirara is still beta and we recommend for you to write scripts.

## Binary Distribution

Download the latest release from [the official site](https://suika2.com/en/).

**Note:** We provide a zip file that contains binaries for Windows, macOS, Web and Linux.

## Building from Source

See [build/README.md](https://github.com/suika2engine/suika2/blob/master/build/README.md).

## CI

If you would like to try development versions before release,
please go to [Github Actions](https://github.com/ktabata/suika2/actions/workflows/suika-windows.yml) and check artifacts.

We do CI, but we don't make releases automatically because our Certum code signing certificate can't be used in GitHub Actions.

## License

This software is released under the MIT license.
You can redistribute this software commercially.
See [COPYING](https://github.com/suika2engine/suika2/blob/master/doc/COPYING) for the all licenses including the dependency libraries.

## Usage

* On Windows:
  * Download a zip file from [the official site](https://suika2.com/en/) and extract it.
  * Open the `suika2` folder in Explorer.
  * Start the `suika.exe` application.

* On Mac:
  * Download a zip file from [the official site](https://suika2.com/en/) and extract it.
  * Open the `suika2` folder in Finder.
  * Open the `mac.dmg` file and copy the `suika` app inside `mac.dmg` to the `suika2` folder.
  * Start the `suika` app in the `suika2` folder.

* On Linux:
  * Download a zip file from [the official site](https://suika2.com/en/) and extract it.
  * Enter suika2 directory and run the following commands:
  ```
  chmod +x tools/suika-linux
  cp tools/suika-linux game-en/
  cd game-en
  ./suika-linux
  ```

* On iPhone/iPad (iOS):
  * Build from the source code using Xcode.
  * See [build/README.md](https://github.com/suika2engine/suika2/blob/master/build/README.md).

* On Android:
  * A sample game is available on [Google Play](https://play.google.com/store/apps/details?id=jp.luxion.suika&pcampaignid=MKT-Other-global-all-co-prtnr-py-PartBadge-Mar2515-1).
  * To make your own game, you have to build from the source code using Android Studio.
  * See [build/README.md](https://github.com/suika2engine/suika2/blob/master/build/README.md).

* On FreeBSD and NetBSD:
  * Build from the source code.
  * See [build/README.md](https://github.com/suika2engine/suika2/blob/master/build/README.md).

* On Switch Homebrew:  
  * Build from the source code.
  * See [build/README.md](https://github.com/suika2engine/suika2/blob/master/build/README.md).

## Manuals

* [Tutorial](https://suika2.com/en/tutorial.html)
* [Command Reference](https://suika2.com/en/reference.html)
* [Wiki (not up to date)](https://github.com/suika2engine/suika2/wiki/)
* [Japanese Documentation](https://suika2.com/)

## Contribution

Please submit issues light-heartedly as any and all feedback is welcome.

## What does "suika" mean?

"Suika" means "watermelon" in Japanese.

### Trivia

Did you know...

* Midori wears a watermelon themed tie and pair of hair ribbons to stand out in her uniform?
* Suika2 is the successor to "Suika Studio" - you can find Suika Studio on the web site with some deductive skills! (Suika Studio has long since reached its EOL.)
