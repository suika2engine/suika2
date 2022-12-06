Welcome to the world of Suika2!

This document explains how to create a basic visual novel using the engine, don't worry, it's really easy!

Okay, Ready? Let's get started!

# Preparation
We'll be using Windows for this tutorial, but the process is similar on MacOS. For instructions on installing and setting up Suika2 on other platforms, please see [building from source](https://github.com/suika2engine/suika2/wiki/1.-Installation-and-Building-from-Source#building-from-source).

Firstly, download the Suika2 binary from the [releases page](https://github.com/suika2engine/suika2/releases/tag/v2.11.9).

Once you've extracted the contents of the ZIP file, you should be presented with a directory containing the following:

* `suika2/`
	* `bg/` A folder to store background image files.
	* `bgm/` A folder to store background music files.
	* `cg/` A folder to store system image files such as for message boxes.
	* `ch/` A folder to store character image files.
	* `conf/` A folder to store setting file.
	* `cv/` A folder to store voice files.
	* `font/` A folder to store font files.
	* `gui/` A folder to store GUI files. For me information on GUI files, see the documentation.
	* `rule/` A folder to store rule files.
	* `se/` A folder to store sound effect files.
	* `txt/` A folder to store script files.
	* `mac.dmg` Executable application file for MacOS.
	* `mac-pro.dmg` Executable application file for MacOS. (Suika2 Visual Debugger)
	* `suika.exe` Executable application file for Windows.
	* `suika-pro.exe` Executable application file for Windows. (Suika2 Visual Debugger)

You can safely remove `mac.dmg` and `mac-pro.dmg` (or the Windows equivilent if on MacOS).

`suika.exe` is the Suika2 executable file. The other folders contain assets and information needed for the included demo game. When you run `suika.exe`, a `sav/` folder will be automatically created, this will contain save-data.

At the moment, we're just going to use demo-game chunks to play around with Suika2's various features. This tutorial is by no means exhaustive, so please refer to the [reference](https://github.com/suika2engine/suika2/wiki/3.-Suika2Script-and-Commands) for more commands and updated information. This document will be updated in the future to better align with Suika2's current version and documentation.

#Open a Script File
Within the `txt/` folder we can find `init.txt`. When Suika2 starts, this will be the first script file that runs. The great thing about script files is that they're regular `.txt` files, so you don't need any special software to open them!

**Note:** Whenever you see `Insert the following into init.txt:`, make sure you clear your script (highlight and delete the text) this will let you watch each example without interruption or distraction. You can, of course, mix these examples and your own work to see what you can create!

# Comments
Before we get into things, it's important to note that Suika2 scripts support comments. You can use these to make notes and 'sections' of the document. Comments won't appear anywhere in your game and will only be viewable from within the file itself. To use a comment, insert a `#` at the beginning of the line. Suika2 comments don't need a closing tag.

```
# My first Suika2 script!
```

# Show a Background Image
For our first real step, we'll change the background image. This is one of the most basic commands.

Insert the following into `init.txt`:

```
@bg roof.png 1.0
@click
```

[`@bg`](https://github.com/suika2engine/suika2/wiki/3.-Suika2Script-and-Commands#showing-a-background-bg) is the command to show a background image, `roof.png` is a file inside the `bg/` folder, `1.0` is the fade-in time counted in `seconds`, and [`@click`](https://github.com/suika2engine/suika2/wiki/3.-Suika2Script-and-Commands#waiting-for-a-click-click) is the command to wait for the user to click.

After saving it, run `suika.exe`. Suika2 will show a roof terrace, wait for a click, and then exit.

# Show a Character Image
Showing character images is pretty similar to the `@bg` command.

Insert the following into `init.txt`, between the @bg and @click commands:

```
@ch center 001-fun.png 1.0
```

[`@ch`](https://github.com/suika2engine/suika2/wiki/3.-Suika2Script-and-Commands#showing-a-character-ch) is the command to show a character, `center` is the horizontal position to show a character, `001-fun.png` is a file inside the `ch/` folder, and `1.0` is the fade-in time counted in `seconds`.

After saving it, run `suika.exe`. Suika2 will show a roof terrace, a character, wait for a click, and then exit.

# Show a Message
Showing dialogue is fundamental to visual novels. Messages are printed as text inside message boxes. Insert the following into `init.txt`:

```
Hi, my name is Midori!
```

After saving it, run `suika.exe`. Suika2 will display one message and then exit on click.

Any script lines which don't start with `@`, `:`, or `\*` are recognized as messages.

# Show a Message with a Character's Name
In most cases, it's important to know whose speaking, this is achieved by enclosing the character's name in a pair of `*`.

Insert the following into `init.txt`:

```
*Midori*Now that you know my name, you can use it in your scripts!
```

# Show a Message with Voice
Sometimes, you will have voice files that match the dialogue of your story, luckily, this is simple to implement in your Suika2 scripts!

Using voice follows a similar structure to names, however, an extra `*` is inserted between the name and voice file.

Insert the following into `init.txt`:

```
*Midori*025.ogg*Messages can even be shown with voice lines!
```

`025.ogg` is a file inside the `cv/` folder.

**Note:** The English distribution only contains the `cv/025.ogg` and `se/suika.ogg` voice lines. `025.ogg` doesn't reflect the written message above and is just an example.

# Playing Background Music
You can play background music (BGM) using the [`@bgm`](https://github.com/suika2engine/suika2/wiki/3.-Suika2Script-and-Commands#playing-background-music-bgm) command.

Insert the following into `init.txt`:

```
@bgm 01.ogg
Playing BGM.
```

`@bgm` is the command to play BGM and `01.ogg` is a file inside the `bgm/` folder.

After saving it, run `suika.exe`. Suika2 will now play the selected music file and display a message.

**Note:** Suika2 can only play sound files encoded using the `Ogg Vorbis 44.1kHz stereo or monaural` format.

# Showing Options
A game may have options for multiple endings or other branches.

Insert the following into `init.txt`:

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

After saving it, run `suika.exe`. Suika2 will show three options, [`@choose`](https://github.com/suika2engine/suika2/wiki/3.-Suika2Script-and-Commands#showing-options-choose) is the command to show these options (you can create up to eight options with this command); `label1`, `label2`, and `label3` are jump destinations. Option messages follow [labels](https://github.com/suika2engine/suika2/wiki/3.-Suika2Script-and-Commands#defining-a-label).

`:label1` is a `label` to specify a jump destination. Label lines are ignored when they are executed.

# Set a Flag and Branch
Next, we set a flag and create a branch. Insert the following into `init.txt`:

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

After saving it, run `suika.exe`. Suika2 will show three options. If you choose "By foot.", the message "I found money on the ground." is shown.

Here, we use a combination of [`@set`](https://github.com/suika2engine/suika2/wiki/3.-Suika2Script-and-Commands#setting-a-variable-set) and [`@if`](https://github.com/suika2engine/suika2/wiki/3.-Suika2Script-and-Commands#branching-by-variable-if). Please see [operators](https://github.com/suika2engine/suika2/wiki/3.-Suika2Script-and-Commands#branching-by-variable-if) for a list of accepted operators.

`@set` is the command to set variables. `$1` on the LHS means we store to variable at number 1. `=` means simple assignment. Lastly, `1` means an integer of `1`.

All variables are initially `0`. `$1` is set to `1` only when "By foot." is selected.

`@if` is the command to jump by condition. `$1 == 0` means when variable at number `1` is `0`. `end` is the jump destination label.

# Show a Menu
You can make a menu screen from inside Suika2 scripts.

You are encouraged to read about [GUI files](https://github.com/suika2engine/suika2/wiki/4.-Creating-Graphical-User-Interfaces) for an updated and more user-friendly alternative.

Insert the following into `init.txt`:

```
@menu menu.png menu_selected.png START 2 495 314 192 LOAD 316 494 322 203 CONFIG 640 492 317 204 QUIT 960 497 317 201
:START
:LOAD
:CONFIG
:QUIT
```

After saving it, run `suika.exe`. Suika2 will show the title screen of the sample game.

Let's look at this in more detail:

`@menu` is the command to show our menu screen. It uses two images. `@menu menu.png menu_selected.png` means we use `menu.png` and `menu_selected.png` in the `bg/` folder. Compare these images, `menu.png` is an image for when the button isn't being hovered over by the mouse. `menu_selected.png` is an image for when the button is being hovered over by the mouse.

`START 2 495 314 192` creates a button. `START` is the jump destination label. `2 495 314 192` means creating a button with `314x192` size at position `2, 495`.

# Split Script Files
When script files start becoming to combresome to organise and undestand efficeintly, it may be a good idea to split the file. Thankfully, Suika2 supports this functionality by default.

Insert the following into `init.txt`:

```
@bg roof.png 1.0
@ch center 001-fun.png 1.0
inside init.txt
@load second.txt
```

Create a file named `second.txt` in the `txt/` folder as follows:

```
moved to second.txt
```

After saving these files, run `suika.exe`. Suika2 will show "moved to second.txt" after "inside init.txt".

[`@load`](https://github.com/suika2engine/suika2/wiki/3.-Suika2Script-and-Commands#jumping-to-a-script-load) is the command to jump to a specified script file.

# Creating a Package
We can create a single package file that contains scripts, images, sounds, and so on with Suika2's sister application [`suika-pro.exe`](https://github.com/suika2engine/suika2/wiki/5.-Suika2-Pro-for-Creators).

To do so, follow these steps:

* Open `suika-pro.exe`.
* Select `File -> Export Package` in the menu, this will create a `data01.arc` file.

When distributing, only distribute `suika.exe` and `data01.arc` (as well as any video files).

When we have both the package and a normal file, Suika2 uses the normal file.

# Support
If you require further support, please don't hesitate to join our [Discord](https://discord.gg/ZmvXxE8GFg) or send an e-mail to `midori@suika2.com`.
