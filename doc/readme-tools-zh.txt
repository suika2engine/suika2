================================
Suika2 变体文件夹
================================

■适用于 Windows
suika-64.exe    ... 在 64 位版 Windows 上速度略有提升的可执行文件
suika-arm64.exe ... 在像 Surface Pro 这种 Arm64 环境下速度大幅提升的可执行文件

请将以上文件复制到含有 suika.exe 的文件夹中使用。

■测试工具
suika-capture.exe ... 记录游戏操作并生成 record 文件夹
suika-replay.exe  ... 播放 record 文件夹中的记录并生成 replay 文件夹

请将以上文件复制到含有 suika.exe 的文件夹中使用。

★使用途径
  A. 保留测试游戏记录，使任何人都能复现错误。
  B. 保留正常运行的记录，升级 Suika2 后进行自���回放，并确认结果无变化（即验证 Suika2 是否出现错误）。

★提示
  - 可以使用额外提供的 suika-compare.exe 比较 record 和 replay（差异表示错误）。
  - 可以将 record 重命名，然后将其拖放到 suika-replay.exe 上。
  - 如果仅回放，仅保留 record 中的 main.csv 并删除屏幕截图即可。

■适用于 macOS（暂不包含）
suika-capture.dmg ... macOS 的屏幕捕捉应用
suika-replay.dmg  ... macOS 的回放应用

DMG 中包含应用程序。
请将以上文件复制到含有 suika.exe 的文件夹中使用。

■适用于 Linux
suika-linux ... 适用于 Linux 的可执行文件（面向希望通过 Steam 发行的用户）

请将以上文件复制到含有 suika.exe 的文件夹中使用。
执行前仅需运行一次 `chmod +x suika-linux`。

■其他
android-src ... 适用于 Android 的源代码
ios-src     ... 适用于 iOS 的源代码
installer   ... 适用于 Windows 的安装程序素材

这些文件将用于 Suika2 Pro 导出，您不需要过多关注。
