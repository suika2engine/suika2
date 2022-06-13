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

// 英語モードか
static bool isEnglish;

// 実行中か
static bool isRunning;

// ボタンが押下されたか
static bool isResumePressed;
static bool isNextPressed;
static bool isPausePressed;
static bool isChangeScriptPressed;
static bool isLineChangePressed;
static bool isCommandUpdatePressed;
static bool isReloadPressed;

// 前方参照
static NSString *nsstr(const char *utf8str);
static void setWaitingState(void);
static void setRunningState(void);
static void setStoppedState(void);

//
// DebugWindowController
//

@interface DebugWindowController ()
@property (strong) IBOutlet NSWindow *debugWindow;
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
@property (weak) IBOutlet NSButton *buttonUpdateVariables;
@end

@implementation DebugWindowController

//
// NSWindowDelegate
//

// ウィンドウがロードされたときのイベント
- (void)windowDidLoad {
    [super windowDidLoad];
    [[self window] setDelegate:self];
    [[self tableViewScript] setDataSource:self];
    [[self tableViewScript] setDelegate:self];
    [[self tableViewScript] setTarget:self];
    [[self tableViewScript] setDoubleAction:@selector(doubleClickTableView:)];

    // メインスクリーンの位置とサイズを取得する
    NSRect sr = [[NSScreen mainScreen] visibleFrame];

    // ウィンドウの位置とサイズを取得する
    NSRect wr = [[self window] frame];

    // ウィンドウの座標を計算する
    NSRect cr = NSMakeRect(sr.origin.x + sr.size.width - wr.size.width,
                           sr.origin.y + sr.size.height - wr.size.height,
                           wr.size.width,
                           wr.size.height);

    // ウィンドウを移動する
    [[self window] setFrame:cr display:YES animate:NO];
}

// ウィンドウが閉じられるときのイベント
- (BOOL)windowShouldClose:(id)sender {
    // メインループから抜ける
    [NSApp stop:nil];
    return NO;
}

//
// IBAction (Buttons)
//

// 続けるボタンが押下されたイベント
- (IBAction) onResumeButton:(id)sender {
    isResumePressed = true;
}

// 次へボタンが押下されたイベント
- (IBAction) onNextButton:(id)sender {
    isNextPressed = true;
}

// 停止ボタンが押下されたイベント
- (IBAction) onPauseButton:(id)sender {
    isPausePressed = true;
}

// スクリプトファイル名の反映ボタンが押下されたイベント
- (IBAction)onUpdateScriptNameButton:(id)sender {
    isChangeScriptPressed = true;
}

// 行番号の反映ボタンが押下されたイベント
- (IBAction)onUpdateLineNumber:(id)sender {
    isLineChangePressed = true;
}

// コマンドの反映ボタンが押下されたイベント
- (IBAction)onUpdateCommandTextButton:(id)sender {
    isCommandUpdatePressed = true;
}

// エラーを探すボタンが押下されたイベント
- (IBAction)onNextErrorButton:(id)sender {
    // 行数を取得する
	int lines = get_line_count();

    // 選択されている行を取得する
	int start = (int)[[self tableViewScript] selectedRow];

    // 選択されている行より下を検索する
	for (int i = start + 1; i < lines; i++) {
        const char *text = get_line_string_at_line_num(i);
		if(text[0] == '!') {
            // みつかったので選択する
            NSIndexSet *indexSet = [NSIndexSet indexSetWithIndex:i];
            [[self tableViewScript] selectRowIndexes:indexSet
                                byExtendingSelection:NO];
            [[self tableViewScript] scrollRowToVisible:i];
			return;
		}
	}

    // 先頭行から、選択されている行までを検索する
	if (start != 0) {
		for (int i = 0; i <= start; i++) {
			const char *text = get_line_string_at_line_num(i);
			if(text[0] == '!') {
                // みつかったので選択する
                NSIndexSet *indexSet = [NSIndexSet indexSetWithIndex:i];
                [[self tableViewScript] selectRowIndexes:indexSet
                                    byExtendingSelection:NO];
                [[self tableViewScript] scrollRowToVisible:i];
                return;
			}
		}
	}

    log_info(isEnglish ? "No error." : "エラーはありません。");
}

// 上書き保存ボタンが押下されたイベント
- (IBAction)onOverwriteButton:(id)sender {
    // スクリプトファイル名を取得する
	const char *scr = get_script_file_name();
	if(strcmp(scr, "DEBUG") == 0)
		return;

    // 確認のダイアログを開く
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:conf_language == NULL ? @"はい" : @"Yes"];
    [alert addButtonWithTitle:conf_language == NULL ? @"いいえ" : @"No"];
    [alert setMessageText:isEnglish ?
           @"Are you sure you want to overwrite the script file?" :
           @"スクリプトファイルを上書き保存します。\nよろしいですか？"];
    [alert setAlertStyle:NSWarningAlertStyle];
    if([alert runModal] != NSAlertFirstButtonReturn)
        return;

    // ファイルを開く
	char *path = make_valid_path(SCRIPT_DIR, scr);
    if (path == NULL)
        return;
    FILE *fp = fopen(path, "w");
    free(path);
    if (fp == NULL)	{
        log_error(isEnglish ?
                  "Cannot write to file." : "ファイルに書き込めません。");
        return;
    }

    // 書き出す
	for (int i = 0; i < get_line_count(); i++) {
		int body = fputs(get_line_string_at_line_num(i), fp);
		int lf = fputs("\n", fp);
		if (body < 0 || lf < 0) {
            log_error(isEnglish ?
                      "Cannot write to file." : "ファイルに書き込めません。");
            break;
		}
	}
	fclose(fp);
}

// 再読み込みボタンが押下されたイベント
- (IBAction)onReloadButton:(id)sender {
    isReloadPressed = true;
}

// 変数の反映ボタンが押下されたイベント
- (IBAction)onUpdateVariablesButton:(id)sender {
	// テキストフィールドの内容を取得する
	NSString *text = [self getVariablesText];

	// パースする
	const char *p = [text UTF8String];
    const char *next_line;
	while (*p) {
		/* 空行を読み飛ばす */
		if (*p == '\n') {
			p++;
			continue;
		}

		// 次の行の開始文字を探す
		next_line = p;
		while (*next_line) {
			if (*next_line == '\n') {
				next_line++;
				break;
			}
			next_line++;
		}

		// パースする
        int index, val;
		if (sscanf(p, "$%d=%d\n", &index, &val) != 2)
			index = val = -1;
		if (index >= LOCAL_VAR_SIZE + GLOBAL_VAR_SIZE)
			index = -1;

		// 変数を設定する
		if (index != -1)
			set_variable(index, val);

		// 次の行へポインタを進める
		p = next_line;
	}

    // 変更された後の変数でテキストフィールドを更新する
	[self updateVariableTextField];
}

//
// IBAction (Menus)
//

// スクリプトを開くメニューが押下されたイベント
- (IBAction)onMenuScriptOpen:(id)sender {
}

// スクリプトを上書き保存するメニューが押下されたイベント
- (IBAction)onMenuScriptOverwrite:(id)sender {
    [self onOverwriteButton:sender];
}

// 続けるメニューが押下されたイベント
- (IBAction)onMenuResume:(id)sender {
    [self onResumeButton:sender];
}

// 次へメニューが押下されたイベント
- (IBAction)onMenuNext:(id)sender {
    [self onNextButton:sender];
}

// 停止メニューが押下されたイベント
- (IBAction)onMenuPause:(id)sender {
    [self onPauseButton:sender];
}

// 次のエラー箇所へ移動メニューが押下されたイベント
- (IBAction)onMenuNextError:(id)sender {
    [self onNextErrorButton:sender];
}

// 再読み込みが押下されたイベント
- (IBAction)onMenuReload:(id)sender {
    [self onReloadButton:sender];
}

//
// NSTableViewDataSourceおよびNSTableViewのダブルクリックアクション
//

// テーブルビューの行数を返す
- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView {
    return get_line_count();
}

// テーブルビューの行の文字列を返す
- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex {
    return nsstr(get_line_string_at_line_num((int)rowIndex));
}

// ダブルクリックされたときのイベント
- (void)doubleClickTableView:(id)object {
    NSInteger line = [[self tableViewScript] clickedRow];
    [[self textFieldScriptLine] setStringValue:
                                    [NSString stringWithFormat:@"%ld", line]];
    isLineChangePressed = true;
}

//
// ウィンドウ/ビューの設定/取得
//

// ウィンドウのタイトルを設定する
- (void)setTitle:(NSString *)title {
    [[self window] setTitle:title];
}

// 続けるボタンを設定する
- (void)setResumeButton:(BOOL)enabled text:(NSString *)text {
    [[self buttonResume] setEnabled:enabled];
    [[self buttonResume] setTitle:text];
}

// 次へボタンを設定する
- (void)setNextButton:(BOOL)enabled text:(NSString *)text {
    [[self buttonNext] setEnabled:enabled];
    [[self buttonNext] setTitle:text];
}

// 停止ボタンを設定する
- (void)setPauseButton:(BOOL)enabled text:(NSString *)text {
    [[self buttonPause] setEnabled:enabled];
    [[self buttonPause] setTitle:text];
}

// スクリプト名のテキストフィールの有効状態を設定する
- (void)enableScriptTextField:(BOOL)state {
    [[self textFieldScriptName] setEnabled:state];
}

// スクリプト名のテキストフィールドの値を設定する
- (void)setScriptName:(NSString *)name {
    [[self textFieldScriptName] setStringValue:name];
}

// スクリプト名のテキストフィールドの値を取得する
- (NSString *)getScriptName {
    return [[self textFieldScriptName] stringValue];
}

// スクリプト変更ボタンの有効状態を設定する
- (void)enableScriptUpdateButton:(BOOL)state {
    [[self buttonScriptNameUpdate] setEnabled:state];
}

// スクリプト行番号ラベルを設定する
- (void)setLineNumberLabel:(NSString *)text {
    [[self labelScriptLine] setStringValue:text];
}

// スクリプト行番号のテキストフィールドの有効状態を設定する
- (void)enableLineNumberTextField:(BOOL)state {
    [[self textFieldScriptLine] setEnabled:state];
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

// スクリプト行番号変更ボタンの有効状態を設定する
- (void)enableLineNumberUpdateButton:(BOOL)state {
    [[self buttonScriptLine] setEnabled:state];
}

// コマンドのラベルのテキストを設定する
- (void)setCommandLabel:(NSString *)text {
    [[self labelCommand] setStringValue:text];
}

// コマンドのテキストフィールドの有効状態を設定する
- (void)enableCommandTextField:(BOOL)state {
    [[self textFieldCommand] setEditable:state];
}

// コマンドのテキストフィールドの値を設定する
- (void)setCommandText:(NSString *)text {
    [[self textFieldCommand] setString:text];
}

// コマンドのテキストフィールドの値を取得する
- (NSString *)getCommandText {
    return [[[self textFieldCommand] textStorage] string];
}

// コマンド反映ボタンの有効状態を設定する
- (void)enableCommandUpdateButton:(BOOL)state {
    [[self buttonCommandUpdate] setEnabled:state];
}

// スクリプトテーブルビューの有効状態を設定する
- (void)enableScriptTableView:(BOOL)state {
    [[self tableViewScript] setEnabled:state];
}

// エラーを探すボタンの有効状態を設定する
- (void)enableNextErrorButton:(BOOL)state {
    [[self buttonSearchError] setEnabled:state];
}

// 上書きボタンの有効状態を設定する
- (void)enableOverwriteButton:(BOOL)state {
    [[self buttonOverwriteScript] setEnabled:state];
}

// リロードボタンの有効状態を設定する
- (void)enableReloadButton:(BOOL)state {
    [[self buttonReloadScript] setEnabled:state];
}

// 変数のテキストフィールドの有効状態を設定する
- (void)enableVariableTextField:(BOOL)state {
    [[self textFieldVariables] setEditable:state];
}

// 変数の書き込みボタンの有効状態を設定する
- (void)enableVariableUpdateButton:(BOOL)state {
    [[self buttonUpdateVariables] setEnabled:state];
}

// 変数のテキストフィールドの値を設定する
- (void)setVariablesText:(NSString *)text {
    [[self textFieldVariables] setString:text];
}

// 変数のテキストフィールドの値を取得する
- (NSString *)getVariablesText {
    return [[[self textFieldVariables] textStorage] string];
}

///
/// スクリプトのテーブルビュー
///

// スクリプトのテキストビューの内容を更新する
- (void)updateScriptTableView {
    [[self tableViewScript] reloadData];
}

// スクリプトのテーブルビューをスクロールする
- (void)scrollScriptTableView {
    int line = get_line_num();
    NSIndexSet *indexSet = [NSIndexSet indexSetWithIndex:line];
    [[self tableViewScript] selectRowIndexes:indexSet byExtendingSelection:NO];
    [[self tableViewScript] scrollRowToVisible:line];
}

//
// 変数のテキストフィールド
//

- (void)updateVariableTextField {
    @autoreleasepool {
        int index, val;
        NSMutableString *text = [NSMutableString new];

        for (index = 0; index < LOCAL_VAR_SIZE + GLOBAL_VAR_SIZE; index++) {
            // 変数が初期値の場合
            val = get_variable(index);
            if(val == 0 && !is_variable_changed(index))
                continue;

            // 行を追加する
            [text appendString:
                      [NSString stringWithFormat:@"$%d=%d\n", index, val]];
        }

        [[self textFieldVariables] setString:text];
    }
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

    // 英語モードかどうかをロケールから決定する
    NSString *lang = [[NSLocale preferredLanguages] objectAtIndex:0];
    isEnglish = [lang isEqualToString:@"ja-JP"] ? false : true;
    if (isEnglish)
        conf_language = strdup("English");

    // デバッグウィンドウのXibファイルをロードする
    debugWindowController = [[DebugWindowController alloc]
                                  initWithWindowNibName:@"DebugWindow"];
    if (debugWindowController == NULL)
        return FALSE;

    // メニューのXibをロードする
    NSBundle *bundle  = [NSBundle mainBundle];
    NSArray  *objects = [NSArray new];
    [bundle loadNibNamed:@"MainMenu"
                   owner:debugWindowController
         topLevelObjects:&objects];

    // デバッグウィンドウを表示する
    [debugWindowController showWindow:debugWindowController];

    // ビューを更新する
    set_running_state(false, false);

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

//
// platform.h
//

//
// 再開ボタンが押されたか調べる
//
bool is_resume_pushed(void)
{
    bool ret = isResumePressed;
    isResumePressed = false;
    return ret;
}

//
// 次へボタンが押されたか調べる
//
bool is_next_pushed(void)
{
    bool ret = isNextPressed;
    isNextPressed = false;
    return ret;
}

//
// 停止ボタンが押されたか調べる
//
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

//
// 実行する行番号が変更されたか調べる
//
bool is_line_changed(void)
{
    bool ret = isLineChangePressed;
    isLineChangePressed = false;
    return ret;
}

//
// 変更された行番号を取得する
//
int get_changed_line(void)
{
    return [debugWindowController getScriptLine];
}

//
// コマンドがアップデートされたかを調べる
//
bool is_command_updated(void)
{
    bool ret = isCommandUpdatePressed;
    isCommandUpdatePressed = false;
    return ret;
}

//
// アップデートされたコマンド文字列を取得する
//
const char *get_updated_command(void)
{
    @autoreleasepool {
        static char command[4096];
        snprintf(command, sizeof(command), "%s",
                 [[debugWindowController getCommandText] UTF8String]);
        return command;
    }
}

//
// スクリプトがリロードされたかを調べる
//
bool is_script_reloaded(void)
{
    bool ret = isReloadPressed;
    isReloadPressed = false;
    return ret;
}

//
// コマンドの実行中状態を設定する
//
void set_running_state(bool running, bool request_stop)
{
    // 実行状態を保存する
    isRunning = running;

    // 停止によりコマンドの完了を待機中のとき
    if(request_stop) {
        setWaitingState();
        return;
    }

    // 実行中のとき 
    if(running) {
        setRunningState();
        return;
    }

    // 完全に停止中のとき
    setStoppedState();
}

// 停止によりコマンドの完了を待機中のときのビューの状態を設定する
static void setWaitingState(void)
{
    // ウィンドウのタイトルを設定する
    [debugWindowController setTitle:isEnglish ?
                           @"Waiting for command finish..." :
                           @"コマンドの完了を待機中..."];

    // 続けるボタンを無効にする
    [debugWindowController setResumeButton:NO text:isEnglish ?
                           @"Resume" :
                           @"続ける"];

    // 次へボタンを無効にする
    [debugWindowController setNextButton:NO text:isEnglish ?
                           @"Next" :
                           @"次へ"];

    // 停止ボタンを無効にする
    [debugWindowController setPauseButton:NO text:isEnglish ?
                           @"Pause" :
                           @"停止"];

    // スクリプトテキストボックスを無効にする
    [debugWindowController enableScriptTextField:NO];

    // スクリプト変更ボタンを無効にする
    [debugWindowController enableScriptUpdateButton:NO];

    // スクリプト選択ボタンを無効にする
    //[debugWindowController enableScriptOpenButton:NO];

    // 行番号ラベルを設定する
    [debugWindowController setLineNumberLabel:isEnglish ?
                           @"Current Waiting Line:" :
                           @"現在完了待ちの行番号:"];

    // 行番号テキストボックスを無効にする
    [debugWindowController enableLineNumberTextField:NO];

    // 行番号変更ボタンを無効にする
    [debugWindowController enableLineNumberUpdateButton:NO];

    // コマンドラベルを設定する
    [debugWindowController setCommandLabel:isEnglish ?
                           @"Current Waiting Command:" :
                           @"現在完了待ちのコマンド:"];

    // コマンドテキストボックスを無効にする
    [debugWindowController enableCommandTextField:NO];

    // コマンドアップデートボタンを無効にする
    [debugWindowController enableCommandUpdateButton:NO];

    // コマンドリセットボタンを無効にする
    //[debugWindowController enableCommandResetButton:NO];

    // リストボックスを有効にする
    [debugWindowController enableScriptTableView:NO];

    // エラーを探すを無効にする
    [debugWindowController enableNextErrorButton:NO];

    // 上書き保存ボタンを無効にする
    [debugWindowController enableOverwriteButton:NO];

    // 再読み込みボタンを無効にする */
    [debugWindowController enableReloadButton:NO];

    // 変数のテキストボックスを無効にする
    [debugWindowController enableVariableTextField:NO];

    // 変数の書き込みボタンを無効にする
    [debugWindowController enableVariableUpdateButton:NO];

    // TODO: スクリプトを開くメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:1] submenu] itemWithTag:100] setEnabled:NO];

    // TODO: 上書き保存メニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:1] submenu] itemWithTag:101] setEnabled:NO];

    // TODO: 続けるメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:102] setEnabled:NO];

    // TODO: 次へメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:103] setEnabled:NO];

    // TODO: 停止メニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:104] setEnabled:NO];

    // TODO: 次のエラー箇所へメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:105] setEnabled:NO];

    // TODO: 再読み込みメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:106] setEnabled:NO];
}

// 実行中のときのビューの状態を設定する
static void setRunningState(void)
{
    // ウィンドウのタイトルを設定する
    [debugWindowController setTitle:isEnglish ?
                           @"Running..." :
                           @"実行中..."];

    // 続けるボタンを無効にする
    [debugWindowController setResumeButton:NO text:isEnglish ?
                           @"Resume" :
                           @"続ける"];

    /* 次へボタンを無効にする */
    [debugWindowController setNextButton:NO text:isEnglish ?
                           @"Next" :
                           @"次へ"];

    /* 停止ボタンを有効にする */
    [debugWindowController setPauseButton:TRUE text:isEnglish ?
                           @"Pause" :
                           @"停止"];

    // スクリプトテキストボックスを無効にする
    [debugWindowController enableScriptTextField:NO];

    // スクリプト変更ボタンを無効にする
    [debugWindowController enableScriptUpdateButton:NO];

    // スクリプト選択ボタンを無効にする
    //[debugWindowController enableScriptOpenButton:NO];

    // 行番号ラベルを設定する
    [debugWindowController setLineNumberLabel:isEnglish ?
                           @"Current Running Line:" :
                           @"現在実行中の行番号:"];

    // 行番号テキストボックスを無効にする
    [debugWindowController enableLineNumberTextField:NO];

    // 行番号変更ボタンを無効にする
    [debugWindowController enableLineNumberUpdateButton:NO];

    // コマンドラベルを設定する
    [debugWindowController setCommandLabel:isEnglish ?
                           @"Current Running Command:" :
                           @"現在実行中のコマンド:"];

    // コマンドテキストボックスを無効にする
    [debugWindowController enableCommandTextField:NO];

    // コマンドアップデートボタンを無効にする
    [debugWindowController enableCommandUpdateButton:NO];

    // コマンドリセットボタンを無効にする
    //[debugWindowController enableCommandResetButton:NO];

    // リストボックスを有効にする
    [debugWindowController enableScriptTableView:NO];

    // エラーを探すを無効にする
    [debugWindowController enableNextErrorButton:NO];

    // 上書きボタンを無効にする
    [debugWindowController enableOverwriteButton:NO];

    // 再読み込みボタンを無効にする
    [debugWindowController enableReloadButton:NO];

    // 変数のテキストボックスを無効にする
    [debugWindowController enableVariableTextField:NO];

    // 変数の書き込みボタンを無効にする
    [debugWindowController enableVariableUpdateButton:NO];

    // TODO: スクリプトを開くメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:1] submenu] itemWithTag:100] setEnabled:NO];

    // TODO: 上書き保存メニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:1] submenu] itemWithTag:101] setEnabled:NO];

    // TODO: 続けるメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:102] setEnabled:NO];

    // TODO: 次へメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:103] setEnabled:NO];

    // TODO: 停止メニューを有効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:104] setEnabled:YES];

    // TODO: 次のエラー箇所へメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:105] setEnabled:NO];

    // TODO: 再読み込みメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:106] setEnabled:NO];
}

// 完全に停止中のときのビューの状態を設定する
static void setStoppedState(void)
{
    // ウィンドウのタイトルを設定する
    [debugWindowController setTitle:isEnglish ?
                           @"Stopped" :
                           @"停止中"];

    // 続けるボタンを有効にする
    [debugWindowController setResumeButton:YES text:isEnglish ?
                           @"Resume" :
                           @"続ける"];

    // 次へボタンを有効にする
    [debugWindowController setNextButton:YES text:isEnglish ?
                           @"Next" :
                           @"次へ"];

    // 停止ボタンを無効にする
    [debugWindowController setPauseButton:NO text:isEnglish ?
                           @"Pause" :
                           @"停止"];

    // スクリプトテキストボックスを有効にする
    [debugWindowController enableScriptTextField:YES];

    // スクリプト変更ボタンを有効にする
    [debugWindowController enableScriptUpdateButton:YES];

    // スクリプト選択ボタンを有効にする
    //[debugWindowController enableScriptOpenButton:YES];

    // 行番号ラベルを設定する
    [debugWindowController setLineNumberLabel:isEnglish ?
                           @"Next Line to be Executed:" :
                           @"次に実行される行番号:"];

    // 行番号テキストボックスを有効にする
    [debugWindowController enableLineNumberTextField:YES];

    // 行番号変更ボタンを有効にする
    [debugWindowController enableLineNumberUpdateButton:YES];

    // コマンドラベルを設定する
    [debugWindowController setCommandLabel:isEnglish ?
                           @"Next Command to be Executed:" :
                           @"次に実行されるコマンド:"];

    // コマンドテキストボックスを有効にする
    [debugWindowController enableCommandTextField:YES];

    // コマンドアップデートボタンを有効にする
    [debugWindowController enableCommandUpdateButton:YES];

    // コマンドリセットボタンを有効にする
    //[debugWindowController enableCommandResetButton:YES];

    // リストボックスを有効にする
    [debugWindowController enableScriptTableView:YES];

    // エラーを探すを有効にする
    [debugWindowController enableNextErrorButton:YES];

    // 上書き保存ボタンを有効にする
    [debugWindowController enableOverwriteButton:YES];

    // 再読み込みボタンを有効にする
    [debugWindowController enableReloadButton:YES];

    // 変数のテキストボックスを有効にする
    [debugWindowController enableVariableTextField:YES];

    // 変数の書き込みボタンを有効にする
    [debugWindowController enableVariableUpdateButton:YES];

    // TODO: スクリプトを開くメニューを有効にする
    [[[[[NSApp mainMenu] itemAtIndex:1] submenu] itemWithTag:100] setEnabled:YES];

    // TODO: 上書き保存メニューを有効にする
    [[[[[NSApp mainMenu] itemAtIndex:1] submenu] itemWithTag:101] setEnabled:YES];

    // TODO: 続けるメニューを有効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:102] setEnabled:YES];

    // TODO: 次へメニューを有効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:103] setEnabled:YES];

    // TODO: 停止メニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:104] setEnabled:NO];

    // TODO: 次のエラー箇所へメニューを有効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:105] setEnabled:YES];

    // TODO: 再読み込みメニューを有効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:106] setEnabled:YES];
}

//
// デバッグ情報を更新する
//
void update_debug_info(bool script_changed)
{
    [debugWindowController setScriptName:nsstr(get_script_file_name())];
    [debugWindowController setScriptLine:get_line_num()];
    [debugWindowController setCommandText:nsstr(get_line_string())];

    if (script_changed)
        [debugWindowController updateScriptTableView];

    [debugWindowController scrollScriptTableView];
    
	if (check_variable_updated() || script_changed)
		[debugWindowController updateVariableTextField];
}
