How to add a command
====================

* `script.h`
    * Add a constant to `enum command_type`
        * The new constant shows a new command we'll add
    * Create `enum xxx_command_param`
        * Define command parameter indices like other `enum xxx_command_param`

* `script.c`
    * Add a command to `struct insn_item insn_tbl[]`

* `main.h`
    * Add a prototype declaration of `xxx_command()`

* `main.c`
    * Add a command dispatch in `dispatch_command()`

* `cmd_xxx.c`
    * Create a file

* `common.mk`
    * Add `cmd_xxx.c`
