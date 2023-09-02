================================
Suika2の変種が入っているフォルダ
================================

どのツールもsuika.exeのあるフォルダにコピーして使ってください。

■Windows系
suika-64.exe    ... 64ビット版Windowsで少しだけ高速に動く実行ファイル
suika-arm64.exe ... Surface ProなどのArm64環境でかなり高速に動く実行ファイル

■Linux系
suika-linux     ... Linux用の実行ファイル (Steamで配信したい人向け)

■テスト系ツール
suika-capture.exe ... ゲームの操作を記録して、recordフォルダを生成します
suika-replay.exe  ... recordフォルダの記録を再生して、replayフォルダを生成します

 ★使い道
  A. テストプレイの記録を残して、誰でもバグを再現できるようにする
  B. 正常に実行できた記録を残しておいて、Suika2のバージョンアップ後にリプレイを行い、
     結果に変化がない(バグが混入していない)ことを確かめる

 ★TIPS
  - 別途配布されているsuika-compare.exeでrecordとreplayを比較できます(差分はバグ)
  - recordを別名に変更してもOKで、それをsuika-replay.exeにドロップすると、
    そのフォルダがリプレイされます
  - リプレイするだけなら、recordの中のスクリーンショットを消してmain.csvだけでOK
