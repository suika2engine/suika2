Suika2 Tutorial
================

Welcome to the world of Suika2!

This document explains how to make a basic visual novel using the engine, don't worry, it's really easy!

Okay, Ready? Let's get started!

## Preparation

We'll be using Windows for this tutorial, but the process is similar on MacOS.
For usage on other platforms, see [usage](https://github.com/suika2engine/suika2#usage).

Firstly, [download](https://suika2.com/en/) the Suika2 binary.

Extract the downloaded ZIP file. Within the extracted ZIP file, you'll see the following:

* `suika2/`
    * `bg/` A folder to store background image files.
    * `bgm/` A folder to store background music files.
    * `cg/` A folder to store system image files such as for message boxes.
    * `ch/` A folder to store character image files.
    * `conf/` A folder to store setting file.
    * `cv/` A folder to store voice files.
    * `font/` A folder to store font files.
    * `gui/` A folder to store GUI files. For me information on GUI files, see the [documentation](https://suika2.com/en/reference.html#gui).
    * `rule/` A folder to store rule files.
    * `se/` A folder to store sound effect files.
    * `txt/` A folder to store script files.
    * `suika.exe` Executable application file for Windows.
    * `suika-pro.exe` Executable application file for Windows. (Suika2 Visual Debugger)

`suika.exe` is the Sukia2 executable file. The other folders contain assets and information needed for the included demo game.
When you run `suika.exe`, a `sav/` folder will be automatically created, this will contain save-data.

At the moment, we're just going to use demo-game chunks to play around with Suika2's various features.
This tutorial is by no means exhaustive, so please refer to the [documentation](https://suika2.com/en/reference.html) for more commands and updated information. This document will be updated in the future to better align with Suika2's current version and documentation.

## Enable International Mode

* Open `conf/config.txt` with a text editor.
* Locate the following line near the top of the document:

```
i18n=1
```

* If the value is set to disabled (`0`), make sure to enable it by replacing the `0` with `1`.

## Open a Script File

Within the `txt` folder we can find `init.txt`.
When Suika2 starts, this will be the first script file that runs.
The great thing about script files is that they're regular `.txt` files,
so you don't need any special software to open them!

Once you've removed the contents of `init.txt`, you're ready to start!

## Comments

Before we get into things, it's important to note that Suika2 scripts support comments. 
You can use these to make notes and 'sections' of the document.
Comments won't appear anywhere in your game, and will only be viewable in the file itself.
To use a comment, insert a `#` at the beginning of the line. Suika2 comments don't need a closing tag.

```
# My first Suika2 script!
```

## Show a Background Image

For our first real step, we'll change the background image.
This is the most basic command.

Insert the following into `init.txt`:

```
@bg roof.png 1.0
@click
```

`@bg` is the command to show a background image.
`roof.png` is a file inside the `bg` folder.
`1.0` is the fade-in time counted in seconds.
`@click` is the command to wait for the user to click.

After saving it, run `suika.exe`.
Suika2 will show a roof terrace, wait for a click and then exit.

## Show a Character Image

Showing character images is pretty similar to the `@bg` command.

Insert the following into `init.txt`, between the `@bg` and `@click` commands:

```
@ch center 001-fun.png 1.0
```

`@ch` is the command to show a character.
`center` is the horizontal position to show a character.
`001-fun.png` is a file inside the `ch` folder.
`1.0` is the fade-in time counted in seconds.

After saving it, run `suika.exe`.
Suika2 will show a roof terrace, a character, wait for a click and then exit.

## Show a Message

Showing dialogue is fundamental to visual novels.
Messages are printed as text inside message boxes.
Insert the following into `init.txt`, benearth the `@ch` command:

```
Hi, my name is Midori.
```

After saving it, run `suika.exe`.
Suika2 will show a roof terrace, a character, one message and then exit.

Any script lines which don't start with `@`, `:` or `\*` are recognized as messages.

## Show a Message with a Character's Name

In most cases, it's important to know whose speaking,
this is achieved by enclosing the character's name in `*`.

Insert the following into `init.txt`:

```
*Midori*Now that you know my name, you can use it in your init file!
```

## Show a Message with Voice

Sometimes, you will have voice files that match the dialogue of your story,
luckily, this is simple to implement with Suika2 scripts!

Using voice follows a similar structure to names, however, an extra `*`
is inserted between the name and voice file.

Insert the following into `init.txt`:

```
*Midori*025.ogg*Messages can even be shown with voice lines!
```

`025.ogg` is a file inside the `cv` folder.
Note: The english distrobution only contains the `cv/025.ogg` and `se/suika.ogg` voice lines.
`025.ogg` doesn't reflect the written message above, and is just an example.

## Playing Background Music

You can play background music (BGM) using the `@bgm` command.
Replace the contents of `init.txt` with the following:

```
@bgm 01.ogg
Playing BGM.
```

`@bgm` is the command to play BGM.
`01.ogg` is a file inside the `bgm` folder.

After saving it, run `suika.exe`.
Suika2 will now play the selected music file and display a message.

Note: Suika2 can only play sound files encoded using Ogg Vorbis 44.1kHz stereo or monaural format.

## Showing Options

A game may have options for multiple endings.
Replace the contents of `init.txt` with the following:

```
Ok, I'm gonna go to school.
@choose label1 "By foot." label2 "On my bicycle." label3 "I decided not to."
:label1
I'm gonna go to school by foot.
@goto end
:label2
I'm gonna go to school on my bicycle.
@goto end
:label3
I'm home sick from school.
:end
```

After saving it, run `suika.exe`.
Suika2 will show three options.
`@choose` is the command to show options. (You can create up to eight options with this command.)
`label1`, `label2` and `label3` are jump destinations.
Option messages follow labels.

`:label1` is a label to specify a jump destination.
Label lines are ignored when they are executed.

## Set a Flag and Branch

Next, we set a flag and create a branch.
Replace the contents of `init.txt` as follows:

```
@bg roof.png 1.0
@ch center 001-fun.png 1.0
Ok, I'm gonna go to school.
@choose label1 "By foot." label2 "By bicycle." label3 "I decided not to."
:label1
I'm gonna go to school by foot.
@set $1 = 1
@goto next
:label2
I'm gonna go to school by bicycle.
@goto next
:label3
I'm home sick from school.
:next
@if $1 == 0 end
I found money on the ground.
:end
```

After saving it, run `suika.exe`.
Suika2 will show three options.
If you choose "By foot.", the message "I found money on the ground." is shown.

Here, we use a combination of `@set` and `@if`.

`@set` is the command to set variables.
`$1` on the LHS means we store to variable at number 1.
`=` means simple assignment.
Lastly, `1` means integer 1.

All variables are initially zero.
`$1` is set to `1` only when "By foot." is selected.

`@if` is the command to jump by condition.
`$1 == 0` means `when variable at number 1 is 0`.
`end` is the jump destination label.

## Show a Menu

You can make a menu screen using Suika2 scripts.
Replace the contents of `init.txt` as follows:

```
@menu menu.png menu_selected.png START 2 495 314 192 LOAD 316 494 322 203 CONFIG 640 492 317 204 QUIT 960 497 317 201
:START
:LOAD
:CONFIG
:QUIT
```

After saving it, run `suika.exe`.
Suika2 will show the title screen of the sample game.

Let's look at this in more detail:

`@menu` is the command to show the menu screen. It uses two images.
`@menu menu.png menu_selected.png` means we use `menu.png` and `menu_selected.png` in the `bg` folder.
Compare these images.
`menu.png` is an image for when a button isn't being pointed at by the mouse.
`menu_selected.png` is an image for when the button is being pointed at by the mouse.

`START 2 495 314 192` creates a button.
`START` is the jump destination label.
`2 495 314 192` means creating a button with 314x192 size at position (2, 495).

## Split Script Files

When script files start becoming to combresome to organise and undestand efficeintly,
it may be a good idea to split the file.
Thankfully, Suika2 supports this functionality by default.

Replace the contents of `init.txt` as follows:

* init.txt
```
@bg roof.png 1.0
@ch center 001-fun.png 1.0
inside init.txt
@load second.txt
```

Create `second.txt` in the `txt` folder as follows.

* second.txt
```
moved to second.txt
```

After saving these files, run `suika.exe`.
Suika2 will show "moved to second.txt" after "inside init.txt".

`@load` is the command to jump to a specified script file.

## Creating a Package

We can create a single package file that contains scripts, images, sounds and so
on with Suika2 sister application `suika-pro.exe`.

To do so, follow these steps:

1. Double click `suika-pro.exe`. `Suika2 Pro for Creators` will open.
2. Click `File` in the menu, then click `Export package`. It will create a file, `data01.arc`.
3. We should only distribute `suika.exe` and `data01.arc`.

When we have both the package and a normal file, Suika2 uses the normal file.

## Support

Please don't hesitate to reach out via Discord or E-Mail if you need any support,
we'd love to hear from you!
