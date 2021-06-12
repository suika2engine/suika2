Suika 2 Command Reference
=========================

## @bg

This command changes the background image.
After changing the background image,
character images vanish from the stage.

* You can specify following effects:
    * `normal`, `n` or not-specified -> default alpha blending
    * `mask` or `m` → 28-level pointillism
    * `curtain-right`, `curtain`, `cr` or `c` -> right curtain
    * `curtain-left` or `cl` -> left curtain
    * `curtain-up` or `cu` -> up curtain
    * `curtain-down` or `cd` -> down curtain
    * `slide-right` or `sr` -> right slide
    * `slide-left` or `sl` -> left slide
    * `slide-up` or `su` -> up slide
    * `slide-down` or `sd` -> down slide

* Usage 1: Changes the background image to `sample.png` with a 1.5 second fade-in time.
```
@bg sample.png 1.5
```

* Usage 2: Changes the background image to `sample.png` immediately.
```
@bg sample.png
```

* Usage 3: Changes the background image to `sample.png` with a 1.5 second fade-in time. Fading type is right curtain.
```
@bg sample.png 1.5 c
```

* Usage 4: Changes the background color with a 1.5 second fade-in time. Fading type is left curtain.
```
@bg sample.png 1.5 curtain-left
```

## @bgm

This command plays BGM.
BGM files need to be stored in the `bgm` folder.
Suika 2 can only play Ogg Vorbis 44.1kHz stereo and monaural format.

* Usage 1: Plays `sample.ogg`.
```
@bgm sample.ogg
```

* Usage 2: Stops BGM.
```
@bgm stop
```

* Application: Fade-out BGM in 2 seconds.
```
@vol bgm 0 2
@wait 2
@bgm stop
```

## @ch

This command changes the character.

* Character positions are:
    * `center` (front center)
    * `right`
    * `left`
    * `back` (back center)

* Effects are:
    * `normal`, `n` or not-specified -> alpha blending
    * `mask` or `m` -> 28-level pointillism

* Usage 1: Displays `sample.png` in 0.5 seconds at front center.
```
@ch center sample.png 0.5
```

* Usage 2: `center` can be abbreviated as `c`.
```
@ch c sample.png 0.5
```

* Usage 3: Specifying `none` will cause the character to vanish.
```
@ch c none 0.5
```

* Usage 4: Changes are applied immediately when the fade-in time is not specified.
```
@ch c sample.png
```

* Usage 5: Specify effect.
```
@ch c sample.png 1.0 mask
```

* Usage 6: Specify offset of character position. Following example does 100-pixel right shift and 50-pixel down shift.
```
@ch c sample.png 1.0 n 100 50
```

* Usage 7: In order to set animation origin, load character image with alpha value. The range of alpha value is `0` to `255`. You can also specify `show` as `255` or `hide` as `0`.
```
@ch c sample.png 1.0 n 0 0 show
```

## @cha

This command moves a character image.
Refer to `@ch` section for details of the specifier of character position and alpha value.

* Usage 1: Moves the center character to 600-px left and hide it with a 1.0 second animation time.
```
@cha center 1.0 move -600 0 hide
```

* Usage 2: Same as Usage 1, but accelerates movement.
```
@cha center 1.0 accel -600 0 hide
```

* Usage 3: Same as Usage 1, but deaccelerates movement.
```
@cha center 1.0 brake -600 0 hide
```

* Usage 4: Before `@cha`, loads a character image outside the screen, then moves it into the screen.
```
@ch right sample.png 0 n 600 0 hide
@cha right 2.0 move -600 0 show
```

## @click

This command instructs Suika 2 to wait for a click before continuing.
While waiting for a click, the message box is hidden.

* Usage:
```
@click
```

## @gosub

This command jumps to the specified subroutine. Use `@return` to return.

* Usage: Runs subroutine `SUB`.
```
@gosub SUB

:SUB
Describe the process here.
@return
```

## @goto

This command jumps to a label.
Use `@goto` to make or break a loop.

* Usage 1: Jumps to the label `abc` (loop).
```
:abc
Describe the process here.
@goto abc
```

* Usage 2: Show load screen.
```
@goto $LOAD
```

## @if

This command jumps to the specified label if the specified condition is true.

* Available operators are:
    * `>` greater than
    * `>=` greater than or equal to
    * `==` equal to
    * `<=` less than or equal to
    * `<` less than
    * `!=` not equal to

* LHS must be a variable name (e.g., `$1`).

* RHS must be an integer or a variable name.

* Usage 1: Jumps to the label `abc` if the variable at number 1 is greater than or equal to 10.
```
@if $1 >= 10 abc
Variable 1 is less than 10.
:abc
```

## Label

This creates a label to be used as a jump target.
Use with `@goto`, `@gosub`, `@if`, `@menu`, `@retrospect` and `@switch`.

* Usage:
```
:JumpTarget
Show some messages.
@goto JumpTarget
```

## @load

This command jumps to another script.
Script files need to be in the `txt` folder.

* Usage: Jumps to the script file `001.txt`.
```
@load 001.txt
```

## @menu

This command displays a menu using two images.
`@menu` can create up to 16 buttons.

Basically the first image is displayed,
but while a button is pointed at by the mouse,
the area of the button is changed to the second image.

Menus can't be canceled using right click.

* Usage:
```
@menu menu.png menu-selected.png START 640 480 240 120
:START
```

* Meanings of parameters:
   * menu.png ... first image
   * menu-selected.png ... second image
   * START ... jump target label
   * 640 480 ... top-left position of button
   * 240 120 ... button size

## Message

Prints text to the message box.

* Use `\n` to insert a line feed.
* Use `$ + number` to show the value of a variable (e.g., $1).

* Usage:
```
Hello, world!
```

## @news

This command is a variant of `@switch`. It shows first four options on
north, east, west and south of screen.

Parent options are hidden when `*` is specified.

## @retrospect

This command executes event image catalog mode.
Similarly to `@menu`, `@retrospect` uses two images.

* The difference between `@menu` and `@retrospect` is:
    * `@retrospect` can create up to twelve buttons.
    * `@retrospect` can hook variables to buttons. If the value of the variable is 0, the button is hidden.
    * `@retrospect` can be canceled using right click.

* `@retrospect` has many parameters.
    * background image.
    * background image (selected button).
    * RGB values to fill buttons when the specified variable is `0`.
    * The width and height of buttons.
    * For each button:
        * The jump target label.
        * The variable to check.
        * The X and Y position of the button.

* Usage:
```
@retrospect menu.png menu-selected.png 0 0 0 320 240 PICTURE1 $1 320 240
Canceled.
@goto END
:PICTURE1
@bg picture1.png 1.0
@click
:END
```

## @return

This is used to return from subroutines.

* Usage:
```
@gosub SUB

:SUB
Describe the process here.
@return
```

## @se

This command plays sound effects.
Sound effect files need to be in the `se` folder.
Suika 2 can only play Ogg Vorbis 44.1kHz stereo and monaural format.

* Usage:
```
@se click.ogg
```

* Special usage: Plays sound effect file on voice track to check voice volume without text message.
```
@se click.ogg voice
```

## @select

This command shows options and jumps to the specified label.
The number of options is fixed to three.

* Usage:
```
@select label1 label2 label3 "Good morning." "Good afternoon." "Good evening."
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

## Message with character name

This prints text to the message box, and the character name to the name box.

Suika 2 can play voice files when printing messages.
Voice files need to be in the `voice` folder.
Suika 2 can only play Ogg Vorbis 44.1kHz stereo and monaural format.

* Usage 1: Prints a message with the character name.
```
*Name*Hello, world!
```

* Usage 2: Prints a message with the character name and plays the character's voice.
```
*Name*001.ogg*Hello, world!
```

* Usage 3: Prints a message with the character name and plays a beep.
```
*Name*@beep.ogg*Hello, world!
```

## @set

This sets a value to a variable.

Local variables are `$0` to `$9999`.
Local variables are stored in all save data.

Global variables are `$10000` to `$10999`.
Global variables are stored commonly across all save data.

* Available operators are:
    * `=` (assignment)
    * `+=` (addition)
    * `-=` (subtraction)
    * `*=` (multiplication)
    * `/=` (division)
    * `%=` (remainder)

The initial value of a variable is `0`.

* Usage 1: Sets a value of `1` to the variable `$0`.
```
@set $0 = 1
```

* Usage 2: Adds the value `23` to the variable `$10`.
```
@set $10 += 23
```

* Usage 3: Sets a random number (from 0 to 2147483647) to the variable `$0`.
```
@set $0 = $RAND
```

* Usage 4: Adds the value of the variable `$2` to the variable `$1`.
```
@set $1 += $2
```

## @shake

This command shakes screen.

* Usage 1: Takes 1.0 second to shake screen 3 times horizontally 100px. `horizontal` can be abbreviated as `h`.
```
@shake horizontal 1.0 3 100
```

* Usage 2: Takes 1.0 second to shake screen 3 times vertically 100px. `vertical` can be abbreviated as `v`.
```
@shake vertical 1.0 3 100
```

## @switch

Shows up to two levels of options.
The number of parent options are eight,
and each parent option has eight child options.

* Usage 1: Shows one level option.
```
@switch "Parent option 1" "Parent option 2" * * * * * * LABEL1 * * * * * * * * * * * * * * * LABEL2 * * * * * * * * * * * * * * *
```

* Usage 2: Shows two level options.
```
@switch "Parent option 1" "Parent option 2" * * * * * * LABEL1 "Child option 1" LABEL2 "Child option 2" * * * * * * * * * * * * LABEL3 "Child option 3" LABEL4 "Child option 4" * * * * * * * * * * * *
```

## @vol

This command sets the sound volume.

* Sound channels are:
    * `bgm` for BGM.
    * `voice` for voice.
    * `se` for sound effects.

Sound volumes of these three channels are stored in each local save file.
You can change the volumes frequently for sound production purpose.

If you want to set global master volumes which are common between save files, you can use CAPITAL name of channels. When you set global master volumes, set fading time to 0.

* Usage 1: Sets BGM volume to 0.5 in 1 second.
```
@vol bgm 0.5 1.0
```

* Usage 2: `bgm` can be abbreviated as `b`.
```
@vol b 0.5 1.0
```

* Usage 3: Sets global master volume of BGM to 0.5
```
@vol BGM 0.5 0
```

## @wait

This command instructs Suika 2 to wait for input from the keyboard or mouse before continuing.
Input from the keyboard or mouse will interrupt `@wait` (thus allowing Suika 2 to continue).
While waiting for input, the message box is hidden.

* Usage: Waits for 5 seconds.
```
@wait 5.0
```
