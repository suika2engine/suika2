<!-- Big Banner -->
<img src="https://github.com/suika2engine/suika2/raw/master/doc/img/main_repo.png" width="100%">

## What is Suika2?

Suika2 is an open-source suite for developing Visual Novels, designed with a focus on cross-platform compatibility, and equipped with features and tools of production-grade quality.

## What are the Development Goals of Suika2?

Suika2 aspires to set the benchmark for future Visual Novel Creation Tools. It facilitates creators to craft their projects effortlessly and efficiently, without sacrificing features and usability. It provides tools and features to enrich both the developer’s and user’s experience.

## What are the characteristics of Suika2?

1. Simplicity: Suika2 enables the creation of Visual Novels in an easy and efficient manner.
2. Speed: Apps are constructed using native technologies, ensuring resource efficiency.
3. Cost-free: Suika2 is a FOSS (Free and Open Source Software), available for use by anyone without any hidden costs, fees, or payments.
4. Portability: Suika2 allows games to be developed on various platforms, and its nature makes porting to any platform extremely straightforward.
5. Cultural Value: We hold the belief that the creation of information is humanity’s true value.
6. Ready for Commerce: Game developers can publish their games on stores and earn income. Suika2 does not charge any royalty fees!
7. Prosperity: Our aim is to create a world where anyone can make a living with just a single computer.

## Downloads

<!-- Release Banner -->
<img src="https://img.shields.io/github/release/suika2engine/suika2?style=for-the-badge&color=dark-green">

[Download the latest release files from the official website.](https://suika2.com/en/dl/)

Note that releases on GitHub are infrequent. Please visit the website to obtain the latest release.

## Discord

Our community is a cherished and invaluable space that embraces developers irrespective of their nationality, language, ethnicity, color, lineage, beliefs, gender, education, age, religion, or identity. You are undoubtedly deserving of being a part of this community and we invite you to join our server.

<a href="https://discord.gg/ZmvXxE8GFg"><img src="https://discordapp.com/api/guilds/986623112617541677/widget.png?style=banner1"></a>

We hold a grand vision for cultural growth and engage in profound discussions, but at our core, we are simply developers. We are always receptive to dialogues and conversations!

## Suika2 Pro

`Suika2 Pro Desktop` and `Suika2 Pro Mobile` are the developer tools for Suika2, available for Windows, MacOS, IOS and Linux. They provide developers with many functionalities such as editing, debugging and exporting games.

## License

This software is released under the MIT license. There is no restriction on distribution and modification of the Suika2 Source Code.

## Contribution

The best way to contribute this project is to use it and give us feedback. We are also open to suggestions and ideas.

## Live Web Demo

<a href="https://suika2.com/run/sample/"><img src="https://github.com/suika2engine/suika2/raw/master/doc/img/screenshot.jpg"></a>

## Usage

* On Windows (Suika2 Pro Desktop for Windows):
  * Open [Microsoft Store](https://apps.microsoft.com/detail/XP99CQV05PR79W)

* On Mac:
  * Open [Mac App Store](https://apps.apple.com/us/app/suika2-pro-desktop/id6474658254)

* On iPhone/iPad:
  * Open [App Store](https://apps.apple.com/us/app/suika2-pro-mobile/id6474521680)

* On Android/Chromebook:
  * Open [Google Play](https://play.google.com/store/apps/details?id=jp.luxion.suikapro)

* On Linux including Chromebook:
  * You can build from the souce code:
  ```
  sudo apt-get install -y git build-essential libasound2-dev libx11-dev mesa-common-dev qt6-base-dev qt6-multimedia-dev libwebp-dev
  git clone https://github.com/suika2engine/suika2.git
  ./configure
  make
  sudo make install
  suika2
  ```
  * Or you can use the `apt` command on Ubuntu:
  ```
  sudo add-apt-repository ppa:ktabata/ppa
  sudo apt update
  sudo apt install suika2
  suika2
  ```

## Documentation

* [Japanese Documentation](https://suika2.com/wiki/?%E3%83%89%E3%82%AD%E3%83%A5%E3%83%A1%E3%83%B3%E3%83%88)
* [English Documentation](https://suika2.com/en/doc/)

## Portability

Games made with Suika2 can run on Windows, macOS, Web browsers, iPhone, iPad, Android, Linux, FreeBSD, NetBSD, and some consoles.

Suika2 consists of a platform independent core and a hardware abstraction layer (HAL).
The core is written in ANSI C, the most portable programming language in the world.
The implementations of HAL are currently written in C, C++, Objective-C and Java.

If you would like to port Suika2 to a new target platform,
you are only required to write a thin HAL and it will take for you only one week.

Suika2 never depends on SDKs or frameworks such as Unity, Godot or SDL2.
This is because HAL, our compatibility layer, is very complete in its API design.

## CI

We do CI but currently we don't make releases automatically on GitHub due to a code signing issue.

## Trivia

Did you know that...
* Midori wears a watermelon themed tie and pair of hair ribbons to stand out in her uniform!
* "Suika" means "watermelon" in Japanese.
* Suika2 is the successor to "Suika Studio":
  * [See the 2002 version here](https://github.com/ktabata/suika-studio-2002-gpl)
  * [See the 2003 version here](https://github.com/ktabata/suika-studio-2003-gpl)
  * The author is a pioneer in the field of GUI editors for visual novel creation.

## Sponsors

Here is the very place where you raise your banner.

**Come forward**, those who resonate with our vision and are of like mind.

Venite et videbitis.
