<!-- Big Banner -->
<img src="https://github.com/suika2engine/suika2/raw/master/doc/img/main_repo.png" width="100%">

## What is Suika2?

Suika2 is a development suite for Visual Novels.
It has an open-source philosophy and is designed with a true cross-platform portablity including iOS and Android.
It is equipped with features and tools of production-grade quality.
Suika2 is perfect for both beginners and seasoned creators alike.

Effortlessness and efficiently are key concepts of Suika2.
It provides that without sacrificing features or usability to enrich both the creator and user experience.

We don't aim for no-code but we achieve low-code using multiple DSLs, Domain Specific Languages.
Specifically, there are separate languages for scenarios, GUI, and logic.
We are in the process of adding a little graphical support for each DSL.
This is quite different from other engines and we call it "Visual Live Scripting" technology.
The author thinks as a researcher of software engineering that VLS is one shape of RAD, Rapid Application Development.

## What is the Suika2's Development Goal?

**"The New Standard for Visual Novel Creation"**, in the era of 2020s and beyond, is the goal of the Suika2 development.

## What are the Suika2's Mission, Vision and Values (MVV)?

* Mission
  * **Simple:** Suika2 enables the creation of Visual Novels in an easy and efficient manner.
  * **Fast:** Apps are constructed using native technologies only, ensuring resource efficiency on mobile devices.
  * **Free:** Suika2 is an OSS project and free for anyone without any hidden costs while it respects intellectual properties of others.
* Vision
  * **Earning:** Game developers can publish their games on stores and earn income.
  * **Prosperity:** Our aim is to create a world where anyone can make a living with just a single computer including mobile devices.
* Values
  * **Market:** We shall complete the market launch of game subscriptions with world leading platform partners.
  * **Cultural:** We hold the belief that the creation and generation of information including story writing is humanity's true value.
  * **Diverse:** Develop, distribute, and publish on all platforms - Suika2 seeks the true portability and we call it diversity.

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
Please visit [the official website](https://suika2.com/en/dl/) to obtain the latest release.

## Discord

Our community is a cherished and invaluable space that embraces developers, creators, and end-users irrespective of their nationality, language, ethnicity, color, lineage, beliefs, gender, education, age, religion, or identity.
That's why we'd love you to join our community!
You're definitely deserved to be a part of it.

<a href="https://discord.gg/ZmvXxE8GFg"><img src="https://discordapp.com/api/guilds/986623112617541677/widget.png?style=banner1"></a>

We hold a grand vision for cultural growth and engage in profound discussions, but at our core, we are simply developers and creators, so we are always interested in dialogue and conversation!

## Suika2 Pro series

`Suika2 Pro Desktop` and `Suika2 Pro Mobile` are Suika2's creator tools, available for Windows PC, Mac, iPhone, iPad, Android phone/tablet, Chromebook and Linux.
They provide creators with all the functionality they need, such as editing, debugging, and exporting games out of the box.

## License

This software is released under the MIT license. There is no restriction on distribution and or modification of the Suika2 Source Code.

## Contribution

The best way to contribute to this project is to use it and give us feedback.
We are always open to suggestions and ideas.

## Live Web Demo

<a href="https://suika2.com/run/sample/"><img src="https://github.com/suika2engine/suika2/raw/master/doc/img/screenshot.jpg"></a>

## Documentation

* [Japanese Documentation](https://suika2.com/wiki/?%E3%83%89%E3%82%AD%E3%83%A5%E3%83%A1%E3%83%B3%E3%83%88)
* [English Documentation](https://suika2.com/en/doc/)

## Portability

Games made with Suika2 can run on Windows PC, Mac, iPhone, iPad, Android phone/tablet, Web browser, Chromebook, Linux, FreeBSD, NetBSD, OpenBSD, and some consoles.

Suika2 consists of a platform independent core (CORE) and a hardware abstraction layer (HAL).
The CORE is written in ANSI C, the most portable programming language in the world, while HAL implementations are currently written in C, C++, Objective-C, and Java.

If you would like to port Suika2 to a new target platform, you only need to write a thin HAL, this is generally possible within a week.

Suika2 does not depend on SDKs or frameworks such as Unity, Godot, or SDL2, this is thanks to the extensive design of our compatibility layer's API.

## CI/CD (Continuous Integration and Continuous Deployment)

* We do CI for build sanity checks on every push to the repository.
* We currently don't have a way to do CD on the cloud due to a lack of signing capability.
* However, the author has a release script and releases are fully automated on his MacBook Pro.

## Trivia

Did you know that...
* Midori wears a watermelon themed tie and pair of hair ribbons to stand out in her uniform?
* "Suika" means "watermelon" in Japanese?
* Suika2 is the successor to "Suika Studio" and the author is a pioneer in the field of GUI editors for visual novel creation?
  * [See the 2002 version here](https://github.com/ktabata/suika-studio-2002-gpl)
  * [See the 2003 version here](https://github.com/ktabata/suika-studio-2003-gpl)

## Sponsors

You've taken your stand, here's where you raise your banner!

**Come forward**, those who resonate with our vision and are of like mind.

Venite et videbitis.
