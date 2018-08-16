Suika 2 Tutorial
================

Welcome to the world of Suika 2!

This document explains how to make a visual novel using Suika 2.
The tutorial is very easy.

Are you ready? Let's get started!

## Preparation

At the beginning, [download](http://luxion.jp/s2/dl.html) the binary of Suika 2.
We use the Windows version here.

Extract the downloaded ZIP file. We see the following file and folders.
`suika.exe` is the executable file of Suika 2. Other folders include sample
game.

* `suika2`
    * `bg/` A folder to store background image files.
    * `bgm/` A folder to store background music files.
    * `cg/` A folder to store system image files such as message box.
    * `ch/` A folder to store character image files.
    * `conf/` A folder to store setting file.
    * `cv/` A folder to store voice files.
    * `font/` A folder to store font file.
    * `se/` A folder to store sound effect files.
    * `tool/` A folder that includes file packager.
    * `txt/` A folder to store script files.
    * `suika.exe` Application executable file for Windows.

This tutorial makes changes to the sample game.

## Open script file

We can find a file named `init.txt` in the `txt` folder.
This file is the first script file to be loaded after Suika 2 starts.
If you open this file by a text editor,
you can understand that the script file is just a plain text.

## Show background image

As a first step, we change background image.
This is the most basic command.

Replace the content of `init.txt` as follows.
Note that the character encoding is UTF-8 without BOM.

```
@bg roof.png 1.0
@click
```

After save it, run `suika.exe`.
Suika 2 shows a roof terrace, then waits for click, and exits.

`@bg` is the command to show background image.
`roof.png` is a file inside `bg` folder.
`1.0` is fade-in time counted by seconds.

## Show character image

Showing character image is most of the same as `@bg` command.
Replace the content of `init.txt` as follows.

```
@bg roof.png 1.0
@ch center 001-fun.png 1.0
@click
```

After save it, run `suika.exe`.
Suika 2 shows a roof terrace and a character,
then waits for click, and exits.

`@ch` is the command to show a character.
`center` is the horizontal position to show a character.
`001-fun.png` is a file inside `ch` folder.
`1.0` is fade-in time counted by seconds.

## Show message

Showing dialogue is fundamental to visual novels.
You can print text in the message box.
Replace the content of `init.txt` as follows.

```
@bg roof.png 1.0
@ch center 001-fun.png 1.0
Hi, my name is Midori.
*Midori*Message can be shown with character name.
*Midori*011.ogg*Message can be shown with voice.
```

After save it, run `suika.exe`.
Suika 2 shows a roof terrace and a character,
then shows three messages,
and exits.

Any script lines which doesn't start with @ or * is recognized as message.

## Play BGM

You can play BGM by `@bgm` command.
Replace the content of `init.txt` as follows.

```
@bgm 01.ogg
@bg roof.png 1.0
@ch center 001-fun.png 1.0
Playing BGM.
```

After save it, run `suika.exe`.
Suika 2 plays music.
`@bgm` is the command to play BGM.
`01.ogg` is a file inside `bgm` folder.

Suika 2 can only play Ogg Vorbis 44.1kHz stereo or monaural format.

## Show options

Multiple ending game may have options.
Replace the content of `init.txt` as follows.

```
@bg roof.png 1.0
@ch center 001-fun.png 1.0
Ok, I'm gonna go to school.
@select label1 label2 label3 "By foot." "By bicycle." "I decided not to."
:label1
I'm gonna go to school by foot.
@goto end
:label2
I'm gonna go to school by bicycle.
@goto end
:label3
I'm home sick from school.
:end
```

After save it, run `suika.exe`.
Suika 2 shows three options.
`@select` is the command to show options.
`label1`, `label2` and `label3` are jump destination.
Option messages follow labels.

`:label1` is a label to be specified as jump destination.
Label lines are ignored when they are executed.

## Set flag and branch

Next, we set flag and branch.
Replace the content of `init.txt` as follows.

```
@bg roof.png 1.0
@ch center 001-fun.png 1.0
Ok, I'm gonna go to school.
@select label1 label2 label3 "By foot." "By bicycle." "I decided not to."
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

After save it, run `suika.exe`.
Suika 2 shows three options.
If you choose "By foot.", the message "I found money on the ground." is shown.

Here, we use combination of `@set` and `@if`.

`@set` is the command to set variable.
`$1` on the LHS means we store to variable at number 1.
`=` means simple assignment.
Last `1` means integer 1.

All variables are initially zero.
`$1` is set to `1` only when "By foot." is selected.

`@if` is the command to jump by condition.
`$1 == 0` means `when variable at number 1 is 0`.
`end` is jump destination label.

## Show menu

You can make menu screen.
Replace the content of `init.txt` as follows.

```
:MENU
@menu menu.png menu_selected.png START 27 485 317 193 LOAD 357 493 318 213 QUIT 678 492 315 191
:START
Start.
@goto END
:LOAD
Load.
@goto $LOAD
@goto MENU
:QUIT
Quit.
:END
```

After save it, run `suika.exe`.
Suika 2 shows the title screen of the sample game.

Let's look in more detail.

`@menu` is the command to show menu screen. It uses two images.
`@menu menu.png menu_selected.png` means we use `menu.png` and `menu_selected.png` in `bg` folder.
Compare these images.
`menu.png` is a image when the button is not pointed by mouse.
`menu_selected.png` is a image when the button is pointed by mouse.

`START 27 485 317 193` creates button.
`START` is the jump destination label.
`27 485 317 193` means creating a button with 317x193 size at position (27, 485).
We can make up to four buttons.

## Split script file

It is difficult for author to maintain single large script file.
We have fashion to split script files.

Replace the content of `init.txt` as follows.

* init.txt
```
@bg roof.png 1.0
@ch center 001-fun.png 1.0
inside init.txt
@load second.txt
```

Create `second.txt` in `txt` folder as follows.

* second.txt
```
moved to second.txt
```

After save them, run `suika.exe`.
Suika 2 shows "moved to second.txt" after "inside init.txt".

`@load` is the command to jump to specified script file.

## Create package

We can create a single package file to contain scripts, images, sounds and so on.
Follow these steps.

1. Move `package-win.exe` in `tools` folder into the folder where `suika.exe` exists.
2. Double click `package-win.exe`. It creates `data01.arc`.
3. We should only distribute `suika.exe` and `data01.arc`.

When we have package and normal file, Suika 2 use normal file.
