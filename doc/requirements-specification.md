***

# Software Requirements Specification for Suika2

Author: ktabata

Modified By:

Organization: the Suika2 Development Team

Updated: 29th October 2022

Created: 27th October 2022

Revision: 0.4

***

# Table of Contents

- [Introduction](#introduction)
    - [The Necessity of This Document](#the-necessity-of-this-document)
- [File and Folders](#file-and-folders)
    - [`bg` Folder](#bg-folder)
    - [`bgm` Folder](#bgm-folder)
    - [`cg` Folder](#cg-folder)
    - [`ch` Folder](#ch-folder)
    - [`conf` Folder](#conf-folder)
    - [`cv` Folder](#cv-folder)
    - [`font` Folder](#font-folder)
    - [`gui` Folder](#gui-folder)
    - [`mov` Folder](#mov-folder)
    - [`rule` Folder](#rule-folder)
    - [`sav` Folder](#sav-folder)
    - [`se` Folder](#se-folder)
    - [`txt` Folder](#txt-folder)
    - [`data01.arc` File](#data01arc-file)
    - [`suika.exe` File](#suikaexe-file)
    - [`suika.app` Bundle](#suikaapp-bundle)
- [Main Screen Components](#main-screen-components)
    - [Background Layer](#background-layer)
    - [Character Layers](#character-layers)
    - [Message Box Layer](#message-box-layer)
        - [In-box Menu](#in-box-menu)
    - [Namebox Layer](#namebox-layer)
    - [Collapsed System Menu Layer](#collapsed-system-menu-layer)
    - [System Menu Layer](#system-menu-layer)
- [Command Features](#command-features)
    - [Showing a Background (`@bg`)](#showing-a-background-bg)
    - [Showing a Character (`@ch`)](#showing-a-character-ch)
    - [Showing a Message](#showing-a-message)
    - [Showing a Line](#showing-a-Line)
    - [Playing Background Music (`@bgm`)](#playing-background-music-bgm)
    - [Playing Sound Effect (`@se`)](#playing-sound-effect-se)
    - [Changing a Volume (`@vol`)](#changing-a-volume-vol)
    - [Showing Options (`@choose`)](#showing-options-choose)
    - [Defining a Label](#defining-a-label)
    - [Showing an Animation (`@cha`)](#showing-an-animation-cha)
    - [Changing Characters and Background at Once (`@chs`)](#changing-characters-and-background-at-once-chs)
    - [Shaking the Screen (`@shake`)](#shaking-the-screen-shake)
    - [Playing a Video (`@video`)](#playing-a-video-video)
    - [Showing a GUI (`@gui`)](#showing-a-gui-gui)
    - [Waiting for a Click (`@click`)](#waiting-for-a-click-click)
    - [Waiting for Specified Time (`@wait`)](#waiting-for-specified-time)
    - [Prohibiting Skip (`@skip`)](#prohibiting-skip-skip)
    - [Jumping to a Label (`@goto`)](#jumping-to-a-label-goto)
    - [Setting a Variable (`@set`)](#setting-a-variable-set)
    - [Branching by Variable (`@if`)](#branching-by-variable-if)
    - [Jumping to a Script (`@load`)](#jumping-to-a-script-load)
    - [Setting a Chapter Name (`@chapter`)](#setting-a-chapter-name-chapter)
    - [Call a Procedure (`@gosub`)](#call-a-procedure-gosub)
    - [Return From a Procedure (`@return`)](#return-from-a-procedure-return)
    - [Multilingualization Prefix](#multilingualization-prefix)
    - [Calling an Advanced Script](#calling-an-advanced-script)
- [Variables](#variables)
    - [Local Variables](#local-variables)
    - [Global Variables](#local-variables)
- [Seen Flags](#seen-flags)
- [Main Screen](#main-screen)
    - [Background Changing Mode](#background-changing-mode)
    - [Character Changing Mode](#character-changing-mode)
    - [Text Output Mode for Message](#text-output-mode-for-message)
    - [Click Waiting Mode for Message](#click-waiting-mode-for-message)
    - [Sound Fading Mode](#sound-fading-mode)
    - [Timed Waiting Mode](#timed-waiting-mode)
    - [Click Waiting Mode](#click-waiting-mode)
    - [Auto Mode](#auto-mode)
    - [Skip Mode](#skip-mode)
- [Special Screens](#special-screens)
    - [Save Screen](#save-screen)
    - [Load Screen](#load-screen)
    - [History Screen](#history-screen)
    - [Config Screen](#config-screen)
- [Graphical User Interface](#graphical-user-interface)
    - [GUI File Header](#gui-file-header)
    - [Jump To Label Button](#jump-to-label-button)
    - [Save/Load Page N Button](#save-load-page-n-button)
    - [Save Slot Button](#save-slot-button)
    - [Load Slot Button](#load-slot-button)
    - [Text Speed Slider](#text-speed-slidebar)
    - [Auto Mode Speed Slider](#auto-mode-speed-slider)
    - [Text Speed Preview](#text-speed-preview)
    - [BGM Volume Slider](#bgm-volume-slider)
    - [Sound Effect Volume Slider](#sound-effect-volume-slider)
    - [Voice (All-character) Volume Slider](#voice-all-character-volume-slider)
    - [Voice (Per-character) Volume Slider](#voice-per-character-volume-slider)
    - [Font Selection Button](#font-selection-button)
    - [Full Screen Button](#full-screen-button)
    - [Window Button](#window-button)
    - [Reset Button](#reset-button)
    - [Page Button](#page-button)
    - [Jump To File Button](#jump-to-file-button)
    - [Cancel Button](#cancel-button)
    - [Gallery Button](#gallery-button)
- [Configuration Features](#configuration-features)
    - [Language Settings](#language-settings)
        - [International Mode](#international-mode)
        - [Language Mapping](#language-mapping)
    - [Window Settings](#window-settings)
        - [Window Title](#window-title)
        - [Window Width and Height](#window-width-and-height)
        - [Background Color](#background-color)
    - [Font Settings](#font-settings)
        - [Font File Name](#font-file-name)
        - [Font Size](#font-size)
        - [Font Colors](#font-colors)
        - [Font Outline](#font-outline)
    - [Namebox Settings](#namebox-settings)
        - [Namebox Image](#namebox-image)
        - [Namebox Position](#namebox-position)
        - [Namebox Top Margin](#namebox-top-margin)
        - [Namebox Left Margin](#namebox-left-margin)
        - [Namebox Centering](#namebox-centering)
        - [Namebox Visibility](#namebox-visibility)
    - [Message Box Settings](#message-box-settings)
        - [Message Box Position](#message-box-position)
        - [Message Box Margins](#message-box-margins)
        - [Message Box Text Speed](#message-box-text-speed)
        - [Position of Hide Button](#position-of-hide-button)
        - [Message Box Sound Effects](#message-box-sound-effects)
        - [Skipping Unseen Massages](#skipping-unseen-messages)
        - [Dimming Seen Messages](#dimming-seen-messages)
    - [Click Animation Settings](#click-animation-settings)
        - [Click Animation Position](#click-animation-position)
        - [Following The Text](#following-the-text)
        - [Click Animation Images](#click-animation-images)
        - [Click Animation Interval](#click-animation-interval)
        - [Click Animation Visibility](#click-animation-visibility)
    - [Options Settings](#options-settings)
        - [Option Box Images](#option-box-images)
        - [First Option Position](#first-option-position)
        - [Vertical Space Between Options](#vertical-space-between-options)
        - [Option Top Margin](#option-top-margin)
        - [Pointed Option Color](#pointed-option-color)
        - [Options Sound Effects](#options-sound-effects)
    - [Save/Load Screen Settings](#save-load-screen-settings)
        - [Save Item Thumbnail Size](#save-item-thumbnail-size)
    - [System Menu Settings](#system-menu-settings)
        - [System Menu Position](#system-menu-position)
        - [System Menu Images](#system-menu-images)
        - [System Menu Button Positions](#system-menu-button-positions)
        - [System Menu Sound Effects](#system-menu-sound-effects)
        - [Collapsed Position](#collapsed-position)
        - [Collapsed Images](#collapsed-images)
        - [Collapsed Sound Effect](#collapsed-sound-effect)
        - [System Menu Visibility](#system-menu-visibility)
    - [Auto Mode Settings](#auto-mode-settings)
        - [Auto Mode Banner Image](#auto-mode-banner-image)
        - [Auto Mode Banner Position](#auto-mode-banner-position)
        - [Auto Mode Speed](#auto-mode-speed)
    - [Skip Mode Settings](#skip-mode-settings)
        - [Skip Mode Banner Image](#skip-mode-banner-image)
        - [Skip Mode Bannder Position](#skip-mode-banner-position)
    - [Initial Sound Volumes](#initial-sound-volumes)
    - [Per-character Sound Volumes](#per-character-sound-volumes)
    - [Character Message Colors](#character-message-colors)
    - [UI Messages](#ui-messages)
    - [Miscellaneous](#miscellaneous)
        - [Voice Continues On Click](#voice-continues-on-click)
        - [Full Screen Mode](#full-screen-mode)
        - [Window Maximization](#window-maximization)
        - [Window Title Separator](#window-title-separator)
        - [Chapter Name](#chapter-name)
        - [Show Message Box On Character Change](#show-message-box-on-character-change)
        - [Release Mode](#release-mode)
- [Package](#package)
    - [Package Generation](#package-generation)
    - [Package Obfuscation](#package-obfuscation)
    - [Package Usage](#package-usage)
- [File Formats](#file-formats)
    - [Text File Format](#text-file-format)
    - [Image File Formats](#image-file-formats)
    - [Audio File Format](#audio-file-format)
    - [Video File Formats](#video-file-formats)
- [Advanced Script](#advanced-script)
    - [Syntax and Usage](#syntax-and-usage)
        - [Defining and Calling Functions](#defining-and-calling-functions)
        - [Types and Variables](#types-and-variables)
        - [Loops](#loops)
        - [Branches](#branches)
        - [Arrays](#arrays)
        - [Conversions](#conversions)
    - [Calling Suika2 Engine](#calling-suika2-engine)
        - [Showing an Alert](#showing-an-alert)
- [Non-functional Requirements](#non-functional-requirements)
    - [Performance Requirements](#performance-requirements)
        - [Native Application](#native-application)
        - [CPU Usage](#cpu-usage)
        - [Fast Math](#fast-math)
        - [GPU](#gpu)
        - [Frame Rate](#frame-rate)

***

# Introduction

Suika2 is a tool for creating visual novels.
It is cross-platform software, compatible with, but not limited to, Windows, Mac, Linux, iOS, Android and Web browsers.
This document outlines the software requirements specification (SRS) for Suika2.

## The Necessity of This Document

This section explains why the SRS is needed for Suika2, an open-source software (OSS) project.

This project has grown with an emphasis on documentation and translation.
To our delight, the project has several sources of documentation: the main repository, the old website, the new website, their Japanese and Chinese translations, and the Wiki.
These documents may cause inconsistencies in the future, with minute inconsistencies as of the writing of this document.

Therefore, the project decided to create an SRS that would be the principal source of all documentation,
and from which other documentation would be adapted and created.

If a description of a specification that exists in another document does not exist in the SRS, it must be added to the SRS.

***

# Files and Folders

* Game Folder
    * bg/
    * bgm/
    * cg/
    * ch/
    * conf/
    * cv/
    * font/
    * gui/
    * rule/
    * sav/
    * se/
    * txt/
    * data01.arc
    * suika.exe
    * suika.app

## `bg` Folder

This folder is for background image files.

## `bgm` Folder

This folder is for background music files

## `cg` Folder

This folder is for the following images.
* Message box
* Namebox
* System menu
* Collapsed system menu
* Option box
* Click animation
* GUI (including save, load, history, config screens)
* Auto mode banner
* Skip mode banner

## `ch` Folder

This folder is for character sprites.

## `conf` Folder

This folder is for config files, namely, `config.txt`.

## `cv` Folder

This folder is for character voice files.

## `font` Folder

This folder is for font files.

## `gui` Folder

This folder is for GUI definition files.

## `rule` Folder

This folder is for rule image files which are used in rule-based transitions of
background and character images.

## `sav` Folder

This folder is for save data. It is automatically generated by Suika2.
When release mode is enabled,
this folder is not generated in the game folder.
See further details in the "Release Mode" section.

## `se` Folder

This folder is for sound effect files.

## `data01.arc` File

This is a package file.
See further details in the "Package" section.

## `suika.exe` File

This is an application for Windows.

## `suika.app` Bundle

This is an application for macOS.

***

# Main Screen Components

In main screen (See further details in the ("Main Screen")[#main-screen] section),
screen components are layered.
When referring to all layers, the term 'stage' is used.

The stage consists of the following, in order from bottom to top:

1. Background Layer
2. Back Center Character Layer
3. Left Character Layer
4. Right Character Layer
5. Center Character Layer
6. Message Box Layer
7. Namebox Layer
8. Character Face Layer
9. Click Animation Layer
10. Options Layer
11. Collapsed System Menu Layer
12. System Menu Layer

## Background Layer

The background image is the lowest layer that makes up the screen.
Its size must be equal to that of the window.

## Character Layers

1. `center` (Center Character)
2. `right` (Right Character)
3. `left` (Left Character)
4. `back` (Back Center Character)
5. `face` (Character Face)

There are four regular character types
depending on their horizontal display position
(`center`, `right`, `left` and `back).

The `center` and `back` charater positions have the same horizontal display position,
but the layers displayed are different.

There is a position for the character's `face` to be displayed to
the left of the message box.

All characters are vertically aligned at the bottom of the screen.

## Message Box Layer

The message box is a window for outputting text.

It is made up of a set of "bg" (background) and "fg" (foreground) images.

In simple terms, the bg image is displayed by default (often referred to as an "idle" image).

When a developer creates an "In-box Menu" and a game user points (hovers) over a button,
the button area of the fg image is substituted in.

Given that the fg image must be specified, a developer may choose to use the same image
for both the fg and bg images. This is most often the case when the message box does not
include a menu.

### In-box Menu

The message box can have menu buttons.
Acceptable button types are:

1. `qsave`
2. `qload`
3. `save`
4. `load`
5. `history`
6. `auto`
7. `skip`
8. `config`
9. `hide`

The current demo game only uses the `hide` button.
If developers want to use the other buttons,
they can simply provide appropriate message box images and add some configs.

## Namebox Layer

The namebox is a window for outputting a character's name.

Names are usually horizontally centered, but can be left-aligned by config.

The namebox can be hidden by a config
in order to realize a full-screen style novel.

## Collapsed System Menu Layer

This is a button that is visible during the display of messages and choices.
Clicking this button opens the "System Menu".
The collapsed system menu is invisible and disabled while
the "System Menu" is visible.

## System Menu Layer

This is a menu that consists of buttons.
Acceptable button types are:

1. `qsave`
2. `qload`
3. `save`
4. `load`
5. `history`
6. `auto`
7. `skip`
8. `config`

The system menu is activated by clicking on the collapsed system menu or
by right-clicking when a message or option(s) is displayed.

While the system menu is displayed,
right-clicking or clicking anywhere other than the buttons will exit
the system menu and return to the collapsed system menu.

***

# Command Features

An important building block of Suika2's functionality are "commands".
Therefore, more than half of the functional requirements are found in the design of commands.

See details regarding command implementation in the "Command Reference".

## Showing a Background (`@bg`)

This feature shows a background image.
The command that accomplishes this feature is `@bg`.

The `@bg` command changes the background image.
It fades in a new background image over a specified time in seconds.

Background image files need to be stored in the [`bg`](#bg-folder) folder.
The hex RGB color specifier (e.g., `#0088ff`) may be used instead of an image file name.

When `@bg` is run, the originally displayed background as well as any character images
are removed from the stage.
Game developers can use [`@chs`](#changing-characters-and-background-at-once-chs) instead of `@bg`
when they want to keep character images on the stage.

If you are looking for a way to keep your characters from disappearing when
you change the background, write the following command:


Fade-ins are accompanied by transition effects.
The following is the list of effects types.

|Effect Type                               |Effect Name         |Abbrev. Name 1|Abbrev. Name 2 |
|------------------------------------------|--------------------|--------------|---------------|
|fade/dissolve (alpha blending)            |`normal`            |`n`           |(not-specified)|
|rule-based transition                     |`rule:file-mame`    |              |               |
|right curtain                             |`curtain-right`     |`curtain`     |`c`            |
|left curtain                              |`curtain-left`      |`cl`          |               |
|up curtain                                |`curtain-up`        |`cu`          |               |
|down curtain                              |`curtain-down`      |`cd`          |               |
|right slide                               |`slide-right`       |`sr`          |               |
|left slide                                |`slide-left`        |`sl`          |               |
|up slide                                  |`slide-up`          |`su`          |               |
|down slide                                |`slide-down`        |`sd`          |               |
|right shutter                             |`shutter-right`     |`shr`         |               |
|left shutter                              |`shutter-left`      |`shl`         |               |
|up shutter                                |`shutter-up`        |`shu`         |               |
|down shutter                              |`shutter-down`      |`shd`         |               |
|clockwise wipe                            |`clockwise`         |`cw`          |               |
|counterclockwise wipe                     |`counterclockwise`  |`ccw`         |               |
|clockwise wipe (20 degrees stepped)       |`clockwise20`       |`cw20`        |               |
|counterclockwise wipe (20 degrees stepped)|`counterclockwise20`|`ccw20`       |               |
|clockwise wipe (30 degrees stepped)       |`clockwise30`       |`cw30`        |               |
|counterclockwise wipe (30 degrees stepped)|`counterclockwise30`|`ccw30`       |               |
|open eyes                                 |`eye-open`          |              |               |
|close eyes                                |`eye-close`         |              |               |
|open eyes (vertical)                      |`eye-open-v`        |              |               |
|close eyes (vertical)                     |`eye-close-v`       |              |               |
|open slit                                 |`slit-open`         |              |               |
|close slit                                |`slit-close`        |              |               |
|open slit (vertical)                      |`slit-open-v`       |              |               |
|close slit (vertical)                     |`slit-close-v`      |              |               |
|rule (universal transition/1-bit)         |`rule:<rule-file>`  |              |               |
|melt (universal transition/8-bit)         |`melt:<rule-file>`  |              |               |

The `mask` Effect type was removed as the `rule` effect type was introduced.
The demo includes a `rule-mask.png` file as a rule image corresponding to `mask`, therefore `rule:rule-mask.png` works fine.

### Usage 1

The following script changes the background image to `sample.png` immediately.
```
@bg sample.png
```

### Usage 2

The following script changes the background image to `sample.png` with a 1.5 second fade-in time.
The applicable effect type is "fade/dissolve (alpha blending)", which is default.
```
@bg sample.png 1.5
```

### Usage 3

The following script changes the background image to `sample.png` with a 1.5 second fade-in time.
The applicable effect type is "right curtain", which is like closing a curtain in the right direction.
```
@bg sample.png 1.5 curtain-right
```

### Usage 4

The following script changes the background to the color black with a 1.5 second fade-in time.
The applicable effect type is "close eyes", which is like closing eyes slowly.
```
@bg #000000 1.5 eye-close
```

### Usage 5

The following script changes the background image to `sample.png` with a 1.5 second fade-in time.
The applicable effect type is "rule (universal transition/1-bit)", which uses `rule-star.png` image for transition rule.
```
@bg sample.png 1.5 rule:rule-star.png
```

### Usage 6

The following script changes the background image to `sample.png` with a 1.5 second fade-in time.
The applicable effect type is "melt (universal transition/8-bit)", which uses `rule-star.png` image for transition rule.
```
@bg sample.png 1.5 melt:rule-star.png
```

## Showing a Character (`@ch`)

This feature shows a character image.
The command that accomplishes this feature is `@ch`.

The `@ch` command changes the character.
Character image files need to be in the [`ch`](#ch-folder) folder.

Character layers include `center` (center front), `right`, `left`,
`back` (center back) and `face` (character face).

|Display Position  |Target Name|Abbrev. Name|
|------------------|-----------|------------|
|Front Center      |`center`   |`c`         |
|Right             |`right`    |`r`         |
|Left              |`left`     |`l`         |
|Back Center       |`back`     |`b`         |
|Face Icon         |`face`     |`f`         |

The effect types are the same as "Showing a Background (`@bg`)".

All characters are vertically aligned at the bottom of the screen.

### Usage 1

The following script displays `sample.png` immediately in the front center position.
Note that `center` can be abbreviated as `c`.
```
@ch center sample.png
```

### Usage 2

The following script displays `sample.png` in 0.5 seconds in the front center position.
The applicable effect type is "fade/dissolve (alpha blending)", which is default.
```
@ch center sample.png 0.5
```

### Usage 3

The following script removes the front center character in 0.5 seconds.
The applicable effect type is "fade/dissolve (alpha blending)", which is default.
```
@ch center none 0.5
```

### Usage 4

The following script displays `sample.png` in 1.0 second in the front center position.
The applicable effect type is "clockwise wipe".
```
@ch center sample.png 1.0 clockwise
```

### Usage 5

The following script displays `sample.png` in 1.0 second in the front center position.
Here, `100 50` means offset, shifting the display position 50 pixels to the right and 50 pixels down.
`normal` means the effect type "fade/dissolve (alpha blending)".
```
@ch center sample.png 1.0 normal 100 50
```

### Usage 6

The following script displays `sample.png` in 1.0 second in the front center position with the alpha value `127` (50%).
The alpha value ranges from `0` to `255`.
For the alpha value specifier, game developers can write `show` as an equivalent to `255` or `hide` as `0`.
```
@ch center sample.png 1.0 normal 0 0 127
```

## Showing a Message

This feature shows the descriptive part of the story.
It is not an atmark command that makes this feature possible.
A message is one-line of text that does not begin with an atmark ("`@`"),
asterisk ("`*`") or colon ("`:`").

Messages are output to a message box.
As the descriptive part does not have the speaker's name,
the namebox is hidden while the message is showing.

A messages is displayed one character at a time.
When clicked in the middle, all characters are displayed at once.
When all characters are displayed, Suika2 goes into
"Click Waiting Mode for Message" and displays a click animation prompting click-input.
If a click is made while a click animation is being displayed,
Suika2 moves to the next command.

At any point during the display of a message,
unless if the control key is pressed and the message is unread,
Suika2 moves to the next command.

The following format specifiers are available.

* `\n` ... Insert a new line.
* `$ + number` ... Print the value of the variable (e.g., `$1`).
* `\` at the beginning of the line ... Append the message to the previous message. (append mode)

Append mode is useful for the creation of full-screen style novels.

End users can use the return key and down key instead of left click.

### Usage
```
Hello, world!
```

## Showing a Line

This feature shows the dialog of the story.
It is not an atmark command that makes this feature possible.
A line is one-line text that begin with an asterisk.

This command prints text to the message box and also the character name to the name box.
It consists of a character name, a message and optionally a voice file name,
and they are separated by asterisks. For example:
```
*name*dialog message
*name*voice.ogg*dialog message
```

The name appears before the message.

Audio starts playing when the name is displayed.
Normally, audio playback is stopped before moving on to the next command,
but this behavior can be suppressed in the config.

If `@` is added to the beginning of a voice file name,
the audio playback is looped for the length of the message.

Other functions are exactly the same as ["Showing a Background (`@bg`)"](#showing-a-backgounrd-bg).

## Playing Background Music (`@bgm`)

This feature plays background music.
The command that accomplishes this feature is `@bgm`.

The `@bgm` command plays background music (BGM).
BGM files need to be stored in the `bgm` folder.

Unless otherwise specified, BGM is played in a loop.
Game developers can use the `once` specifier to prevent BGM from looping.

The ability to crossfade multiple BGMs is not available.
Therefore, game developers may first need to fade out the old BGM and then fade in the new BGM.

### Usage 1

The following script plays `sample.ogg` as BGM. Playback is looped.
```
@bgm sample.ogg
```

### Usage 2

The following script stops BGM immediately.
```
@bgm stop
```

### Usage 3

The following script plays `sample.ogg` as BGM. Playback is not looped.
```
@bgm sample.ogg once
```

### Usage 4 (Application)

The following sample script fades-out the BGM in 2 seconds.
```
@vol bgm 0.0 2.0
@wait 2.0
@bgm stop
@vol bgm 1.0
```

## Playing Sound Effect (`@se`)

This feature plays a sound effect.
The command that accomplishes this feature is `@se`.

The `@se` command plays sound effects.
Sound effect files need to be in the `se` folder.

### Usage 1

The following script starts the playback of a sound effect file.
```
@se click.ogg
```

### Usage 2

The following script stops the playback of a sound effect file.
```
@se stop
```

### Usage 3

The following script loops a sound effect file.
```
@se sample.ogg loop
```

### Usage 4

The following script plays a sound effect file in the `voice` track.
This feature is available for checking voice-volume without a text message.
```
@se click.ogg voice
```

## Changing a Volume (`@vol`)

This feature changes the volume.
The command that accomplishes this feature is `@vol`.

The `@vol` command sets the sound volume.
Suika2 has three independent sound tracks:

* `bgm` or `b` for the BGM track
* `voice` or `v` for the character voice track
* `se` or `s` for the sound effect track

The volume of the specified sound track will fade in/out for the specified time.
This command does not wait until the fade in/out is complete.
Combining with the ["Waiting for Specified Time (`@wait`)"](#waiting-for-specified-time-wait) feature is useful if necessary.

`bgm`, `voice` and `se` tracks are for "local" volumes and there are "global" volumes.
Local volumes are stored in each save data file.
Global volumes are common to all save data files.
The final volume will be a multiplication of the local and global volumes.
To specify the global volumes, game developers can use capital `BGM`, `VOICE` and `SE` tracks.
Since the global volume changes immediately, it is not possible to specify a fade time.

Both the value of local and global volumes range 0.0 to 1.0.

### Usage 1

The following script sets the local volume of BGM to 0.5 (50%) in 1.0 second.
```
@vol bgm 0.5 1.0
```

### Usage 2

The following script sets the global volume of BGM to 0.5 (50%) immediately.
Usually, the global volumes are set from the config screen,
so the use of this feature is not as common.
```
@vol BGM 0.5 0
```

## Showing Options (`@choose`)

This feature shows options and lets the user choose one option,
then jumps to the label selected.
The command that accomplishes this feature is `@choose`.

The `@choose` command can display up to eight options.
A minimum of one option must be displayed.

### Usage

The following script shows options.

```
@choose label1 "Good morning." label2 "Good afternoon." label3 "Good evening."
:label1
Good morning.
@goto end
:label2
Good afternoon.
@goto end
:label3
Good evening.
:end
```

## Defining a Label

This is a jump target inside a script.
It is not an atmark command, but one-line of text that begin with a colon.

This command can be used as a jump target,
which is typically used with the [`@choose`](#showing-options-choose) command,
[`@goto`](#jumping-to-a-label-goto) command,
[`@if`](#branching-by-variable-if) commands and
the [GUI](#graphical-user-interface) buttons.

### Usage

The following script creates a loop.
```
:JumpTarget
Show some messages.
@goto JumpTarget
```

## Showing an Animation (`@cha`)

This feature displays a character sprite in motion.
The command that accomplishes this feature is `@cha`.

The `@cha` command translates one character sprite at a time
and can also change its alpha value.
Refer to the "Showing a Character Image (`@ch`)" section for details on how to specify the character position and the alpha value.

Constant velocity (`move`), acceleration (`accel`) and deceleration (`brake`) can be used for translation.

`@cha` is an abbreviation of "character animation".

### Usage 1

The following script moves the character in the front center position 600 pixels to the left and hides it with an animation time of 1.0 second.
```
@cha center 1.0 move -600 0 hide
```

### Usage 2

The following script moves the character in the front center position 600 pixels to the left and hides it with an animation time of 1.0 second.
This script is the same as Usage 1, but this one uses acceleration.
```
@cha center 1.0 accel -600 0 hide
```

### Usage 3

The following script moves the character in the front center position 600 pixels to the left and hides it with an animation time of 1.0 second.
This script is the same as Usage 1, but this one uses deceleration.
```
@cha center 1.0 brake -600 0 hide
```

### Usage 4

The following script preloads the character image out of screen, then moves it into the screen.
```
@ch right sample.png 0 normal 600 0 hide
@cha right 2.0 move -600 0 show
```

## Changing Characters and Background at Once (`@chs`)

This feature changes character sprites and the background image all at once.
The command that accomplishes this feature is `@chs`.

The `@chs` command has the ability to change just the character(s) or just the background, or all of these.

The character specification order is `center`, `right`, `left` and `back`.
The effect types are the same as ["Showing a Background (`@bg`)"](#showing-a-background-bg).

"chs" is an abbreviation of "change stage".

### Special Usage (Change Only Background)

The following script changes background image while leaving the characters in place with a 1.0 second fade time.
```
@chs stay stay stay stay 1.0 background.png
```

### Usage 1

The following script changes front center and right characters with a 1.0 second fade time.
Other characters will not be changed.
```
@chs center.png right.png stay stay 1.0
```

### Usage 2

The following script vanishes the character in the front center position with a 1.0 second fade time.
Other characters will not be changed.
```
@chs none stay stay stay 1.0
```

### Usage 3

The following script changes the background without any changes to the characters with a 1.0 second fade time.
```
@chs stay stay stay stay 1.0 background.png
```

### Usage 4

The following script changes the character in the front center position as well as the background with a 1.0 second fade time.
The effect type is "right curtain".
```
@chs center.png stay stay stay 1.0 background.png curtain-right
```

## Shaking the Screen (`@shake`)

This feature shakes the screen.
The command that accomplishes this feature is `@shake`.

The `@shake` command allows the screen to oscillate horizontally or vertically.
The oscillation is a "simple harmonic motion".
The period and the amplitude can be specified.

### Usage 1

The following script shakes the screen horizontally by 100px three times over a period of 1.0 second.
`horizontal` can be abbreviated as `h`.
```
@shake horizontal 1.0 3 100
```

### Usage 2

The following script shakes the screen vertically by 100px three times over a period of 1.0 second.
`vertical` can be abbreviated as `v`.
```
@shake vertical 1.0 3 100
```

## Playing a Video (`@video`)

This feature plays a video file.
The command that accomplishes this feature is `@video`.

The `@video` command plays a video file (if the file format is supported by the platform).
Available video formats vary from platform to platform,
but will be standardized to mp4 in the future.
At the moment, `.wmv` is preferred on Windows and `.mp4` is preferred on other platforms.
Note that on Linux, the end user must have the Gstreamer plugin which the game developer intended.

Video files are stored in the [`mov`](#mov-folder) folder.
The `mov` folder is not stored in the `data01.arc` file.

For further details, see the ["File Formats"](#file-formats) section.

### Usage

The following script plays a video file.
```
@video sample.avi
```

## Showing a GUI (`@gui`)

This feature shows a graphical user interface (GUI).
The command that accomplishes this feature is `@gui`.

A GUI is a menu consisting of three images
and can show up to 128 buttons on the screen.
Button types include "jump to label", "show if variable is set", etc.

GUI definition files are also used for the config, save, load and history screens.

For further details, see the ["Graphical User Interface"](#graphical-user-interface) section.

### Usage 1

The following script shows GUI `menu.txt`.
```
@gui menu.txt
```

### Usage 2

The following script show GUI `menu.txt`.
It allows cancellation by right click.
```
@gui menu.txt cancel
```

## Waiting for a Click (`@click`)

This feature waits for a user click.
The command that accomplishes this feature is `@click`.

During auto mode, Suika2 resumes after a 2-second pause.
Also see the ["Click Waiting Mode"](#click-waiting-mode) section.

### Usage

The following script waits for a click.
```
@click
```

## Waiting for Specified Time (`@wait`)

This feature waits for a specified time in seconds.
The command that accomplishes this feature is `@wait`.

If a click or keystroke is made, the wait is canceled.

If the previous command is a message,
the message box is visible while waiting for input,
otherwise, the message box is hidden while waiting for input.

Also see ["Timed Waiting Mode"](#timed-waiting-mode)

### Usage

The following script waits for 5 seconds.
```
@wait 5.0
```

## Prohibiting Skip (`@skip`)

This feature enables/disables skip by control key and skip mode.
The command that accomplishes this feature is `@skip`.

The `@skip` command enables or disables the ability to skip timed commands by way of user interaction.
Typically, this command is used to display a brand logo on boot time.

In non-skippable mode,
mid-flight skipping of message display is inhibited,
but to use this,
you have to use `@setsave disable` to inhibit the save/load screen.
This is because the state of non-skippable mode is not recorded to the save data files.
In addition, don't use `@goto $LOAD` and `@goto $SAVE` while non-skippable mode is active.
The reason is the same as above.

### Usage 1

The following script enables skip. (default/skippable-mode)
```
@skip enable
```

### Usage 2

The following script disables skip. (non-skippable mode)
```
@skip disable
```

## Jumping to a Label (`@goto`)

This feature moves the script execution position to the specified label.
The command that accomplishes this feature is `@jump`.

Currently, there is no way to jump directly to labels in other scripts.

Suika2 behaves in a special way when `$LOAD` or `$SAVE` is specified as the label for the jump destination.

### Usage 1

The following script jumps to the label `abc` (thus creating a loop).
```
:abc
Describe the process here.
@goto abc
```

### Usage 2

The following script shows the load screen.
```
@goto $LOAD
```

### Usage 3

The following script shows the save screen.
```
@goto $SAVE
```

## Setting a Variable (`@set`)

This feature sets variable values.
The command that accomplishes this feature is `@set`.

The `@set` command can perform calculations as well as simple assignments.
Addition, subtraction, multiplication and remainder are possible.
Available operators are:

* `=` (assignment)
* `+=` (addition)
* `-=` (subtraction)
* `*=` (multiplication)
* `/=` (division)
* `%=` (remainder)

On RHS, `$RAND` can be specified for a random number.

Local variables range from `$0` to `$9999`. They are stored independently in each save data file.
Global variables range from `$10000` to `$10999`. They are stored globally across all save data files.
All variables have 32-bit integer values and are initialized as `0`.

See also ["Variables"](#variables).

### Usage 1

The following script sets the value of `1` to the variable `$0`.
```
@set $0 = 1
```

### Usage 2

The following script adds the value `23` to the variable `$10`.
```
@set $10 += 23
```

### Usage 3

The following script assigns a random number (from 0 to 2147483647) to the variable `$0`.
```
@set $0 = $RAND
```

### Usage 4

The following script adds the value of the variable `$2` to the variable `$1`.
```
@set $1 += $2
```

## Branching by Variable (`@if`)

This feature moves the script execution position to the specified label
if the specified condition of a preset variable is met.
The command that accomplishes this feature is `@if`.

Available operators are:

|Operator|Meaning                 |
|--------|------------------------|
|`>`     |greater than            |
|`>=`    |greater than or equal to|
|`==`    |equal to                |
|`<=`    |less than or equal to   |
|`<`     |less than               |
|`!=`    |not equal to            |

LHS must be a variable name (e.g., `$1`).

RHS must be an integer or a variable name.

### Usage

The following script jumps to the label `abc` if the variable at number 1 is equal to `1`.
```
@if $1 == 1 abc

Story when flag is not set.

@goto END
:abc

Story when flag is set.

@goto END
:END

Stories join here.
```

## Jumping to a Script (`@load`)

This feature loads a script file.
The command that accomplishes this feature is `@load`.

Script files need to be in the [`txt`](#txt-folder) folder.

### Usage

The following script jumps to the script file `001.txt`.
```
@load 001.txt
```

## Setting a Chapter Name (`@chapter`)

This feature sets a chapter name.
The command that accomplishes this feature is `@chapter`.

The name of the chapter is reflected in the window title and saved data items.

### Usage

The following script sets the chapter name.
```
@chapter "Chapter 1"
```

## Call a Procedure (`@gosub`)

This feature calls a subroutine.
The command that accomplishes this feature is `@gosub`.

Note that nested calls of subroutines are not supported.

### Usage

The following script runs subroutine `SUB`.
```
@gosub SUB
@goto END
:SUB
Describe the process here.
@return
:END
```

## Return From a Procedure (`@return`)

This feature returns from a subroutine.
The command that accomplishes this feature is `@return`.

### Usage

The following script runs subroutine `SUB`.
```
@gosub SUB
@goto END
:SUB
Describe the process here.
@return
:END
```

## Multilingualization Prefix

This feature automatically switches the language to be displayed
depending on the user's system locale.

To use this feature, a script line must be started with a plus sign.

```
+en+Hello.
+fr+Bonjour.
```

This is not exactly a command, but it is a prefix to a command.

See also ["International Mode"](#international-mode) and ["Language Mapping"](#language-mapping).

## Calling an Advanced Script

This feature executes an advanced script called `WMS`.
The command that accomplishes this feature is `@wms`.

WMS files need to be in the [`wms`](#wms-folder) folder.

For further details, see the ["Advanced Script](#advanced-script) section.

### Usage

The following script executes an advanced script named `sample.scr` in the `wms` folder.
```
@script sample.scr
```

***

# Variables

All variables are 32-bit integers, and are initialized as `0`.

## Local Variables

Variables ranging from `$0` to `$9999` are local variables, these are stored in each
save data file.
Typically, these are used to branch scenarios.

## Global Variables

Variables ranging from `$10000` to `$10999` are global variables that are common to
all save data files.
Typically, these are used to record which pictures should be displayed
in the gallery.

***

# Seen Flags

A seen flag is stored for each message.

Unseen messages cannot be skipped.
However, game developers can set an option to modify this behaviour.

The reason why the term "seen" is used instead of "read" is that
"read" appears to be an instruction to the computer.

Seen flags are stored in the `sav` folder.
They are grouped into individual scripts.
The file names of the seen flags are a hex representation of
the script file name.

***

# Main Screen

Here, the screen during normal gameplay is referred to as the main screen.
See "Special Screens" for other screens.

The main screen has some modes.

## Background Changing Mode

This mode changes the background image.

While this mode is active, the message box is not displayed.

## Character Changing Mode

This mode changes the character images and the background image.

While the `@ch` command changes one character at a time,
the `@chs` command changes 0 or more characters and can optionally change the background.

While this mode is active, the message box is not displayed,
however, game developers can optionally set a config to display the message box
while this mode is active.

## Text Output Mode for Message

In this mode, text is output one character at a time to the message box.

If the message has a character's name,
the name of the character is output to namebox prior to this mode.

When the text output is complete, Suika2 enters "Click Waiting Mode for Message".

When a click or enter key is pressed during text output, Suika2 enters "Click Waiting Mode for Message".

When the control key is pressed, Suika2 moves to the next command.

## Click Waiting Mode for Message

In this mode, a prompt animation appears and waits for the user to click.
If not in auto mode, this mode ends when there is a click or keystroke.
If in auto mode, this mode ends when the playback of the voice is completed
or the waiting time based on the number of characters has completed.

This mode is not executed when in skip mode and the message is already seen.

## Sound Fading Mode

This mode proceeds simultaneously alongside other modes.
If necessary, game developers can make use of a timed wait for completion of fading.

## Timed Waiting Mode

This mode waits for the specified amount of time in seconds.
If a click or keystroke is made, the wait is canceled.

## Click Waiting Mode

This mode stops action until a user click or keystroke is made.
During auto mode, Suika2 resumes after a 2-second pause.

## Auto Mode

This mode proceeds simultaneously alongside other modes.
This mode effects "Click Waiting Mode for Message".
See also "Click Waiting Mode for Message".

During this mode, a banner representing auto mode is displayed.

## Skip Mode

This mode proceeds simultaneously alongside other modes.
This mode effects "Click Waiting Mode for Message".
See also "Click Waiting Mode for Message".

During this mode, a banner representing skip mode is displayed.

***

# Special Screens

Apart from the main screen (normal layered game screen),
there are some special screens.

These special screens are realized by the graphical user interface (GUI) feature.

## Save Screen

The save screen is for viewing and recording saved data.

This screen is realized within the GUI file `save.txt`.

The number of save slots per page can be determined in `save.txt`.

## Load Screen

The load screen is for viewing and loading saved data.

This screen is realized within the GUI file `load.txt`.

The number of save slots per page can be determined in `load.txt`.

## History Screen

The history screen displays a history of seen messages.
The maximum number of messages recorded is 100.

End users can click on a line with voice to play the audio.

This screen is realized within the GUI file `history.txt`.

## Config Screen

The config screen allows end users to set volume, text speed,
auto mode speed and font.
It also provides a "back to game title logo" button.
It may also show a preview of text speed and auto mode speed.

This screen is realized within the GUI file `config.txt`.

***

# Graphical User Interface

Graphical user interface (GUI) interacts with the user.
It is described by a text file, and it defines various buttons.

GUI is used to realize the special screen mode.
Typically, it is also used to implement the title as well as other menus.

Note that in a GUI file,
the characters after `#` in a line are ignored and treated as comments.

## GUI File Header

GUI files have a header.
In a header, developers have to define three images;
`idle`, `hover` and `active`.

```
global {
    idle:   config-idle.png;
    hover:  config-hover.png;
    active: config-active.png;
}
```

The `idle` image is displayed as a base layer.
The `hover` image is displayed when the button area is pointed.
The `active` image is displayed when the button is active.
Slidebar buttons are always displayed.

## Jump To Label Button

This button is used for creating a normal menu.

```
QUIT {
    type: goto;
    label: QUIT;
    x: 960;
    y: 497;
    width: 317;
    height: 201;
    pointse: btn-change.ogg;
    clickse: click.ogg;
}
```

## Save/Load Page N Button

This button is used for creating pagenation within the save/load screen.

```
PAGE1 {
    type: savepage;
    index: 0;
    x: 1137;
    y: 0;
    width: 70;
    height: 63;
    pointse: btn-change.ogg;
    clickse: click.ogg;
}
```

## Save Slot Button

This button is used for creating a save slot.

```
SAVESLOT1 {
    type: save;
    index: 0;
    x: 50;
    y: 138;
    width: 1106;
    height: 140;
    margin: 10;
    pointse: btn-change.ogg;
    clickse: click.ogg;
}
```

When game developers create the save slot button,
they need an extra header item within the GUI file.

```
    saveslot: 3;
```

## Load Slot Button

This button is used for creating a load slot.

```
SAVESLOT1 {
    type: load;
    index: 0;
    x: 50;
    y: 138;
    width: 1106;
    height: 140;
    margin: 10;
    pointse: btn-change.ogg;
    clickse: click.ogg;
}
```

When game developers create the load slot button,
they need an extra header item within the GUI file.

```
    saveslot: 3;
```

## Text Speed Slider

This button is used for creating a text speed slidebar.

```
TEXTSPEED {
    type: textspeed;
    x: 68;
    y: 250;
    width: 266;
    height: 21;
    pointse: btn-change.ogg;
}
```

Note that:
* The knob for the slidebar is on the far left of the active image
* The width of the slidebar knob is equal to the height of the button

## Auto Mode Speed Slider

This button is used for creating an auto mode speed slidebar.

```
AUTOSPEED {
    type: autospeed;
    x: 68;
    y: 339;
    width: 266;
    height: 21;
    pointse: btn-change.ogg;    
}
```

Note that:
* The knob for the slidebar is on the far left of the active image
* The width of the slidebar knob is equal to the height of the button

##  Text Speed Preview

This button is used for displaying a preview of text and auto mode speed.

```
PREVIEW {
    type: preview;
    msg: "This message is a preview of text speed and auto mode speed.";
    x: 442;
    y: 453;
    width: 590;
    height: 120;
}
```

## BGM Volume Slider

This button is used for setting the BGM volume.

```
BGM {
    type: bgmvol;
    x: 420;
    y: 249;
    width: 266;
    height: 21;
    pointse: btn-change.ogg;
}
```

Note that:
* The knob for the slidebar is on the far left of the active image
* The width of the slidebar knob is equal to the height of the button

## Sound Effect Volume Slider

This button is used for setting the volume of sound effects.

```
SE {
    type: sevol;
    file: click.ogg;
    x: 420;
    y: 339;
    width: 266;
    height: 21;
    pointse: btn-change.ogg;
}
```

Note that:
* The knob for the slidebar is on the far left of the active image
* The width of the slidebar knob is equal to the height of the button

## Voice (All-character) Volume Slider

This button is used for setting the volume of voice (all-characters).

```
VOICE {
    type: voicevol;
    file: 025.ogg;
    x: 68;
    y: 498;
    width: 266;
    height: 21;
    pointse: btn-change.ogg;
}
```

Note that:
* The knob for the slidebar is on the far left of the active image
* The width of the slidebar knob is equal to the height of the button

## Voice (Per-character) Volume Slider

This button is used for setting the volume of voice (per-character).

See also "Per-character Voice Volume Settings" section.

```
MIDORI {
    type: charactervol;
    index: 1;
    file: 025.ogg;
    x: 506;
    y: 506;
    width: 266;
    height: 21;
    pointse: btn-change.ogg;
}
```

Note that:
* The knob for the slidebar is on the far left of the active image
* The width of the slidebar knob is equal to the height of the button

## Font Selection Button

This button is used for setting the font.

```
FONT1 {
    type: font;
    file: VL-PGothic-Regular.ttf;
    x: 770;
    y: 328;
    width: 87;
    height: 30;
    pointse: btn-change.ogg;
    clickse: click.ogg;
}
```

When there are multiple font buttons,
the active image is displayed for the selected font.

## Full Screen Button

This button is used for setting full screen mode.

```
FULLSCREEN {
    type: fullscreen;
    x: 750;
    y: 206;
    width: 200;
    height: 25;
    pointse: btn-change.ogg;
    clickse: click.ogg;
}
```

When there are full screen and window buttons,
the active image is displayed for the selected one.

## Window Button

This button is used for setting window mode.

```
WINDOW {
    type: window;
    x: 995;
    y: 206;
    width: 200;
    height: 25;
    pointse: btn-change.ogg;
    clickse: click.ogg;
}
```

When there are full screen and window buttons,
the active image is displayed for the selected one.

##  Reset Button

This button is used for resetting options.

```
DEFAULT {
    type: default;
    x: 1131;
    y: 61;
    width: 115;
    height: 40;
    pointse: btn-change.ogg;
    clickse: click.ogg;
}
```

## Page Button

This button is used for moving to another GUI.

```
PAGE2 {
    type: gui;
    file: system-page2.txt;
    x: 1234;
    y: 132;
    width: 35;
    height: 35;
    pointse: btn-change.ogg;
    clickse: click.ogg;
}
```

## Jump To Title Button

This button is for jumping to the title screen.
Before the jump, a Yes/No dialog is displayed.

```
TITLE {
    type: title;
    file: init.txt;
    x: 1007;
    y: 652;
    width: 109;
    height: 25;
    pointse: btn-change.ogg;
    clickse: click.ogg;
}
```

## Cancel Button

This button closes a GUI screen.

```
BACK {
    type: cancel;
    x: 1156;
    y: 653;
    width: 103;
    height: 21;
    pointse: btn-change.ogg;
    clickse: click.ogg;
}
```

## Gallery Button

This button is visible when the specified variable is non-zero.
This feature is for the gallery mode.

```
CG1 {
    type: gallery;
    label: CG1;
    var: $10000;
    x: 1156;
    y: 653;
    width: 103;
    height: 21;
    pointse: btn-change.ogg;
    clickse: click.ogg;
}
```

***

# Configuration Features

Suika2 has a configuration file that allows customization of the application.
The configuration file consists of `key=value` lines.
Some keys are required and some are optional.

Note that configuration lines beginning with `#` are ignored and treated as comments.

## Language Settings

Suika2 only provides game developers with Japanese and English error messages.
However, it can offer multiple-language stories to end users according to
their system locale.

### Language Mapping

This is the ability to display messages in a specific language
when a specific system locale is set.

For example, `language.ja=en` is an indication that the message is
to be displayed in English when the system locale is Japanese.

The default language mappings are as follows (all locales fall back to English).
```
language.en=en
language.fr=en
language.de=en
language.es=en
language.it=en
language.el=en
language.ru=en
language.zh=en
language.tw=en
language.ja=en
language.other=en
```

## Window Settings

Here, a window is the drawable area of the screen.

### Window Title

This is the application title.

On a desktop operating system, a window may have a title.
In such an environment,
this application title becomes the first half of the window title.
The second half of the window title is the chapter name,
but this is omissible by a config.

```
window.title=Suika
```

### Window Width and Height

This is the width and height of the window.

```
window.width=1280
window.height=720
```

### Background Color

Game developers can choose whether the window background should be
white or black.

```
window.white=1
```

### Menu Bar on Windows

On Windows, Suika2 can have a menu bar.
Game developers can decide whether the window has a menu bar or not.

```
window.menubar=1
```

## Font Settings

### Font File Name

This is the font file name.
Font files are stored in the `font` folder.

```
font.file=VL-PGothic-Regular.ttf
```

### Font Size

This is the font size in pixels.
The font size is applied to name, message, history and save item text.

```
font.size=30
```

### Font Colors

It is possible to specify the base color of the text as well as the outline color.

```
font.color.r=255
font.color.g=255
font.color.b=255
font.outline.color.r=128
font.outline.color.g=128
font.outline.color.b=128
```

### Font Outline

Game developers can choose whether or not to display font outlines.

```
font.outline.remove=0
```

## Namebox Settings

### Namebox Image

This is the image file name of the namebox.
The file is stored in the `cg` folder.

```
namebox.file=namebox.png
```

### Namebox Position

This is the position at which to show the namebox.

```
namebox.x=95
namebox.y=480
```

### Namebox Top Margin

This is the top margin of text in the namebox image.

```
namebox.margin.top=11
```

### Namebox Left Margin

This is the left margin of text in the namebox image.

### Namebox Centring

Game developers can choose whether or not to center the character name in the namebox.

```
namebox.centering.no=0
```

### Namebox Visibility

Game developers can choose whether or not to show the namebox at all.

```
namebox.hidden=0
```

## Message Box Settings

### Message Box Images

The message box is made up of a background and foreground image.
The foreground image is used for utilizing an in-box menu such as a hide button.

### Message Box Position

This is the position at which to show the message box image.

```
msgbox.x=43
msgbox.y=503
```

### Message Box Margins

* Left margin of text in the message box image
* Top margin of text in the message box image
* Right margin of text in the message box image
* Line spacing

```
msgbox.margin.left=80
msgbox.margin.top=50
msgbox.margin.right=80
msgbox.margin.line=40
```

### Message Box Text Speed

Game developers can set the number of characters of text to be displayed per second.

```
msgbox.speed=40.0
```

### Position of Hide Button

The message box can have a hide button.
This is optional.

```
msgbox.btn.hide.x=1146
msgbox.btn.hide.y=16
msgbox.btn.hide.width=29
msgbox.btn.hide.height=29
```

### Message Box Sound Effects

The message box has sound effects.

```
msgbox.btn.change.se=btn-change.ogg
msgbox.history.se=click.ogg
msgbox.config.se=click.ogg
msgbox.hide.se=click.ogg
msgbox.show.se=click.ogg
msgbox.auto.cancel.se=click.ogg
msgbox.skip.cancel.se=click.ogg
```

### Skipping Unseen Messages

The game developer can decide whether unseen text can be skipped.
Skipping includes skip mode and control key skip.

```
msgbox.skip.unseen=1
```

### Dimming Seen Messages

Game developers can dim the seen messages.
This is useful for the full-screen novel mode (NVL).

```
msgbox.dim=1
msgbox.dim.color.r=80
msgbox.dim.color.g=80
msgbox.dim.color.b=80
msgbox.dim.color.outline.r=40
msgbox.dim.color.outline.g=40
msgbox.dim.color.outline.b=40
```

## Click Animation Settings

The click-waiting prompt that appears above the message box is a click animation.

### Click Animation Position

This is the position at which to show the click animation image.

```
click.x=1170
click.y=660
```

### Following The Text

The click animation is usually displayed at a fixed position,
but it can also be displayed at the end of the text.
This is useful when making a full screen style novel.

```
click.move=1
```

### Click Animation Images

The first one is required, and others are optional.

```
click.file1=click1.png
click.file2=click2.png
click.file3=click3.png
click.file4=click4.png
click.file5=click5.png
click.file6=click5.png
```

### Click Animation Interval

This is the click animation interval in second.

```
click.interval=1.0
```

### Click Animation Visibility

Developers can hide the click animation.
This is mainly for the visually impaired.

```
click.interval=1.0
```

## Options Settings

### Option Box Images

There are background and foreground images for an option box.
The background image is displayed by default,
but when the option box is pointed by the mouse, the foreground image is displayed.

```
switch.bg.file=switch-bg.png
switch.fg.file=switch-fg.png
```

### First Option Position

This is the position of the first option box.

```
switch.x=406
switch.y=129
```

### Vertical Space Between Options

This is the vertical space between the option box images.

```
switch.margin.y=20
```

### Option Top Margin

This is the top margin of text in the option box image.

```
switch.text.margin.y=18
```

### Pointed Option Color

This is the color of pointed option's text.

```
switch.color.active=0
switch.color.active.body.r=255
switch.color.active.body.g=0
switch.color.active.body.b=0
switch.color.active.outline.r=128
switch.color.active.outline.g=128
switch.color.active.outline.b=128
```

### Options Sound Effects

Options can have sound effects.

```
switch.parent.click.se.file=click.ogg
switch.child.click.se.file=click.ogg
switch.change.se=btn-change.ogg
```

## Save/Load Screen Settings

Most of the settings for save/load screens are defined within their GUI files.
Here, we have just one configuration.

### Save Item Thumbnail Size

This is the thumbnail size of the save data.

```
save.data.thumb.width=213
save.data.thumb.height=120
```

## System Menu Settings

### System Menu Position

This is the position to show the system menu image.

```
sysmenu.x=731
sysmenu.y=29
```

### System Menu Images

The system menu has three images; `idle`, `hover`, `disable`.
`idle` is the base image.
`hover` is displayed when the button area is pointed.
`disable` is displayed when the button is disabled.

```
sysmenu.idle.file=sysmenu-idle.png
sysmenu.hover.file=sysmenu-hover.png
sysmenu.disable.file=sysmenu-disable.png
```

### System Menu Button Positions

The following buttons are available:
* `qsave` (quick save)
* `qload` (quick load)
* `save` (save mode)
* `load` (save mode)
* `auto` (auto mode)
* `skip` (skip mode)
* `history` (history mode)
* `config` (config mode)

The following is the example for `qsave`.

```
sysmenu.qsave.x=62
sysmenu.qsave.y=7
sysmenu.qsave.width=60
sysmenu.qsave.height=58
```

### System Menu Sound Effects

The system menu can have sound effects.

```
sysmenu.enter.se=click.ogg
sysmenu.leave.se=click.ogg
sysmenu.change.se=btn-change.ogg
sysmenu.qsave.se=click.ogg
sysmenu.qload.se=click.ogg
sysmenu.save.se=click.ogg
sysmenu.load.se=click.ogg
sysmenu.auto.se=click.ogg
sysmenu.skip.se=click.ogg
sysmenu.history.se=click.ogg
sysmenu.config.se=click.ogg
```

### Collapsed Position

This is the position at which to show the collapsed system menu image.
The collapsed system menu is usually displayed, and when it is clicked (expanded), the system menu is displayed.

```
sysmenu.collapsed.x=1219
sysmenu.collapsed.y=29
```

### Collapsed Images

THe collapsed system menu has two images; `idle` and `hover`.

```
sysmenu.collapsed.idle.file=sysmenu-collapsed-idle.png
sysmenu.collapsed.hover.file=sysmenu-collapsed-hover.png
```

### Collapsed Sound Effect

The collapsed system menu can have a sound effect.

```
sysmenu.collapsed.se=btn-change.ogg
```

### System Menu Visibility

Game developers can decide whether the collapsed system menu and system menu are visible.

```
sysmenu.hidden=0
```

Note: This will take effect when displaying messages.
When displaying options, the collapsed system menu or system menu are always shown,
this is because of a lack of a way to save without the system menu.

## Auto Mode Settings

Suika2 shows the auto mode banner when auto mode is enabled.

### Auto Mode Banner Image

This is the banner image.

```
automode.banner.file=auto.png
```

### Auto Mode Banner Position

This is the banner position.

```
automode.banner.x=0
automode.banner.y=126
```

### Auto Mode Speed

This is the speed of auto mode.
The wait time for the message is `automode.speed` seconds per character.

```
automode.speed=0.15
```

## Skip Mode Settings

Suika2 shows skip the mode banner when skip mode is enabled.

### Skip Mode Banner Image

This is the banner image.

```
skipmode.banner.file=skip.png
```

### Skip Mode Banner Position

This is the banner position.

```
skipmode.banner.x=0
skipmode.banner.y=186
```

## Initial Sound Volumes

These are the sound volume values for initial boot time.
If a user has the save data,
then the volume settings in the save data will be used
instead of the initial sound volumes.

```
sound.vol.bgm=1.0
sound.vol.voice=1.0
sound.vol.se=1.0
sound.vol.character=1.0
```

## Per-character Voice Volume Settings

If the character name of a line matches to one from the name list,
then the respective per-character volume will be applied.

Per-character volumes can be set in the config screen. (#0 to #15)
See also "Voice (Per-character) Volume Slider" section.

Developers can specify the names for up to 15 characters. (#1 to #15)

If the character name of the message doesn't match a name from the name list,
then the #0 per-character volume will be used.

The following is an example of name list.
```
sound.character.name1=Midori
```

## Character Message Colors

Game developers can specify text colors for up to 64 characters.
Colors are defined for each character by name.

To use character message colors, write the following.
```
serif.color1.name=Haruka
serif.color1.r=255
serif.color1.g=200
serif.color1.b=200
serif.color1.outline.r=0
serif.color1.outline.g=0
serif.color1.outline.b=0
```

## Miscellaneous

These are detailed config items according to individual user requirements.

### Voice Continues On Click

This option enables voice continuation on click.
Normally, voice playback is stopped before moving on to the next command,
but this option suppresses this behaviour.

See alose "Showing a Line" section.

To enable this, write the following line in the config:
```
voice.stop.off=1
```

### Full Screen Mode

This option disables full screen mode.
If game developers don't want to allow full screen mode,
write the following line.

```
window.fullscreen.disable=1
```

### Window Maximization

This option disables window maximization.
If game developers don't want to allow maximization of the window,
write the following line.
```
window.maximize.disable=1
```

### Window Title Separator

The window title consists of the application title and the chapter name.
This separator is added between the application title and the chapter name.

To set the separator, write the following line. Note that there is a space after `=`.
```
window.title.separator= 
```

### Chapter Name

Game developers can decide whether to show chapter names in the window title.

To hide the chapter name from the window title, write the following line.
```
window.title.chapter.disable=1
```

### Show Message Box On Character Change

By default, the message box disappears while characters are changing.
With this option, developers can decide whether to display the message box
while character is changing.

To enable this option, write the following line.
```
msgbox.show.on.ch=1
```

## Release Mode

This mode is used for installing games to the "Program Files" path on Windows.
If this configuration is enabled,
save data will be stored in OS-specific locations such as 'AppData' on Windows
and 'Library' on macOS.

To enable release mode, write the following line.
```
release=1
```

## Note

Initial values for text speed and auto mode speed cannot be set.
The default value for these is `0.5`.

***

# Package

Package is an archive file that contains all game data except video files.
The package file name is `data01.arc`.

## Package Generation

To generate a package file, developers can use the `pack` program or `Suika2 Pro for Creators`.

## Package Obfuscation

The obfuscation key is stored in `key.h`, and developers can change the value for their games.

## Package Usage

To use a package file, put it in the folder that contains a Suika2 application (app folder).
Game data folders (e.g., `bg`, `ch`, `bgm` and etc.) must be excluded from the app folder.
If the game data folder exists in the app folder, Suika2 uses the files in the game data folder instead of the files in the package file.

***

# File Formats

## Text File Format

Script files, GUI files and config file are plain text files.

### Encoding

Text files must be encoded in UTF-8.

### New Line

CR+LF, LF and CR are accepted.
These can be mixed.

## Image File Formats

PNG and JPEG are supported.
Note that gray scaled JPEG images are not supported.

## Audio File Format

The only accepted format is Ogg Vorbis 44.1kHz stereo or monaural.

## Video File Formats

On Windows, `.wmv` is the recommended format.
AVI with H.264 and AAC will work on Windows 10 or later.

On macOS, `.mp4` with H.264 and AAC is the recommended format.
QuickTime (`.mov` or `.qt`) will work too.

On Linux, any format supported by Gstreamer will work.
Game developers must instruct end users to install the neccesary Gstreamer plugin.

In the future, there are plans to move to `.mp4` with H.264 and AAC on all platforms.

***

# Advanced Script

A script stored in the `txt` folder is called `Suika2 Script`,
which is simple script for scenario writers,
consisting of text and commands.

On the other hand, a script stored in the `wms` folder is called `WMS`,
which is script for more advanced programming.
WMS stands for `Watermelon Script`.

A WMS script file has `.scr` extension.

## Syntax and Usage

This section describes the grammar and usage of WMS.

### Defining and Calling Functions

A program starts from `main()` function.
```
func main() {
    // This is a comment.
    print("Hello, world!");
}
```

A function can call other functions.
```
func main() {
    foo(0, 1 2);
}

func foo(a, b, c) {
    return a + b + c;
}
```

Indirect calling by string variable is allowed.
```
func main() {
    myfunc = "foo";
    myfunc(0, 1 2);
}

func foo(a, b, c) {
    return a + b + c;
}

func myfunc(a, b, c) {
    // This function will not be called because foo() will be called in main().
}
```

### Types and Variables

A program can handle integer, floating point, string and array type variable/values.
```
func main() {
    // Integer
    a = 1;
    if(isint(a)) {
        print("a is int");
    }

    // Floating point
    b = 1.0;
    if(isfloat(b)) {
        print("b is float");
    }

    // String
    c = "string";
    if(isstr(c)) {
        print("c is string");
    }

    // Array (integer key)
    d[0] = 0;
    if(isarray(d)) {
        print("d is array");
    }

    // Array (string key)
    e["abc"] = 0;
    if(isarray(e)) {
        print("e is array");
    }

    // Array of array
    f["key"] = e;
}
```

### Loops

To repeat for a specified times:
```
func main() {
    for(i in 0..9) {
        print(i);
    }
}
```

To repeat for each value in an array:
```
func main() {
    a[0] = 0;
    a[1] = 1;
    a[2] = 2;

    for(v in a) {
        print(v);
    }
}
```

To repeat for each key-value pair in an array:
```
func main() {
    a["key1"] = 0;
    a["key2"] = 1;
    a["key3"] = 2;

    for(k, v in a) {
        print(k + "=" + v);
    }
}
```

To create a normal `while` loop:
```
func main() {
    a = 10;
    while (a > 0) {
        print(a);
        a = a - 1;
    }
}
```

To use `break` or `continue` in a `for` or `while` loop:
```
func main() {
    for(i in 0..9) {
        if(i == 2) {
            continue;
        }
        if(i == 7) {
            break;
        }
        print(i);
    }
```

### Branches

A program can branch by `if` - `else if` - `else` syntax.
```
func main() {
    a = foo();
    if(a > 10) {
        print("a > 10");
    } else if(a > 5) {
        print("a > 5");
    } else {
        print("else");
    }
}

func foo() {
    return 6;
}
```

### Arrays

An array element has a key, and the key must be one of integer, floating point and string.
Keys of integer, floating point and string can be mixed.

To create an array, assign a value with `[key]` syntax.
```
func main() {
    a[0] = 0;
    print(a);
}
```

To remove an array element, use `remove()`.
```
func main() {
    a[0] = 0;
    remove(a, 0);
    print(a);
}
```

To get the size of an array, use `size()`.
```
func main() {
    a[0] = 0;
    print(size(a));
}
```

### Conversions

Integer to string:
```
s = "" + 123;
```

Floating point to string:
```
s = "" + 1.23;
```

Integer to floating point:
```
f = 0.0 + 123;
```

String to integer:
```
i = 0 + "123";
```

String to floating point:
```
f = 0.0 + "1.23";
```

## Calling Suika2 Engine

The Suika2 engine provides predefined functions to manipulate the engine.

### Showing an Alert

```
alert("message");
```

***

# Non-functional Requirements

## Performance Requirements

### Native Application

Suika2 binaries are applications that run natively on the target platform.
This is based on the idea of achieving the fastest possible speed of operation.

### CPU Usage

Suika2 can only use CPU time within the range that the OS determines
Suika2's CPU utilization to be "low".

The evaluation is based on the 6th generation Core i7 and its built-in GPU.

### Fast Math

Suika2 needs to speed up vector operations as much as possible.

On Intel/AMD platforms, Suika2 supports SSE, SSE2, SSE3, SSE4.1, SSE4.2, AVX and AVX2.
Suika2 automatically chooses the fastest extension.
Currently, AVX512 is not supported because of the lack of a test machine.

### GPU

On a desktop OS, Suika2 uses GPU acceleration when possible, otherwise
it falls back to 2D rendering for compatibility purposes.

On other environments, Suika2 will always use GPU acceleration because
the CPU may not be powerful enough.

Suika2 uses its own 2D rendering engine if GPU acceleration is not available,
however, this fallback will be removed in the future.

Note that offscreen renderings are not GPU accelerated and done by 2D rendering.

### Frame Rate

The frame rate is set as high as 60 fps,
within the range that the OS determines Suika2's CPU utilization to be "low".

On Windows and Linux, if the GPU acceleration is enabled,
the target frame rate is 60fps, and if not, 30fps.

On macOS, the target frame rate is 30fps.

On iOS and Android, the frame rate is determined by OpenGL View of the OS.
