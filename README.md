<!-- Big Banner -->
<img src="https://github.com/suika2engine/suika2/raw/master/doc/img/main_repo.png" width="100%">

## What is Suika2?

Suika2 is an open-source suite for developing visual novels; designed with a focus on cross-platform compatibility and equipped with features and tools of production-grade quality, Suika2 is perfect for both beginners and seasoned creators alike.

## What are Suika2's Development Goals?

Suika2 aspires to set a new benchmark for future visual novel creation tools, facilitating creators throughout their journey of creation.
Effortlessness and efficiently are key concepts, and Suika2 provides that without sacrificing features or usability, providing tools
and features to enrich both the creator and user experience.

## What are Suika2's characteristics?

1. **Simplicity:** Suika2 enables the creation of Visual Novels in an easy and efficient manner.
2. **Speed:** Apps are constructed using native technologies, ensuring resource efficiency.
3. **Cost-free:** Suika2 is a FOSS (Free and Open Source Software) project, available for use by anyone without any hidden costs or fees.
4. **Portability:** Develop, distribute, and publish on almost any platform – Suika2 defines portability.
5. **Cultural Value:** We hold the belief that the creation of information is humanity’s true value.
6. **Ready for Commerce:** Game developers can publish their games on stores and earn income. Suika2 does not charge any royalty fees!
7. **Prosperity:** Our aim is to create a world where anyone can make a living with just a single computer.

## Downloads

* On Windows (Suika2 Pro Desktop):
  * Open [Microsoft Store](https://apps.microsoft.com/detail/XP99CQV05PR79W)

* On Mac (Suika2 Pro Desktop):
  * Open [Mac App Store](https://apps.apple.com/us/app/suika2-pro-desktop/id6474658254)

* On iPhone/iPad (Suika2 Pro Mobile):
  * Open [App Store](https://apps.apple.com/us/app/suika2-pro-mobile/id6474521680)

* On Android/Chromebook (Suika2 Pro Mobile):
  * Open [Google Play](https://play.google.com/store/apps/details?id=jp.luxion.suikapro)

* On Linux including Chromebook (Suika2 Pro Desktop):
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

Note that releases on GitHub and the official stores are infrequent.
Please visit [the official website](https://suika2.com/en/dl/) to obtain latest releases.

## Discord

Our community is a cherished and invaluable space that embraces developers, creators, and end-users irrespective of their nationality, language, ethnicity, color, lineage, beliefs, gender, education, age, religion, or identity.
That's why we'd love you to join our community!

<a href="https://discord.gg/ZmvXxE8GFg"><img src="https://discordapp.com/api/guilds/986623112617541677/widget.png?style=banner1"></a>

We hold a grand vision for cultural growth and engage in profound discussions, but at our core, we are simply developers and creators, so we are always interested in dialogue and conversation!

## Suika2 Pro

`Suika2 Pro Desktop` and `Suika2 Pro Mobile` are Suika2's creator tools, available for Windows PC, Mac, iPhone, iPad, Android phone/tablet, Chromebook and Linux.
They provide creators with all the functionality they need, such as editing, debugging, and exporting games out of the box.

## License

This software is released under the MIT license. There is no restriction on distribution and or modification of the Suika2 Source Code.

## Contribution

The best way to contribute to this project is to use it and give us feedback. We are always open to suggestions and ideas.

## Live Web Demo

<a href="https://suika2.com/run/sample/"><img src="https://github.com/suika2engine/suika2/raw/master/doc/img/screenshot.jpg"></a>

## Usage

* On Windows:
  * Download the `exe` file from [the official site](https://suika2.com/en/) and run it.
  * Click the `Suika2 Pro Desktop` icon on your desktop.

* On Mac:
  * Download the `dmg` file from [the official site](https://suika2.com/en/) and open it.
  * Copy the `Suika2 Pro Desktop` app to your `Applications` folder.
  * Open the copied app.

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

Games made with Suika2 can run on Windows PC, Mac, iPhone, iPad, Android phone/tablet, Web browser, Chromebook, Linux, *BSD, and some consoles.

Suika2 consists of a platform independent core and a hardware abstraction layer (HAL).
The core is written in ANSI C, the most portable programming language in the world, while HAL implementations are currently written in C, C++, Objective-C, and Java.

If you would like to port Suika2 to a new target platform, you only need to write a thin HAL, this is generally possible within a week.

Suika2 does not depend on SDKs or frameworks such as Unity, Godot, or SDL2, this is thanks to the extensive design of our compatibility layer's API.

## Continuous Integration (CI) and Continuous Deployment

* We do CI for build sanity checks on every push to the repository.
* We will do CD with Apple's Xcode Cloud.
* We currently don't have a way to release automatically for Windows and Android stores.

## Trivia

Did you know that...
* Midori wears a watermelon themed tie and pair of hair ribbons to stand out in her uniform?
* "Suika" means "watermelon" in Japanese?
* Suika2 is the successor to "Suika Studio":
  * [See the 2002 version here](https://github.com/ktabata/suika-studio-2002-gpl)
  * [See the 2003 version here](https://github.com/ktabata/suika-studio-2003-gpl)
  * The author is a pioneer in the field of GUI editors for visual novel creation.

## Sponsors

You've taken your stand, here's where you raise your banner!

**Come forward**, those who resonate with our vision and are of like mind.

Venite et videbitis.
