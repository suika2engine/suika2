Suika2 命令参考手册
========================

## @bg

这个命令可以更改背景图片。<br/>
在更改背景图片之后，
所有的角色图片从在舞台上消失。

### 用法 1
在1.5秒的淡入后，更改背景图片到 `sample.png`。
```
@bg sample.png 1.5
```

### 用法 2 
立即更改背景图片到 `sample.png`。
```
@bg sample.png
```

### 用法 3
在1.5秒的淡入后，更改背景图片到`sample.png`，
过渡类型是右边缘。
```
@bg sample.png 1.5 c
```

### 用法 4 
在1.5秒的淡入后，更改背景图片颜色。
过渡类型是左边缘。
```
@bg sample.png 1.5 curtain-left
```

### 用法 5
关闭眼睛效果（用于演出）模拟角色闭眼。
```
@bg #000000 1.5 eye-close
```

### 用法 6
关闭眼睛效果（用于演出）模拟角色睁开眼睛。
```
@bg #000000 0
@bg sample.png 1.5 eye-open
```

### 效果

|效果类型                                  |效果名称            |缩略1         |缩略2          |
|------------------------------------------|--------------------|--------------|---------------|
|过渡/溶解(透明度混合)                     |`normal`            |`n`           |(不指定)       |
|基于规则的过渡                            |`rule:file-mame`    |              |               |
|右边缘                                    |`curtain-right`     |`curtain`     |`c`            |
|左边缘                                    |`curtain-left`      |`cl`          |               |
|上边缘                                    |`curtain-up`        |`cu`          |               |
|下边缘                                    |`curtain-down`      |`cd`          |               |
|从右侧滑动                                |`slide-right`       |`sr`          |               |
|从左侧滑动                                |`slide-left`        |`sl`          |               |
|从上边缘滑动                              |`slide-up`          |`su`          |               |
|从下边缘滑动                              |`slide-down`        |`sd`          |               |
|右侧快门                                  |`shutter-right`     |`shr`         |               |
|左侧快门                                  |`shutter-left`      |`shl`         |               |
|上边缘快门                                |`shutter-up`        |`shu`         |               |
|下边缘快门                                |`shutter-down`      |`shd`         |               |
|顺时针擦除                                |`clockwise`         |`cw`          |               |
|逆时针擦除                                |`counterclockwise`  |`ccw`         |               |
|顺时针擦除 (每步20度)                     |`clockwise20`       |`cw20`        |               |
|逆时针擦除 (每步20度)                     |`counterclockwise20`|`ccw20`       |               |
|顺时针擦除 (每步30度)                     |`clockwise30`       |`cw30`        |               |
|逆时针擦除 (每步30度)                     |`counterclockwise30`|`ccw30`       |               |
|打开眼睛                                  |`eye-open`          |              |               |
|关闭眼睛                                  |`eye-close`         |              |               |
|打开眼睛 (纵向)                           |`eye-open-v`        |              |               |
|关闭眼睛 (纵向)                           |`eye-close-v`       |              |               |
|从缝隙打开                                |`slit-open`         |              |               |
|从缝隙关闭                                |`slit-close`        |              |               |
|从缝隙打开 (纵向)                         |`slit-open-v`       |              |               |
|从缝隙关闭 (纵向)                         |`slit-close-v`      |              |               |

效果类型`mask`被删除。请使用`rule:rule-mask.png`代替。

## @bgm

这个命令用来播放背景音乐。
背景音乐文件需要被保存在`bgm`文件夹。
Suika2 只可以播放ogg文件(44.1khz采样率，立体声)和单声道格式。

### 用法 1
播放 `sample.ogg`。
```
@bgm sample.ogg
```

### 用法 2
停止播放背景音乐。
```
@bgm stop
```

### 用法 3
播放背景音乐一次
```
@bgm sample.ogg once
```

### 应用程序
在两秒内淡入背景音乐
```
@vol bgm 0 2
@wait 2
@bgm stop
```

## @ch

这个命令改变角色。

* 角色位置可以是:
    * `center` or `c` (前景居中 front center)
    * `right or `r`
    * `left` or `l`
    * `back` or `b` (背景居中 back center)

效果和 `@bg`一样。

### 用法 1
在0.5秒后，在中心显示`sample.png`。
```
@ch center sample.png 0.5
```

### 用法 2
`center` 可以被简写成`c`.
```
@ch c sample.png 0.5
```

### 用法 3
指定`none`将会使角色消失
```
@ch c none 0.5
```

### 用法 4
当没有指定淡入时间时，变化会立即应用。
```
@ch c sample.png
```

### 用法 5 
指定一个效果。
```
@ch c sample.png 1.0 mask
```

### 用法 6 
指定角色位置偏移。
下面的例子做了100像素的右移和50像素的下移。
```
@ch c sample.png 1.0 n 100 50
```

### 用法 7
为了设置动画原点，加载带有透明度的角色图像。<br/>
透明度的值的范围是从`0`到`255`。<br/>
你也可以指定`show`或者`hide`来替代`255`或者`0`，它们是等价的。
```
@ch c sample.png 1.0 n 0 0 show
```

## @cha

这个命令可以移动一个角色图片。<br/>
关于如何指定角色位置和透明度的细节，请参考"@ch "部分。

### 用法 1
将中间的角色向左移动600px，并以1.0秒的动画时间将其隐藏。
```
@cha center 1.0 move -600 0 hide
```

### 用法 2 Usage 2
和用法1一样，但是使用加速运动。
```
@cha center 1.0 accel -600 0 hide
```

### 用法 3 
和用法1一样，但是使用减速运动。
```
@cha center 1.0 brake -600 0 hide
```

### 用法 4 
在`@cha`之前, 从屏幕外加载一个图片，然后把它移动到屏幕里面。
```
@ch right sample.png 0 n 600 0 hide
@cha right 2.0 move -600 0 show
```

## @chapter

This command sets the chapter title.

### Usage
```
@chapter "Chapter 1"
```

## @choose

这个命令展示多个选项并跳转到指定的标签。
你可以创建最多8个选项。

### 用法
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

## @chs

这个命令立刻更改角色。<br/>
此外，它同时也改变了背景。
角色更改的顺序是 `center`, `right`,  `left` 和 `back`。<br/>
效果指定器和`@bg`一样。

### 用法 1 
在1.0秒后更改在中间或者右边的角色，并且带有1.0秒的淡入。<br/>
其他角色不会被更改。
```
@chs center.png right.png stay stay 1.0
```

### 用法 2 
抹除舞台中间的角色，并且带有1.0秒的过渡。<br/>
其他角色不会被更改。
```
@chs none stay stay stay 1.0
```

### 用法 3 
更改背景，且不在角色上不带有任何改变，且伴随着1.0秒的过渡时间。
```
@chs stay stay stay stay 1.0 background.png
```

### 用法 4
更改中间的角色和背景，并带有一个1.0秒的过渡。
使用窗帘(百叶窗)效果。
```
@chs center.png stay stay stay 1.0 background.png curtain
```

## @click

这条命令指示Suika2在继续之前等待点击。<br/>
在等待点击时，消息框被隐藏。

### Usage
```
@click
```

## @gosub

这个命令跳转到特定的子程序。使用 `@return` 来返回。

### 用法
运行一个子程序 `SUB`。
```
@gosub SUB

:SUB
在此填写子程序内容
@return
```

## @goto

这个命令跳转到一个标签。<br/>
使用 `@goto` 来制作或者打破一个循环。

### 用法 1
跳转到标签 `abc` (循环).
```
:abc
在此填写子程序内容
@goto abc
```

### 用法 2 
显示加载界面。
```
@goto $LOAD
```

### 用法 3
显示保存界面。
```
@goto $SAVE
```

## @gui

This command shows GUI (graphical user interface).
GUI is a replacement of `@menu` and `@retrospect`.

You can show up to 128 buttons on the screen using GUI.
Button types include "jump to label" and "show if variable is set".

GUI definition files are also used for config, save, load and log screens.

### 用法 1
Shows GUI `menu.txt`.
```
@gui menu.txt
```

### 用法 2
Allows right click cancel.
```
@gui menu.txt cancel
```

## @if

如果指定的条件为真，这个命令跳转到特定的标签。
* 可以使用的运算符号有:
    * `>` 大于
    * `>=` 大于等于
    * `==` 等于
    * `<=` 小于等于
    * `<` 小于
    * `!=` 不等于

左值必须是一个变量名 (例如, `$1`).

右值必须是一个整型数字或者变量名。

### 用法
如果变量1的值是1，跳转到标签 `abc`。
```
@if $1 == 1 abc
Variable 1 is not 1.
:abc
```

## 标签(Label)

这个命令创建一个标签，可以用它来设置需要跳转到的目标。<br/>
它们通常被用于在 `@choose`, `@goto`, `@if` 和 `@menu` 命令。

### 用法
```
:JumpTarget
显示一些消息。
@goto JumpTarget
```

## @load

这个命令跳转到另外一个脚本。<br/>
脚本文件需要被放置在`txt`文件夹内。

### 用法
跳转到脚本文件 `001.txt`
```
@load 001.txt
```

## @menu

这个命令展示了一个带有两个图片的菜单。<br/>
`@menu` 可以创建最多16个按钮。

最基础的，默认情况下，第一个图片被展示，<br/>
当一个按钮被悬停时，按钮的贴图会替换成悬停的图片(hover)

请看演示游戏中的一个有用的例子!

注意: 菜单不能用右键取消。

### 用法
```
@menu menu.png menu-selected.png START 640 480 240 120
:START
```

* 参数的含义。
   * menu.png ... 第一个图片
   * menu-selected.png ... 第二个图片
   * START ... 要跳转到的目标标签
   * 640 480 ... 按钮顶部和左侧的位置 (x,y)
   * 240 120 ... 按钮大小 (width, height)

## 消息(Message)

打印消息到消息框。

* 使用 `\n` 来插入一个换行符。
* 使用 `$ + number` 来显示一个变量的值(例子, `$1`).

### 用法
```
Hello, world!
```

## @news

该命令是`@switch`的一个变体。它显示了前四个选项
在屏幕的北部、东部、西部和南部区域。

当指定 "*"时，父选项被隐藏。

## @retrospect

该命令执行 "事件图像目录模式"。
例如，你可以用它来显示玩家已经解锁的CG艺术作品。
显示玩家已经解锁的CG艺术品。

`@retrospect` 和is similar to `@menu`相似，在其使用的两个图像中，
然后它又一些更高级的参数选项。

* `@menu` 和`@retrospect` 的不同之处是:
    * `@retrospect` 可以创建最多12个按钮。
    * `@retrospect` 可以把变量挂在按钮上。如果变量的值是0，按钮就会被隐藏。
    * `@retrospect` 可以背右键取消。

* `@retrospect` 有许多参数。
    * 背景图片。
    * 背景图片 (已选择的按钮
    * 当指定变量为 "0 "时，用于填充按钮的RGB值。
    * 按钮的宽度和高度。
    * 对于每一个按钮
        * 要跳转到的目标标签。
        * 要检查的变量。
        * 按钮的x和y的位置。

### 用法
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

这个命令用于从子程序返回。

### 用法
```
@gosub SUB

:SUB
在这里填写子程序内容。
@return
```

## @se

这个命令播放音效。<br/>
音效文件需要被放置在`se` 文件夹内。<br/>
Suika2只可以播放ogg文件 (44.1khz采样率，立体声或者单声道格式)

### 用法 1
播放一个音效文件。
```
@se click.ogg
```

### 用法 2
停止播放一个音效。
```
@se stop
```

### 用法 3 
重复播放音效。
```
@se sample.ogg loop
```

### 特殊用法
在语音轨道上播放一个声音效果文件，以检查语音音量，而不需要文字信息。
```
@se click.ogg voice
```

## @select

注意: 这个命令过时了，请使用 `@choose` 替代它。

这个命令展示了选项并且跳转到特定的标签<br/>
这个选项的大小被固定为3个。<br>
如果你想要使用一个，或者多个选项，请使用`@choose` 命令。

### 用法
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

## 带有角色标签的名字。

这个语法把文本打印到消息框，把角色名称打印到名称框。

Suika2 只可以在打印消息时播放人声。<br/>
人声文件需要被放置在`voice` 文件夹内。<br/>
Suika2 只可以播放Ogg Vorbis 44.1kHz 立体声和单声道格式。

### 用法 1
打印消息的时候同时打印角色名字。
```
*Name*Hello, world!
```

### 用法 2
打印消息的时候打印角色名字，并且同时播放人声。
```
*Name*001.ogg*Hello, world!
```

### 用法 3
打印消息的时候打印角色名字，并且同时播放beep音频。
```
*Name*@beep.ogg*Hello, world!
```

## @set

这个命令设置一个变量。

本地变量的范围是`$0` 到 `$9999`。
本地变量被存储在每个独立的存档数据文件中。<br/>
全局变量的范围是`$10000` 到 `$10999`<br/>
全局变量在所有的保存数据中共同存储。

注意: 所有的变量名必须是整型数字。

* 可以使用的运算符有:
    * `=` (赋值)
    * `+=` (增加)
    * `-=` (减少)
    * `*=` (乘)
    * `/=` (除)
    * `%=` (取余数)

变量被初始化的值是`0`.

### 用法 1
把`1` 赋值给变量 `$0`.
```
@set $0 = 1
```

### 用法 2
把 `$10`的变量值加上`23`。
```
@set $10 += 23
```

### 用法 3 
设置一个随机数字 (从 0 到 2147483647) 并赋值到变量 `$0`.
```
@set $0 = $RAND
```

### 用法 4 
把变量 `$2` 的值加在 `$1`上面。
```
@set $1 += $2
```

## @setsave

这条命令启用或禁用保存和加载屏幕，在等待信息点击或选项点击时，通过右键点击调用。
当你调用 `@goto $LOAD` 或者 `@goto $SAVE`, `@setsave enable` 被隐含地调用。

### 用法 1
启动保存和加载界面 (默认情况下)。
```
@setsave enable
```

### 用法 2
禁用保存和加载界面。
```
@setsave disable
```

## @shake

这个命令摇动游戏屏幕。

### 用法 1
用1.0秒的时间将屏幕水平晃动3次，幅度为100px。
`horizontal` 可以被简写成`h`.
```
@shake horizontal 1.0 3 100
```

### 用法 2
用1.0秒的时间将屏幕垂直晃动3次，每次100px。
`vertical` 可以被简写成 `v`.
```
@shake vertical 1.0 3 100
```

## @skip

这条命令通过用户互动来启用或禁用定时命令的跳过。<br/>
在启动时显示品牌标志是很有用的。

你可以抑制飞行中的信息显示跳转,
但是在这种情况下,
你不得不使用`@setsave disable` 来禁止保存/加载界面,<br/>
因为不可跳过的模式不会被保存到保存文件中。.

另外, 当在不可跳过模式中不要使用 `@goto $LOAD` 和 `@goto $SAVE`。<br/>
原因与上述相同。

### 用法 1
启用跳过。(默认）
```
@skip enable
```

### 用法 2
禁用跳过。
```
@skip disable
```

## @switch

注意: 这个命令不推荐。 使用`@choose` 替代它。

该命令显示两级选项，<br/>
有八个父选项，<br/>
每个父选项有八个子选项。

### 用法
显示两层的两个选项，共四个。
```
@switch "Parent option 1" "Parent option 2" * * * * * * LABEL1 "Child option 1" LABEL2 "Child option 2" * * * * * * * * * * * * LABEL3 "Child option 3" LABEL4 "Child option 4" * * * * * * * * * * * *
:LABEL1
:LABEL2
:LABEL3
:LABEL4
```

## @video

这个命令播放一个视频文件。
目前，这一功能只在Windows上启用。

推荐的视频格式是`.WMV` 文件，原因是特许权使用费。
不推荐这样做，但在Windows 10/11上也支持包含 `H.264`+`AAC`的`.AVI`文件。

视频文件被保存在`mov` 目录。
`mov` 目录不会被保存在`data01.arc` 文件内。

### 用法
播放一个视频文件。
```
@video sample.avi
```

## @vol

这个命令用来设置声音大小。

* 声音通道游戏:
    * `bgm` for 背景音乐。
    * `voice` for 人声。
    * `se` for 音效。

这三个通道的音量被储存在每个本地保存文件中。<br/>
你可以经常改变音量，以达到声音制作的目的。

如果你想设置在不同的保存文件之间通用的"全局音量"，<br/>
你可以使用大写字母的通道名称。<br/>
当你设置全局音量时，将渐变时间设置为0。

### 用法 1
在1.0秒内淡入bgm音量到0.5。
```
@vol bgm 0.5 1.0
```

### 用法 2
`bgm` 可以被简写成`b`.
```
@vol b 0.5 1.0
```

### 用法 3 
设置背景音的全局音量到0.5 (这在你创建声音配置菜单时很有用。)
```
@vol BGM 0.5 0
```

## @wait

这条命令指示Suika2等待来自键盘或鼠标的输入后再继续。
来自键盘或鼠标的输入将中断`@wait`（从而允许Suika2继续）。

如果前一个命令是一条消息，在等待输入时，消息框是可见的。
否则，当在等待输入时消息框会被隐藏。

### 用法
等待5秒。
```
@wait 5.0
```
