コマンドの追加手順
==================

* script.h
    * enum command_type にコマンドを表す定数を追加する
    * enum xxx_command_param にコマンドのパラメータのインデックスを定義する

* script.c
    * struct insn_item insn_tbl[] にコマンドを追加する

* main.h
    * xxx_command() のプロトタイプ宣言を追加する

* main.c
    * dispatch_command() でコマンドのディスパッチを追加する

* cmd_xxx.c
    * コマンド別のファイルを作成する

* common.mk
    * cmd_xxx.c を追加する
