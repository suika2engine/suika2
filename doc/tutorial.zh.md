Suika2 Tutorial
================

欢迎来到Suika2的世界！

这个文档说明了如何使用Suika2制作一个视觉小说。<br/>
这很简单。

准备好了吗？让我们开始吧！

## 准备工作

在这里，我们将会使用Windows操作系统。

首先，[下载](https://suika2.com/en/) Suika2二进制文件。

解压下载的ZIP文件。我们将会看到下面的文件和文件夹。<br/>
`suika.exe` 是Suika2的可执行文件。其他文件夹包含了一个范例游戏。

* `suika2/`
    * `bg/` 用来保存背景图片文件的文件夹。
    * `bgm/` 用来保存背景音乐文件的文件夹。
    * `cg/` 用来保存系统图片文件，如消息框的文件夹。
    * `ch/` 用来保存角色图片文件的文件夹。
    * `conf/` 用来保存设置文件的文件夹。
    * `cv/` 用来保存人声文件的文件夹。
    * `font/` 用来保存字体文件的文件夹。
    * `se/` 用来保存音效文件的文件夹。
    * `tool/` 这个文件夹里包含了打包工具。
    * `txt/` 用来保存脚本文件的文件夹。
    * `suika.exe` 给Windows使用的可执行应用程序。

这个教程对范例游戏做了一些更改。


## 打开脚本文件

我们可以在 `txt` 文件夹里找到一个叫做 `init.txt` 的文件。<br/>
这个文件是在Suika2启动后被加载的第一个文件。<br/>
如果你使用一个文本编辑器打开这个文件，你将会看见那个脚本文件是一个纯文本文件。

## 显示背景图片

第一步，我们将更改背景图片。<br/>
这时一个最基础的命令。

用下面的代码替换 `init.txt` 里面的内容：

```
@bg roof.png 1.0
@click
```

在保存它之后，运行 `suika.exe`。<br/>
Suika2将会显示一个屋顶阳台。Suika2会等待一个点击，然后退出。

`@bg` 是显示背景图片的命令。
`roof.png` 是一个在 `bg` 文件夹里的文件。
`1.0` 是淡入的时间（以秒为单位）。

## 显示角色图片

要显示角色图片就像使用 `@bg` 命令一样。
用下面的代码替换 `init.txt` 的内容：

```
@bg roof.png 1.0
@ch center 001-fun.png 1.0
@click
```

在保存它之后，运行 `suika.exe`。<br/>
Suika2将会显示一个屋顶阳台，一个角色并等待点击，然后退出。

`@ch` 是用来显示一个角色的命令。
`center` 是在横向居中的位置显示角色。
`001-fun.png` 是在 `ch` 文件夹里的一个文件。
`1.0` 是淡入的时间（以秒为单位）

## 显示消息

显示对话是视觉小说的基础。
你可以在消息框里打印文字。
用下面的代码替换 `init.txt` 里的内容：

```
@bg roof.png 1.0
@ch center 001-fun.png 1.0
Hi, my name is Midori.
*Midori*Message can be shown with character name.
*Midori*011.ogg*Message can be shown with voice.
```

在保存它之后， `suika.exe`。<br/>
Suika2将会展示一个屋顶阳台，一个角色，三个消息，然后退出。

任何不以@, : 或者 \* 开头的脚本行被认为是信息（文本）。

## 播放背景音乐

你可使用 `@bgm` 命令播放背景音乐。<br/>
使用下面的代码替换 `init.txt` 里的内容：

```
@bgm 01.ogg
@bg roof.png 1.0
@ch center 001-fun.png 1.0
Playing BGM.
```

在保存它之后，运行 `suika.exe`.<br/>
Suika2将会播放音乐。<br/>
`@bgm` 是播放背景音乐的命令。
`01.ogg` 是在 `bgm` 文件夹内的一个文件。

Suika2只可以播放Ogg Vorbis 44.1KHZ的音频文件（立体声或者单声道）格式。

## 显示选项

一个游戏也许带有多个结局。<br/>
使用下面的代码替换 `init.txt` 的内容:

```
@bg roof.png 1.0
@ch center 001-fun.png 1.0
Ok, I'm gonna go to school.
@choose label1 "By foot." label2 "By bicycle." label3 "I decided not to."
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

在保存它之后，运行 `suika.exe`。<br/>
Suika2将会显示三个选项。<br/>
`@choose`是用来显示选项的命令。<br/>
`label1`, `label2` 和 `label3` 是跳转目标。<br/>
选项信息跟随标签。

`:label1` 是一个标签，用于指定跳转目的地。<br/>
标签行在执行时被忽略。

## 设置一个标志和分支

接下来，我们设置一个标志并创建一个分支。
使用下面的代码替换 `init.txt` 的内容。

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

在保存它之后，运行 `suika.exe`。<br/>
Suika2将会显示三个选项。<br/>
如果你选择"By foot.", 这个消息"I found money on the ground." 会被展示。<br/>

在这里，我们将组合使用`@set` 和 `@if`这两个命令。

`@set` 是用来设置变量的命令。<br/>
`$1` 在左侧表示的变量名是1。<br/>
`=` 表示赋值<br/>
最后， `1` 表示整型数字1。

所以的变量的初始值都是0。<br/>
只有当"By foot." 被选择的时候，`$1` 被设置成 `1` 。

`@if` 是用来判断跳转条件的命令。<br/>
`$1 == 0` 的意思是 `变量1的值是`。<br/>
`end` 是跳转目的地标签。

## 显示菜单

你可以制作一个菜单界面。
用下面的代码替换`init.txt` 的内容：

```
@menu menu.png menu_selected.png START 2 495 314 192 LOAD 316 494 322 203 CONFIG 640 492 317 204 QUIT 960 497 317 201
:START
:LOAD
:CONFIG
:QUIT
```

在保存它之后，运行 `suika.exe`。<br/>
Suika2将会显示这个范例游戏的标题界面。<br/>

让我们仔细看一下代码细节：

`@menu` 是现用来显示菜单界面的命令。他使用两个图像。<br/>
`@menu menu.png menu_selected.png` 的意思是我们可以使用 `menu.png` 和 `menu_selected.png` 在 `bg` 文件夹内。<br/>
比较这些图像。<br/>
`menu.png` 是一个图像，用于按钮没有被鼠标指向的时候。<br/>
`menu_selected.png` 是一个图像，用于当按钮被鼠标指向的时候。<br/>
`START 2 495 314 192` 创建一个按钮。<br/>
`START` 是跳转目的地标签。<br/>
`2 495 314 192` 意思是在(2, 495)的位置创建一个314x192大小的按钮。<br/>

## 拆分脚本文件

对作者来说，维护一个大的脚本文件是很困难的。<br/>
因此，我们有一种方法来分割脚本文件。<br/>

使用下面的代码替换 `init.txt` 的内容：

* init.txt
```
@bg roof.png 1.0
@ch center 001-fun.png 1.0
inside init.txt
@load second.txt
```

在 `txt`文件夹内创建 `second.txt` 。

* second.txt
```
moved to second.txt
```

在保存这些文件之后，运行 `suika.exe`。<br/>
Suika2将会在 "inside init.txt" 之后显示 "moved to second.txt" 。

`@load` 是用来跳转到特定脚本文件的命令。

## 创建资源包

我们可以创建一个简单的资源包文件，把脚本，文件，声音等文件存进去。<br/>
请按下面的步骤操作：

1. 双击并打开 `suika-pro.exe`. `Suika2 Pro for Creators` 
2. 在菜单栏中点击 `File`，然后点击`Export package`。这将创建一个文件， `data01.arc`.
3. 我们应该只发布 `suika.exe` 和 `data01.arc`。

## 翻译者补充内容

我们也可以不通过suika-pro.exe，直接使用pacakge.exe打包资源文件。

请按下面的步骤操作：

1. 复制 `tool/package-win.exe` 到上一级文件夹
2. 双击运行打包程序，这将创建一个 `data01.arc`
3. 我们应该只发布 `suika.exe` 和 `data01.arc`。


当我们同时拥有资源包和默认文件时，Suika2使用默认文件。
