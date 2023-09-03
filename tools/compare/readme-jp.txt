Suika2 実行結果比較ツール
=========================

このプログラムは `record` フォルダと `replay` フォルダの内容を比較します。

# 下準備

1. `suika-capture.exe` を実行します
  - 実行記録は `record` フォルダに記録されます
2. `suika.exe` をバージョンアップします
3. `suika-replay.exe` を実行します
  - 実行記録の `record` フォルダが再生されます
  - リプレイ結果は `replay` フォルダに出力されます

# 実行

`suika-compare.exe` を実行して、`record` と `replay` を比較します。

もし実行結果に違いがなければ、"OK: do difference."と表示されます。

もし実行結果に違いがあれば、"BAD: xx diffs."と表示されます。
このとき、差分のある画像の名前が `diff.txt` に出力されます。
