![icon](https://github.com/ktabata/suika2/raw/master/doc/icon.png "icon") Suika 2
=================================================================================

Suika 2はWindowsとMacの両方で動くアドベンチャーゲームエンジンです。いわゆるギャルゲー、乙女ゲー、昔ながらの選択肢型アドベンチャーをターゲットに開発されています。

## デモ

![demo](https://github.com/ktabata/suika2/raw/master/doc/screenshot.jpg "screenshot")

## 移植性

正確に言えば、Suika 2はWindows, Mac, Webブラウザ, Android, Linux, FreeBSD, NetBSDで動作します。

Suika 2はプラットフォーム非依存部とハードウェア抽象化レイヤ(HAL)から成ります。プラットフォーム非依存部はANSI Cと少しのGCC pragmaで書かれています。HALは今のところ、C、Objective-C、アセンブラ、そしてJavaで書かれています。

あなたがもしSuika 2を新しいプラットフォームに移植したければ、HALを記述するだけで済みます。

## 参加

気軽にissueを出してください。どんなフィードバックも歓迎です。

## ビルド

`build/README.md` を見てください。

## ライセンス

本ソフトウェアはMIT licenseで配布されています。 game/COPYING をご覧ください。

## 使い方

* Windowsでは:
    * [公式サイト](https://luxion.jp/s2/ja/) からZIPファイルをダウンロードして展開してください
    * `suika2` フォルダをエクスプローラで開いてください
    * `suika.exe` を起動してください

* Macでは:
    * [公式サイト](https://luxion.jp/s2/ja/) からZIPファイルをダウンロードして展開してください
    * `suika2` フォルダをFinderで開いてください
    * `mac.dmg` の中にある `suika` アプリケーションを `suika2` フォルダにコピーしてください
    * `suika2` フォルダにコピーした `suika` アプリケーションを起動してください

* Androidでは:
    * サンプルゲームがGoogle Playにあります。[公式サイト](https://luxion.jp/s2/)をご覧ください
    * ご自身のゲームを作成するには、Android Studioを使用してソースからビルドする必要があります。 `build/README.md` を参照してください

* Linux/FreeBSD/NetBSDでは:
    * ソースからビルドしてください
    * `build/README.md` を参照してください

## マニュアル

* [チュートリアル](https://luxion.jp/s2/en/tutorial.html)
* [チュートリアル動画](https://www.youtube.com/watch?v=9ximIpY0NPo)
* [コマンドリファレンス](https://luxion.jp/s2/en/reference.html)
