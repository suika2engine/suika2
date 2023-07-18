================================
Suika2の変種が入っているフォルダ
================================

どのツールもsuika.exeのあるフォルダにコピーして使ってください。

■テスト系ツール
suika-capture.exe ... ゲームの操作を記録して、recordフォルダを生成します
suika-replay.exe  ... recordフォルダの記録を再生します

 ★使い道
  A. テストプレイの記録を残して、誰でもバグを再現できるようにする
  B. 正常に実行できた記録を残しておいて、スクリプト変更後にリプレイを行い、
     結果に変化がない(バグが混入していない)ことを確かめる

 ★TIPS
  - WinMergeというフリーソフトでrecordとreplayを比較できる(差分はバグ)
  - recordを別名に変更してもOKで、それをsuika-replay.exeにドロップすると、
    そのフォルダがリプレイされる

■Windows系
suika-64.exe    ... 64ビット版Windowsで少しだけ高速に動く実行ファイル
suika-arm64.exe ... Surface ProなどのArm64環境でかなり高速に動く実行ファイル

■Mac系
suika-capture.dmg ... Mac版のキャプチャアプリ (TODO: Mac版リプレイアプリ)

■Linux系
suika-linux        ... Linux用の実行ファイル (Steamで配信したい人向け)
suika-linux-replay ... Linuxでリプレイするための実行ファイル (GitHubの自動テスト用)
