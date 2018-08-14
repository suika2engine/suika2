![icon](https://github.com/ktabata/suika2/raw/master/doc/icon.png "icon") Suika 2
=================================================================================

Suika 2はWindowsとMacの両方で動くアドベンチャーゲームエンジンです。いわゆるギャルゲー、乙女ゲー、昔ながらの選択肢型アドベンチャーをターゲットに開発されています。

## デモ

![demo](https://github.com/ktabata/suika2/raw/master/doc/screenshot.jpg "screenshot")

## 移植性

正確に言えば、Suika 2はWindowsとMac、Linux、そしてAndroidで動作します。

Suika 2はプラットフォーム非依存部とハードウェア抽象化レイヤ(HAL)から成ります。プラットフォーム非依存部はANSI Cと少しのGCC pragmaで書かれています。HALは今のところ、C、Objective-C、そしてJavaで書かれています。

あなたがもしSuika 2を新しいプラットフォームに移植したければ、HALを記述するだけで済みます。

## 参加

気軽にissueを出してください。どんなフィードバックも歓迎です。

## ビルド

build/BUILD.md を見てください。

## ライセンス

本ソフトウェアはMIT licenseで配布されています。 suika2/COPYING をご覧ください。

## 使い方

* Windowsでは:
    * リポジトリを`git clone`するか、ZIPファイルをダウンロードしてください。
    * `suika2` フォルダをエクスプローラで開いてください。
    * `suika.exe` をダブルクリックして起動してください。

* Macでは:
    * リポジトリを`git clone`するか、ZIPファイルをダウンロードしてください。
    * `suika2-mac` フォルダをFinderで開いてください。
    * `Suika` を `アプリケーション` フォルダにコピーしてください。このステップは必須です。
    * `アプリケーション` の `Suika` をダブルクリックして起動してください。

* Linuxでは:
    * リポジトリを`git clone`するか、ZIPファイルをダウンロードしてください。
    * ターミナルで `suika2` ディレクトリに入ってください。
    * `./suika-linux` を実行してください。
