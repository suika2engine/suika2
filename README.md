<!-- Big Banner -->
<img src="https://github.com/suika2engine/suika2/raw/master/doc/img/main_repo.png" width="100%">

Suika2 is a visual novel creation suite with an open-source philosophy, a true cross-platform design, and production-ready quality.

## Downloads

<!-- Release Banner -->
<img src="https://img.shields.io/github/release/suika2engine/suika2?style=for-the-badge&color=dark-green">

[Download the latest release files from the official website.](https://suika2.com/en/dl/)

## Discord

<a href="https://discord.gg/ZmvXxE8GFg"><img src="https://discordapp.com/api/guilds/986623112617541677/widget.png?style=banner1"></a>

## Suika2 Pro

Suika2 Pro is a developer tool for Suika2, available for Windows, macOS and Linux.
It provides developers with many functionalities such as editing, debugging and exporting games.

## License

This software is released under the MIT license.
You can redistribute this software commercially.

## Contribution

Please feel free to submit any issues and pull requests as any and all feedback is appreciated!

## Live Web Demo

<a href="https://suika2.com/run/sample/"><img src="https://github.com/suika2engine/suika2/raw/master/doc/img/screenshot.jpg"></a>

## Usage

* On Windows:
  * Download an exe file from [the official site](https://suika2.com/en/) and run it.
  * Click `Suika2` from Windows menu.

* On Mac:
  * Download a dmg file from [the official site](https://suika2.com/en/) and open it.
  * Copy the `Suika2 Pro` app to the `Applications` folder.
  * Open the `Suika2 Pro` app in the `Applications` folder.

* On Linux:
  * You can use the `apt` command on Ubuntu:
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

Games made with Suika2 run on Windows, macOS, Web browsers, iPhone, iPad, Android, Linux, FreeBSD, NetBSD, and some consoles.

Suika2 consists of a platform independent core and a hardware abstraction layer (HAL).
The core is written in ANSI C, the most portable programming language in the world.
The HAL implementations are currently written in C, C++, Objective-C and Java.

If we would like to port Suika2 to a new target platform, we are only required to write a thin HAL.

Suika2 never depends on SDKs or frameworks such as Unity, Godot or SDL2.
In fact, it takes less than a week for us to port Suika2 to a platform that do not support such SDKs or frameworks.
In other words, HAL, our compatibility layer, is very complete in its API design.

## CI

We do CI for "regression testing".

Currently we don't make releases automatically due to a code signing issue of Windows binaries.
The `Certum code signing certificate in the cloud` we use cannot be used in GitHub Actions.

## Suika Studio

Suika2 is the successor to "Suika Studio":
* [See the 2002 version here](https://github.com/ktabata/suika-studio-2002-gpl)
* [See the 2003 version here](https://github.com/ktabata/suika-studio-2003-gpl)

The author is a pioneer in the field of GUI editors for visual novel creation.

## Suika2 Studio 2024 Wasm

Suika2 Studio 2024 Wasm is an IDE for Suika2, available on Web.
The author believes that the current Wasm runtimes on the major browsers are not mature enough.

[See live demo](https://suika2.com/vls/)

## Trivia

Did you know that...
* Midori wears a watermelon themed tie and pair of hair ribbons to stand out in her uniform!
* "Suika" means "watermelon" in Japanese.

## This software is sponsored by the following

<a href="https://luxion.co.jp/index_en.html"><img src="https://luxion.co.jp/luxion-soft-llc.png" width="256" alt="sponsored-luxion-soft-llc"></a>

Here is the place where you put your banner!
