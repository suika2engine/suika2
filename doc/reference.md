Suika 2 Command Reference
=========================

## @bg

Change background image.
After changing background image,
character images vanish from stage.

* Usage 1: Change background image to `sample.png` with fade-in time 1.5 sec.
```
@bg sample.png 1.5
```

* Usage 2: Change background image to `sample.png` immediately.
```
@bg sample.png
```

* Usage 3: Change background image to `sample.png` with fade-in time 1.5 sec. Fading type is curtain.
```
@bg sample.png 1.5 c
```

## @bgm

Play BGM.
BGM files are required to be stored in `bgm` folder.
Suika 2 can play only Ogg Vorbis 44.1kHz stereo or monaural format.

* Usage 1: Play `sample.ogg`.
```
@bgm sample.ogg
```

* Usage 2: Stop BGM.
```
@bgm stop
```

* Application: Fade-out BGM in 2 sec.
```
@vol bgm 0 2
@wait 2
@bgm stop
```

## @ch

Change character.

* Character positions are:
    * `center` (front center)
    * `right`
    * `left`
    * `back` (back center)

* Usage 1: Display `sample.png` in 0.5 sec at front center.
```
@ch center sample.png 0.5
```

* Usage 2: `center` can be abbreviated as `c`.
```
@ch c sample.png 0.5
```

* Usage 3: Specify `none` to vanish character.
```
@ch c none 0.5
```

* Usage 4: Change is applied immediately when fade-in time is not specified.
```
@ch c sample.png
```

## @click

Wait for a click.
While waiting for a click, message box is hidden.

* Usage:
```
@click
```

## @gosub

Jump to subroutine. Use `@return` to return.

* Usage: Run subroutine `SUB`.
```
@gosub SUB

:SUB
Describe process here.
@return
```

## @goto

Jump to label.
Use `@goto` to make a loop, or break a loop.

* Usage: Jump to label `abc` (loop).
```
:abc
Describe process here.
@goto abc
```

## @if

Jump to label if condition is true.

* Available operators are:
    * `>` greater than
    * `>=` greater than or equal to
    * `==` equal to
    * `<=` less than or equal to
    * `<` less than
    * `!=` not equal to

* LHS must be a variable name e.g. `$1`

* RHS must be a integer or a variable name.

* Usage 1: Jump to label `abc` if variable at number 1 is greater than or equal to 10.
```
@if $1 >= 10 abc
Variable 1 is less than 10.
:abc
```

## @menu

Display menu using two images.
`@menu` can create up to four buttons.

Basically the first image is displayed,
but while a button is pointed by mouse,
the area of the button is changed to the second image.

Menu is not cancelable by right click.

* Usage:
```
@menu menu.png menu-selected.png START 640 480 240 120
:START
```

## Label

Create label to be a jump target.
Use with `@goto`, `@gosub`, `@if`, `@menu`, `@retrospect` and `@switch`.

* Usage:
```
:JumpTarget
Show some messages.
@goto JumpTarget
```

## Message

Print text to the message box.

* Use `\n` to insert line feed.
* Use `$ + number` to show value of variable. (e.g. $1)

* Usage:
```
Hello, world!
```

## @load

Jump to other script.
Script files are required to be in `txt` folder.

* Usage: Jump to script file `001.txt`.
```
@load 001.txt
```

## @retrospect

Execute event image catalog mode.
In a similar way to `@menu`, `@retrospect` uses two images.

* The difference between `@menu` and `@retrospect` is:
    * `@retrospect` can create up to twelve buttons.
    * `@retrospect` can specify variable to button. If the value of the variable is 0, the button is hidden.
    * `@retrospect` is cancelable by right click.

* `@retrospect` has many parameters.
    * background image.
    * background image (selected button).
    * RGB values to fill buttons when specified variable is `0`.
    * Width of buttons, height of buttons
    * To the number of buttons:
        * Jump target label
        * Variable to check
        * X and Y position of button

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

Return from subroutine.

* Usage:
```
@gosub SUB

:SUB
Describe process here.
@return
```

## @se

Play sound effecct.
Sound effect files are required to be in `se` folder.
Suika 2 can play only Ogg Vorbis 44.1kHz stereo or monaural format.

* Usage:
```
@se click.ogg
```

## @select

Show options and jump to label.
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

Print text to the message box, and print character name to name box.

Suika 2 can play voice when printing message.
Voice files are required to be in `voice` folder.
Suika 2 can play only Ogg Vorbis 44.1kHz stereo or monaural format.

* Usage 1: Print message with character name.
```
*Name*Hello, world!
```

* Usage 2: Print message with character name and voice.
```
*Name*001.ogg*Hello, world!
```

* Usage 3: Print message with character name and beep.
```
*Name*@beep.ogg*Hello, world!
```

## @set

Set value to variable.

Local variables are `$0` to `$9999`.
Local variable is stored in every save data.

Global variables are `$10000` to `$10999`.
Global variable is stored commonly across all save data.

* Available operators are:
    * `=` (assignment)
    * `+=` (addition)
    * `-=` (subtraction)
    * `*=` (multiplication)
    * `/=` (division)
    * `%=` (remainer)

Initial value of variable is `0`.

* Usage 1: Set value `1` to variable `$0`.
```
@set $0 = 1
```

* Usage2: Add value`23` to variable `$10`.
```
@set $10 += 23
```

* Usage 3: Set random number (0 to 2147483647) to variable `$0`.
```
@set $0 = $RAND
```

* Usage 4: Add value of variable `$2` to variable `$1`.
```
@set $1 += $2
```

## @switch

Show up to two levels of options.
Number of parent options are eight,
and each parent option has eight child options.

* Usage 1: Show one level options.
```
@switch "Parent option 1" "Parent option 2" * * * * * * LABEL1 * * * * * * * * * * * * * * * LABEL2 * * * * * * * * * * * * * * *
```

* Usage 2: Show two levels options.
```
@switch "Parent option 1" "Parent option 2" * * * * * * LABEL1 "Child option 1" LABEL2 "Child option 2" * * * * * * * * * * * * LABEL3 "Child option 3" LABEL4 "Child option 4" * * * * * * * * * * * *
```

## @vol

Set sound volume.

* Sound channels are:
    * `bgm` for BGM
    * `voice` for voice
    * `se` for sound effect

* Usage 1: Set BGM volume to 0.5 in 1 sec.
```
@vol bgm 0.5 1.0
```

* Usage 2: `bgm` can be abbreviated as `b`.
```
@vol b 0.5 1.0
```

## @wait

Wait.
Keyboard input and mouse click interrupt `@wait`.
Message box is hidden while `@wait`.

* Usage: Wait of 5 sec.
```
@wait 5.0
```
