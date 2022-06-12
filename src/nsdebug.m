// -*- tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changed]
 *  - 2020/06/11 Created.
 */

#import <stdio.h>
#import <stdlib.h>
#import <string.h>
#import "suika.h"
#import "nsdebug.h"

// デバッグウィンドウのコントローラ
static DebugWindowController *debugWindowController;

// ボタンが押下されたか
static bool isResumePressed;
static bool isNextPressed;
static bool isPausePressed;
static bool isChangeScriptPressed;
static bool isReloadPressed;

// 前方参照
static NSString *nsstr(const char *utf8str);
static NSString *parseint(int num);

//
// DebugWindowController
//

@interface DebugWindowController ()
@property (weak) IBOutlet NSButton *buttonResume;
@property (weak) IBOutlet NSButton *buttonNext;
@property (weak) IBOutlet NSButton *buttonPause;
@property (weak) IBOutlet NSTextField *textFieldScriptName;
@property (weak) IBOutlet NSButton *buttonScriptNameUpdate;
@property (weak) IBOutlet NSTextField *labelScriptLine;
@property (weak) IBOutlet NSTextField *textFieldScriptLine;
@property (weak) IBOutlet NSButton *buttonScriptLine;
@property (weak) IBOutlet NSTextField *labelCommand;
@property (unsafe_unretained) IBOutlet NSTextView *textFieldCommand;
@property (weak) IBOutlet NSButtonCell *buttonCommandUpdate;
@property (weak) IBOutlet NSTableView *tableViewScript;
@property (weak) IBOutlet NSButton *buttonSearchError;
@property (weak) IBOutlet NSButton *buttonOverwriteScript;
@property (weak) IBOutlet NSButton *buttonReloadScript;
@property (unsafe_unretained) IBOutlet NSTextView *textFieldVariables;
@end

@implementation DebugWindowController

//
// コールバック
//

// ウィンドウがロードされたときのイベント
- (void)windowDidLoad {
    [super windowDidLoad];
}

// ウィンドウが閉じられるときのイベント
- (BOOL)windowShouldClose:(id)sender {
    // メインループから抜ける
    [NSApp stop:nil];
    return NO;
}

- (IBAction) onResumeButton:(id)sender {
    isResumePressed = true;
}

- (IBAction) onNextButton:(id)sender {
    isNextPressed = true;
}

- (IBAction) onPauseButton:(id)sender {
    isPausePressed = true;
}

- (IBAction)onTextFieldScriptNameReturn:(id)sender {
    isChangeScriptPressed = true;
}

//
// ビューの設定/取得
//

// スクリプト名のテキストフィールドの値を設定する
- (void)setScriptName:(NSString *)name {
    [[self textFieldScriptName] setStringValue:name];
}

// スクリプト名のテキストフィールドの値を設定する
- (NSString *)getScriptName {
    return [[self textFieldScriptName] stringValue];
}

// スクリプト行番号のテキストフィールドの値を設定する
- (void)setScriptLine:(int)num {
    NSString *s = [NSString stringWithFormat:@"%d", num];
    [[self textFieldScriptLine] setStringValue:s];
}

// スクリプト行番号のテキストフィールドの値を取得する
- (int)getScriptLine {
    NSString *s = [[self textFieldScriptLine] stringValue];
    int num = atoi([s UTF8String]);
    return num;
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

    // メニューのXibをロードする
    NSBundle *bundle  = [NSBundle mainBundle];
    NSArray  *objects = [NSArray new];
    [bundle loadNibNamed:@"MainMenu" owner:NSApp topLevelObjects:&objects];

    // デバッグウィンドウのXibファイルをロードする
    debugWindowController = [[DebugWindowController alloc]
                                  initWithWindowNibName:@"DebugWindow"];
    if(debugWindowController == NULL)
        return FALSE;

    // デバッグウィンドウを表示する
    [debugWindowController showWindow:debugWindowController];

    // デバッグ情報表示を更新する
    update_debug_info(true);

	return TRUE;
}

//
// ヘルパー
//

// UTF-8文字列をNSStringに変換する
static NSString *nsstr(const char *utf8str)
{
    NSString *s = [[NSString alloc] initWithUTF8String:utf8str];
    return s;
}

// intをNSStringに変換する
static NSString *parseint(int num)
{
    NSString *s = [NSString stringWithFormat:@"%d", num];
    return s;
}

//
// platform.h
//

/*
 *再開ボタンが押されたか調べる
 */
bool is_resume_pushed(void)
{
    bool ret = isResumePressed;
    isResumePressed = false;
    return ret;
}

/*
 * 次へボタンが押されたか調べる
 */
bool is_next_pushed(void)
{
    bool ret = isNextPressed;
    isNextPressed = false;
    return ret;
}

/*
 * 停止ボタンが押されたか調べる
 */
bool is_pause_pushed(void)
{
    bool ret = isPausePressed;
    isPausePressed = false;
    return ret;
}

//
// 実行するスクリプトファイルが変更されたか調べる
//
bool is_script_changed(void)
{
    bool ret = isChangeScriptPressed;
    isChangeScriptPressed = false;
    return ret;
}

//
// 変更された実行するスクリプトファイル名を取得する
//
const char *get_changed_script(void)
{
    static char script[256];
    snprintf(script, sizeof(script), "%s",
             [[debugWindowController getScriptName] UTF8String]);
    return script;
}

/*
 * 実行する行番号が変更されたか調べる
 */
bool is_line_changed(void)
{
    return false;
}

/*
 * 変更された行番号を取得する
 */
int get_changed_line(void)
{
    return [debugWindowController getScriptLine];
}

/*
 * スクリプトがリロードされたかを調べる
 */
bool is_script_reloaded(void)
{
    bool ret = isReloadPressed;
    isReloadPressed = false;
    return ret;
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
    [debugWindowController setScriptName:nsstr(get_script_file_name())];
    [debugWindowController setScriptLine:get_line_num()];
}
