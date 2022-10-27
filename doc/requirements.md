***

# Software Requirements Specification for Suika2

***

# Introduction

Suika2 is a tool for creating visual novels.
It is a cross-platform software, running on at least Windows, Mac, Linux, iOS, Android and Web.

This document is for engine developers, not for game developers.
It describes software requirements specification for Suika2, from aspect of software engineering.

***

# Screen Components

Screen components are layered.
They consist of the following, in order from top to bottom

1. System Menu / Collapsed System Menu
2. Options
3. Character Face
4. Click Animation
5. Name Box
6. Message Box
7. Center Character
8. Right Character
9. Left Character
10. Back Center Character
11. Background

## Background

The background image is the lowest layer that makes up the screen.
It must have the same size as window size.

## Characters

There are four types of regular characters, depending on their display position.
The `center` and `back` have the same display position, but the layers displayed are different.

1. center
2. right
3. left
4. back

In addition to these, there is a position for the character's `face` to be displayed to the left of the message box.

## Message Box

The message box is a window for outputting text.

### In-box Menu

Message box can have menu buttons.
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

## Namebox

The namebox is a window for outputting character's name.
This can be hidden by configuration.

## System Menu

The system menu is used to save, load, view logs, and display the config screen during game play.

## Collapsed System Menu

Clicking this button brings up the system menu.
Normally, collapsed system menu is displayed, but when clicked, system menu is displayed.

***

# Special Screens

Apart from the normal layered game screen, there are special screens.
These special screens are realized by the graphical user interface (GUI) feature.

## Save Screen

Save screen is for viewing and recording saved data.

## Load Screen

Load screen is for viewing and loading saved data.

## History Screen

History screen displays the history of messages.

## Config Screen

Config screen allows end users to set volume, text speed, auto mode speed and font.
It may also show the preview of text speed and auto mode speed.

***

# Command Features

The building blocks of Suika2 functionality are commands.
Therefore, more than half of the functional requirements are in the design of the commands.

## Showing a Background Image

This feature shows a background image.

The command that accomplishes this feature is `@bg`.

## Showing Character Images

This feature shows a character image.
Character layers include center front, right, left, center back and face.

The command that accomplishes this feature is `@ch`.

## Showing Message

This feature shows the descriptive part in the story.

It is not a command that makes this feature possible, but the line of text that does not begin with an atmark.

## Showing a Line

This feature shows the dialog in the story.
A line consists of a name, a message and optionally a voice file name.

It is not a command that makes this feature possible, but the line of text that begin with an asterisk.

## Playing Background Music

This feature plays background music.

The command that accomplishes this feature is `@bgm`.

## Playing Sound Effect

This feature plays sound effect.

The command that accomplishes this feature is `@se`.

## Fading a Volume

This feature changes the volume.
The volume will fade for the specified time.

Local volumes are stored for each saved data. Global volumes are common to all saved data.

The command that accomplishes this feature is `@se`.

## Showing Options

This feature shows options and lets user choose one option, then jumps to the label selected.
This feature can display up to eight options.

The command that accomplishes this feature is `@choose`.

## Label

This is a jump target inside a script.

It is not a command, but a line of text that begin with a colon.

## Showing an Animation

This feature displays a character sprite in motion.

The command that accomplishes this feature is `@cha`.

## Changing Characters and Background at Once

This feature changes character sprites and background image at once.
It is also possible to change only the characters or only the background.

The command that accomplishes this feature is `@chs`.

## Shaking the Screen

This feature shakes the screen.

The command that accomplishes this feature is `@shake`.

## Playing a Video

This feature plays a video file.
Available video formats vary from platform to platform, but will be standardized to mp4 in the future.

The command that accomplishes this feature is `@video`.

## Showing a GUI

This feature shows a graphical user interface (GUI).
GUI is a menu consisting of three images.

The command that accomplishes this feature is `@gui`.

## Waiting for a Click

This feature waits for a user click.

The command that accomplishes this feature is `@click`.

## Waiting for Specified Time

This feature waits for specified time in second.

The command that accomplishes this feature is `@wait`.

## Prohibiting Skip

This feature enables/disables skip by control key.

The command that accomplishes this feature is `@skip`.

## Jumping to a Label

This feature moves the script execution position to the specified label.

The command that accomplishes this feature is `@jump`.

## Setting a Variable

This feature sets variable value.
It can perform calculations as well as simple assignments.

The command that accomplishes this feature is `@set`.

## Branching by Variable

This feature moves the script execution position to the specified label if the specified condition of variable meets.

The command that accomplishes this feature is `@if`.

## Jumping to a Script

This feature loads a script file.

The command that accomplishes this feature is `@load`.

## Setting a Chapter Name

This feature sets a chapter name.
The name of the chapter is reflected in the window title and saved data items.

The command that accomplishes this feature is `@chapter`.

## Multilingualization

This feature automatically switches the language to be displayed depending on the user's execution environment.

To use this feature, a line must be written that begins with a plus sign.

```
+en+Hello.
+fr+Bonjour.
```

This is not exactly a command, but it is a prefix to a command.

***

# Graphical User Interface

Graphical user interface (GUI) is a screen mode which interact with user.
It is defined by a text file, and it defines various buttons.

## GUI File Header

GUI file has a header.
In a header, developers have to define three images; `idle`, `hover` and `active`.

```
global {
    idle:   config-idle.png;
    hover:  config-hover.png;
    active: config-active.png;
}
```

`idle` image is displayed as a base layer.
`hover` image is displayed when the button area is pointed.
`active` image is displayed when the button is active, or, for slidebar buttons always displayed for a slidebar.

## Jump To Label Button

This button is used for creating normal menu.

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

This button is used for pagenation of save/load screen.

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

This button is used for creating save slot.

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

## Load Slot Button

This button is used for creating load slot.

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

## Text Speed Slider

This button is used for creating text speed slide bar.

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

## Auto Mode Speed

This button is used for creating auto mode speed slide bar.

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

##  Text Speed Preview

This button is used for displaying preview of text and auto mode speed.

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

This button is used for setting BGM volume.

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

## Sound Effect Volume Slider

This button is used for setting sound effect volume.

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

## Voice (All-character) Volume Slider

This button is used for setting voice (all-character) volume.

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

## Font Selection Button

This button is used for setting font.

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

This button is used for moving to other GUI.

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

## Jump To File Button

This button is for jumping to a script.

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

This button closes GUI screen.

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

***

# Configuration Features

Suika2 has a configuration file that allows customization of the application.
Configuration file consists of `key=value` lines.
Some keys are required and some are optional.

## Language Settings

Suika2 provides game developers only with Japanese and English error messages.
However, it provides multiple-language stories to end users.

### International Mode

If international mode is enabled, error messages will be in English. Otherwise, they will be in Japanese.
Turning on international mode also enables language mappings.

Adding `i18n=1` enables international mode.

### Language Mapping

This is the ability to display messages in a specific language when a specific system locale is set.

For example, `language.ja=en` is an indication that the message is to be displayed in English when the system locale is Japanese.

The default language mappings are as follows
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

In a desktop operating system, a window may have a title.
In such an environment, this application title becomes the first half of the window title.
The second half of the window title is the chapter name, but this is omissible by a config.

```
window.title=Suika
```

### Window Width and Height

This is the window width and height.

```
window.width=1280
window.height=720
```

### Background Color

Game developers can choose whether the window background should be white or black.

```
window.white=1
```

## Font Settings

### Font File Name

This is the font file name.
Font files are stored in `font` folder.

```
font.file=VL-PGothic-Regular.ttf
```

### Font Size

This is the font size in pixels.
Font size is applied to message text and save item text.

```
font.size=30
```

### Font Colors

It is possible to specify the base color of the text and the outline color.

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

### Image

This is the image file name of namebox.
The file is stored in `cg` folder.

```
namebox.file=namebox.png
```

### Position

This is the position to show the namebox.

```
namebox.x=95
namebox.y=480
```

### Top Margin

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

Game developers can choose whether or not to show namebox.

```
namebox.hidden=0
```

## Message Box Settings

### Message Box Images

There are background and foreground images.
Foreground image is used for in-box menu such as hide button.

### Message Box Position

This is a position to show the message box image.

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

Message box can have hide button.
This is optional.

```
msgbox.btn.hide.x=1146
msgbox.btn.hide.y=16
msgbox.btn.hide.width=29
msgbox.btn.hide.height=29
```

### Sound Effects

Message box has sound effects.

```
msgbox.btn.change.se=btn-change.ogg
msgbox.history.se=click.ogg
msgbox.config.se=click.ogg
msgbox.hide.se=click.ogg
msgbox.show.se=click.ogg
msgbox.auto.cancel.se=click.ogg
msgbox.skip.cancel.se=click.ogg
```

### Skipping Unseen Message

The game developer can decide whether unread text can be skipped.
Skipping includes skip mode and control key skip.

## Click Animation Settings

The click-waiting prompt that appears above the message box is a click animation.

### Position

This is a position to show the click animation image.

```
click.x=1170
click.y=660
```

### Following The Text

Click animation is usually displayed at a fixed position, but it can also be displayed at the end of the text.

```
click.move=0
```

### Images

First one is required, and others are optional.

```
click.file1=click1.png
click.file2=click2.png
click.file3=click3.png
click.file4=click4.png
click.file5=click5.png
click.file6=click5.png
```

### Interval

This is the click animation interval in second.

```
click.interval=1.0
```

### Visibility

Developers can hide click animation.
This is mainly for the visually impaired.

```
click.interval=1.0
```

## Options Settings

### Images

There are background and foreground images for an option box.
Background image is displayed usually, but when the option box is pointed by mouse, foreground image is displayed.

```
switch.bg.file=switch-bg.png
switch.fg.file=switch-fg.png
```

### Position

This is the position of the first option box.

```
switch.x=406
switch.y=129
```

### Vertical Space

This is the vertical space between the option box images.

```
switch.margin.y=20
```

### Top Margin

This is the top margin of text in the option box image.

```
switch.text.margin.y=18
```

### Color

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

### Sound Effects

Options can have sound effects.

```
switch.parent.click.se.file=click.ogg
switch.child.click.se.file=click.ogg
switch.change.se=btn-change.ogg
```

## Save/Load Screen Settings

Most of settings for save/load screens are defined in GUI file.
Here, we have just one configuration.

### Thumbnail Size

This is the thumbnail size of the save data.

```
save.data.thumb.width=213
save.data.thumb.height=120
```

## System Menu Settings

### Position

This is the position to show the system menu image.

```
sysmenu.x=731
sysmenu.y=29
```

### Images

System menu has three images; `idle`, `hover`, `disable`.
`idle` is the base image.
`hover` is displayed when the button area is pointed.
`disable` is displayed when the button is disabled.

```
sysmenu.idle.file=sysmenu-idle.png
sysmenu.hover.file=sysmenu-hover.png
sysmenu.disable.file=sysmenu-disable.png
```

### Button Positions

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

### Sound Effects

System menu can have sound effects.

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

This is the position to show the collapsed system menu image.
Collapsed system menu is usually displayed, and when it is clicked, (expanded) system menu is displayed.

```
sysmenu.collapsed.x=1219
sysmenu.collapsed.y=29
```

### Collapsed Images

Collapsed system menu has two images; `idle` and `hover`.

```
sysmenu.collapsed.idle.file=sysmenu-collapsed-idle.png
sysmenu.collapsed.hover.file=sysmenu-collapsed-hover.png
```

### Collapsed Sound Effect

Collapsed system menu can hava sound effect.

```
sysmenu.collapsed.se=btn-change.ogg
```

### Visibility

Game developers can decide whether collapsed system menu and system menu are visible.

```
sysmenu.hidden=0
```

Note: This will take effect when displaying messages.
When displaying options, collapsed system menu and system menu is always shown.

## Auto Mode Settings

Suika2 shows auto mode banner when auto mode enabled.

### Image

This is the banner image.

```
automode.banner.file=auto.png
```

### Position

This is the banner position.

```
automode.banner.x=0
automode.banner.y=126
```

### Speed

This is a speed of auto mode.
Wait time is `automode.speed` seconds per character.

```
automode.speed=0.15
```

## Skip Mode Settings

Suika2 shows skip mode banner when skip mode enabled.

### Image

This is the banner image.

```
skipmode.banner.file=skip.png
```

### Position

This is the banner position.

```
skipmode.banner.x=0
skipmode.banner.y=186
```

## Initial Sound Volumes

These are the initial boot time values for sound volumes.
If a user has the save data, then volume settings in the save data will be used.

```
sound.vol.bgm=1.0
sound.vol.voice=1.0
sound.vol.se=1.0
sound.vol.character=1.0

```

## Per-character Voice Volume Settings

If the character name of the message matches to the name list,
then the per-character volume will be applied.

Per-character volumes can be set in the config screen. (#0 to #15)

Developers can specify the names for 15 characters. (#1 to #15)

If the character name of the message doesn't match the names list,
then the #0 per-character volume will be used.

The following is the example of name list.
```
sound.character.name1=Midori
```

## Character Message Colors

Developers can specify text colors for up to 64 characters.

```
serif.color1.name=Haruka
serif.color1.r=255
serif.color1.g=200
serif.color1.b=200
serif.color1.outline.r=0
serif.color1.outline.g=0
serif.color1.outline.b=0
```

## UI Messages

User interface messages can be localized, but are not multilingualized at this moment.

```
ui.msg.quit=Are you sure you want to quit?
ui.msg.title=Are you sure you want to go to title?
ui.msg.delete=Are you sure you want to delete the save data?
ui.msg.overwrite=Are you sure you want to overwrite the save data?
ui.msg.default=Are you sure you want to reset the settings?
```

## Miscellaneous

There are detailed configuration items according to individual user requirements.

### Voice Continues On Click

This option enables voice continuation on click.

```
voice.stop.off=0
```

### Full Screen Mode

This option disables full screen mode.

```
window.fullscreen.disable=0
```

### Window Maximization

This option disables window maximization.

```
window.maximize.disable=0
```

### Window Title Separator

Window title consists of application title and chapter name.
This separator is added between application title and chapter name.

```
window.title.separator=-
```

### Chapter Name

Game developer can decide whether to show chapter name in window titke

```
window.title.chapter.disable=0
```

### Show Message Box On Character Change

By default, message box disappears while characters are changing.
With this option, developers can decide whether to display message box on character change.

```
msgbox.show.on.ch=1
```

## Release Mode

This mode is used for installing games to the "Program Files" path on Windows.
If this configuration is enabled, save data will be stored in OS-specific locations such as 'AppData' on Windows and 'Library' on macOS.

```
release=1
```

***

# Package

## Generation

To generate archive file, developer can use `pack` program or `Suika2 Pro for Creators`.

## Obfuscation

Obfuscation key is stored in `key.h`, and developer can change the value for their games.

***

# Non-functional Requirements

## Native Application

Suika2 binaries are applications that run natively on the target platform.

## Performance Requirements

### Vector Extension

Suika2 uses CPU's vector extension for offscreen renderng.
If the CPU supports multiple vector extensions, Suika2 chooses fastest extension.
On Intel platforms, Suika2 support SSE, SSE2, SSE3, SSE4.1, SSE4.2, AVX and AVX2.
Currently, AVX512 is not supported because of lack of test machine.

### GPU

Suika2 uses GPU acceleration when possible.
If not, fall back to 2D rendering.

### 2D Rendering

Suika2 uses its own 2D rendering engine if GPU acceleration is not available.
