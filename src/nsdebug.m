// -*- tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changed]
 *  - 2020/06/11 Created.
 */

#import "nsdebug.h"

DebugWindowController *debugWindowController;

BOOL isResumePressed;
BOOL isNextPressed;
BOOL isPausePressed;

//
// DebugWindowController
//

@interface DebugWindowController ()
@end

@implementation DebugWindowController

- (void)windowDidLoad {
    [super windowDidLoad];
}

- (IBAction) onResumeButton:(id)sender
{
    isResumePressed = TRUE;
}

- (IBAction) onNextButton:(id)sender
{
    isNextPressed = TRUE;
}

- (IBAction) onPauseButton:(id)sender
{
    isPausePressed = TRUE;
}

@end

//
// nsmain.cへ提供する機能
//

//
// デバッグウィンドウを初期化する
//
BOOL initDebugWindow(void)
{
    assert(debugWindowController == NULL);

    debugWindowController = [[DebugWindowController alloc]
                                  initWithWindowNibName:@"DebugWindow"];
    if(debugWindowController == NULL)
        return FALSE;

    [debugWindowController showWindow:debugWindowController];

	return TRUE;
}

/*
 * platform.h
 */

/*
 *再開ボタンが押されたか調べる
 */
bool is_resume_pushed(void)
{
    BOOL ret = isResumePressed;
    isResumePressed = FALSE;
    return ret;
}

/*
 * 次へボタンが押されたか調べる
 */
bool is_next_pushed(void)
{
    BOOL ret = isNextPressed;
    isNextPressed = FALSE;
    return ret;
}

/*
 * 停止ボタンが押されたか調べる
 */
bool is_pause_pushed(void)
{
    BOOL ret = isPausePressed;
    isPausePressed = FALSE;
    return ret;
}

/*
 * 実行するスクリプトファイルが変更されたか調べる
 */
bool is_script_changed(void)
{
    return false;
}

/*
 * 変更された実行するスクリプトファイル名を取得する
 */
const char *get_changed_script(void)
{
    return "";
}

/*
 * 実行する行番号が変更されたか調べる
 */
bool is_line_changed(void)
{
    return false;
}

/*
 * 変更された実行するスクリプトファイル名を取得する
 */
int get_changed_line(void)
{
    return 0;
}

/*
 * スクリプトがアップデートされたかを調べる
 */
bool is_script_updated(void)
{
    return false;
}

/*
 * コマンドがアップデートされたかを調べる
 */
bool is_command_updated(void)
{
    return false;
}

/*
 * アップデートされたコマンド文字列を取得する
 */
const char *get_updated_command(void)
{
    return "";
}

/*
 * コマンドの実行中状態を設定する
 */
void set_running_state(bool running, bool request_stop)
{
}

/* デバッグ情報を更新する */
void update_debug_info(bool script_changed)
{
}
