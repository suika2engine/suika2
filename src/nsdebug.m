// -*- tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changed]
 *  - 2020/06/11 Created.
 *  - 2023/12/05 Changed to editor.
 */

#import <stdio.h>
#import <stdlib.h>
#import <string.h>

#import "suika.h"
#import "nsdebug.h"
#import "package.h"

#import "nsmain.h"

#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

// デバッグウィンドウのコントローラ
static DebugWindowController *debugWindowController;

// 英語モードか
static bool isEnglish;

// 実行中か
static bool isRunning;

// ボタンが押下されたか
static bool isContinuePressed;
static bool isNextPressed;
static bool isStopPressed;
static bool isOpenScriptPressed;
static bool isExecLineChanged;

// 変更された実行行
static int changedExecLine;

static BOOL isFirstChange;
static BOOL isRangedChange;

// 前方参照
static NSString *nsstr(const char *utf8str);
static void setWaitingState(void);
static void setRunningState(void);
static void setStoppedState(void);

//
// main()
//

int main(int argc, char * argv[]) {
    NSString *appDelegateClassName;
    @autoreleasepool {
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }
    return NSApplicationMain(argc, argv, nil, appDelegateClassName);
}

//
// DebugAppDelegate
//

@implementation DebugAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
    debugWindowController = [[DebugWindowController alloc] init];
    [debugWindowController showWindow:self];
}

- (void)dealloc
{
    [debugWindowController release];
    [super dealloc];
}

@end

//
// DebugWindowController
//

@interface DebugWindowController ()
@property (strong) IBOutlet NSWindow *window;
@property (strong) IBOutlet SuikaView *view;
@property (weak) IBOutlet NSButton *buttonContinue;
@property (weak) IBOutlet NSButton *buttonNext;
@property (weak) IBOutlet NSButton *buttonStop;
@property (weak) IBOutlet NSTextField *textViewScriptName;
@property (weak) IBOutlet NSButton *buttonOpenScript;
@property (unsafe_unretained) IBOutlet NSTextView *textViewScript;
@property (unsafe_unretained) IBOutlet NSTextView *textViewVariables;
@property (weak) IBOutlet NSButton *buttonUpdateVariables;
@end

@implementation DebugWindowController

//
// NSWindowDelegate
//
#if defined(USE_DEBUGGER)
    // ビューを更新する
    on_change_running_state(false, false);

    // デバッグ情報表示を更新する
    on_load_script();
    on_change_position();
#endif

- (id)init {
    self = [super initWithWindowNibName:@"DebugWindow"];
    return self;
}

// ウィンドウがロードされたときのイベント
- (void)windowDidLoad {
    [super windowDidLoad];
    [[self window] setDelegate:self];

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
- (IBAction) onContinueButton:(id)sender {
    isContinuePressed = true;
}

// 次へボタンが押下されたイベント
- (IBAction) onNextButton:(id)sender {
    isNextPressed = true;
}

// 停止ボタンが押下されたイベント
- (IBAction) onStopButton:(id)sender {
    isStopPressed = true;
}

// スクリプトファイル名の反映ボタンが押下されたイベント
- (IBAction)onOpenScriptButton:(id)sender {
    // TODO: Open a script
    isOpenScriptPressed = true;
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

// ゲームフォルダを開くメニューが押下されたイベント
- (IBAction)onMenuOpenGameFolder:(id)sender {
}

// スクリプトを開くメニューが押下されたイベント
- (IBAction)onMenuOpenScript:(id)sender {
    // .appバンドルのパスを取得する
    NSString *bundlePath = [[NSBundle mainBundle] bundlePath];

    // .appバンドルの1つ上のディレクトリのパスを取得する
    NSString *basePath = [bundlePath stringByDeletingLastPathComponent];

    // txtディレクトリのパスを取得する
    NSString *txtPath = [NSString stringWithFormat:@"%@/%@", basePath, @"txt"];

    // 開くダイアログを作る
    NSOpenPanel *panel= [NSOpenPanel openPanel];
    [panel setAllowedFileTypes:
               [NSArray arrayWithObjects:@"txt", @"'TEXT'", nil]];
    [panel setDirectoryURL:[[NSURL alloc] initFileURLWithPath:txtPath]];
    if ([panel runModal] == NSModalResponseOK) {
        NSString *file = [[panel URL] lastPathComponent];
        if ([file hasPrefix:txtPath]) {
                [self setScriptName:file];
                isOpenScriptPressed = true;
        }
    }
}

// 保存ボタンが押下されたイベント
- (IBAction)onMenuSave:(id)sender {
    // スクリプトファイル名を取得する
    const char *scr = get_script_file_name();
    if(strcmp(scr, "DEBUG") == 0)
        return;

    // 確認のダイアログを開く
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:!isEnglish ? @"はい" : @"Yes"];
    [alert addButtonWithTitle:!isEnglish ? @"いいえ" : @"No"];
    [alert setMessageText:isEnglish ?
           @"Are you sure you want to overwrite the script file?" :
           @"スクリプトファイルを上書き保存します。\nよろしいですか？"];
    [alert setAlertStyle:NSAlertStyleWarning];
    if([alert runModal] != NSAlertFirstButtonReturn)
        return;

    // ファイルを開く
    char *path = make_valid_path(SCRIPT_DIR, scr);
    if (path == NULL)
        return;
    FILE *fp = fopen(path, "w");
    free(path);
    if (fp == NULL)    {
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

// 続けるメニューが押下されたイベント
- (IBAction)onMenuContinue:(id)sender {
    [self onContinueButton:sender];
}

// 次へメニューが押下されたイベント
- (IBAction)onMenuNext:(id)sender {
    [self onNextButton:sender];
}

// 停止メニューが押下されたイベント
- (IBAction)onMenuStop:(id)sender {
    [self onStopButton:sender];
}

// 次のエラー箇所へ移動メニューが押下されたイベント
- (IBAction)onMenuNextError:(id)sender {
    // 行数を取得する
    int lines = get_line_count();

    // カーソル行を取得する
    NSRange sel = [self.textViewScript selectedRange];
    NSString *viewContent = [self.textViewScript string];
    NSRange lineRange = [viewContent lineRangeForRange:NSMakeRange(sel.location, 0)];
    int start = (int)lineRange.location;

    // カーソル行より下を検索する
    for (int i = start + 1; i < lines; i++) {
        const char *text = get_line_string_at_line_num(i);
        if(text[0] == '!') {
            // みつかったので選択する
            [self selectScriptLine:i];
            //[self.textViewScript scrollRowToVisible:i];
            return;
        }
    }

    // 先頭行から、選択されている行までを検索する
    if (start != 0) {
        for (int i = 0; i <= start; i++) {
            const char *text = get_line_string_at_line_num(i);
            if(text[0] == '!') {
                // みつかったので選択する
                [self selectScriptLine:i];
                //[self.textViewScript scrollRowToVisible:i];
                return;
            }
        }
    }

    log_info(isEnglish ? "No error." : "エラーはありません。");
}

// from: https://stackoverflow.com/questions/54238610/nstextview-select-specific-line
- (void)selectScriptLine:(NSUInteger)line {
    NSLayoutManager *layoutManager = [self.textViewScript layoutManager];
    NSUInteger numberOfLines = 0;
    NSUInteger numberOfGlyphs = [layoutManager numberOfGlyphs];
    NSRange lineRange;
    for (NSUInteger indexOfGlyph = 0; indexOfGlyph < numberOfGlyphs; numberOfLines++) {
        [layoutManager lineFragmentRectForGlyphAtIndex:indexOfGlyph effectiveRange:&lineRange];
        if (numberOfLines == line) {
            [[NSOperationQueue mainQueue] addOperationWithBlock:^{
                [self.textViewScript setSelectedRange:lineRange];
            }];
            break;
        }
        indexOfGlyph = NSMaxRange(lineRange);
    }
}

// メッセージの挿入
- (IBAction)onMenuMessage:(id)sender {
}

// セリフの挿入
- (IBAction)onMenuLine:(id)sender {
}

// セリフ(ボイスつき)の挿入
- (IBAction)onMenuLineWithVoice:(id)sender {
}

// 背景変更の挿入
- (IBAction)onMenuBackground:(id)sender {
}

// 背景のみ変更の挿入
- (IBAction)onMenuBackgroundOnly:(id)sender {
}

// 左キャラの表示
- (IBAction)onMenuShowLeftCharacter:(id)sender {
}

// 左キャラの非表示
- (IBAction)onMenuHideLeftCharacter:(id)sender {
}

// 左中央キャラの表示
- (IBAction)onMenuShowLeftCenterCharacter:(id)sender {
}

// 左中央キャラの非表示
- (IBAction)onMenuHideLeftCenterCharacter:(id)sender {
}

// 中央キャラの表示
- (IBAction)onMenuShowCenterCharacter:(id)sender {
}

// 中央キャラの非表示
- (IBAction)onMenuHideCenterCharacter:(id)sender {
}

// 右中央キャラの表示
- (IBAction)onMenuShoRightwCenterCharacter:(id)sender {
}

// 右中央キャラの非表示
- (IBAction)onMenuHideRightCenterCharacter:(id)sender {
}

// 右キャラの表示
- (IBAction)onMenuShowRightCharacter:(id)sender {
}

// 右キャラの非表示
- (IBAction)onMenuHideRightCharacter:(id)sender {
}

// 複数キャラの変更
- (IBAction)onMenuChsx:(id)sender {
}

// BGM再生
- (IBAction)onMenuBgmPlay:(id)sender {
}

// BGM停止
- (IBAction)onMenuBgmStop:(id)sender {
}

// BGMボリューム
- (IBAction)onMenuBgmVolume:(id)sender {
}

// SE再生
- (IBAction)onMenuSePlay:(id)sender {
}

// SE停止
- (IBAction)onMenuSeStop:(id)sender {
}

// SEボリューム
- (IBAction)onMenuSeVolume:(id)sender {
}

// ボイス再生
- (IBAction)onMenuVoicePlay:(id)sender {
}

// ボイス停止
- (IBAction)onMenuVoiceStop:(id)sender {
}

// ボイスボリューム
- (IBAction)onMenuVoiceVolume:(id)sender {
}

// ビデオ
- (IBAction)onMenuVideo:(id)sender {
}

// 選択肢1
- (IBAction)onMenuChoose1:(id)sender {
}

// 選択肢2
- (IBAction)onMenuChoose2:(id)sender {
}

// 選択肢3
- (IBAction)onMenuChoose3:(id)sender {
}

// クリック
- (IBAction)onMenuClick:(id)sender {
}

// 時間待ち
- (IBAction)onMenuWait:(id)sender {
}

// GUI
- (IBAction)onMenuGUI:(id)sender {
}

// WMS
- (IBAction)onMenuWMS:(id)sender {
}

// Load
- (IBAction)onMenuLoad:(id)sender {
}

// パッケージのエクスポートが押下されたイベント
- (IBAction)onMenuExportForMac:(id)sender {
}

// パッケージのエクスポートが押下されたイベント
- (IBAction)onMenuExportForWindows:(id)sender {
}

// パッケージのエクスポートが押下されたイベント
- (IBAction)onMenuExportForMacAndWindows:(id)sender {
}

// パッケージのエクスポートが押下されたイベント
- (IBAction)onMenuExportForWeb:(id)sender {
}

// パッケージのエクスポートが押下されたイベント
- (IBAction)onMenuExportForIOS:(id)sender {
}

// パッケージのエクスポートが押下されたイベント
- (IBAction)onMenuExportForAndroid:(id)sender {
}

// パッケージのエクスポートが押下されたイベント
- (IBAction)onMenuExportPackage:(id)sender {
    // .appバンドルのパスを取得する
    NSString *bundlePath = [[NSBundle mainBundle] bundlePath];

    // .appバンドルの1つ上のディレクトリのパスを取得する
    NSString *basePath = [bundlePath stringByDeletingLastPathComponent];

    // パッケージを作成する
    if (create_package([basePath UTF8String])) {
        log_info(isEnglish ?
                 "Successfully exported data01.arc" :
                 "data01.arcのエクスポートに成功しました。");
    }
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
    [[self buttonContinue] setEnabled:enabled];
    [[self buttonContinue] setTitle:text];
}

// 次へボタンを設定する
- (void)setNextButton:(BOOL)enabled text:(NSString *)text {
    [[self buttonNext] setEnabled:enabled];
    [[self buttonNext] setTitle:text];
}

// 停止ボタンを設定する
- (void)setStopButton:(BOOL)enabled text:(NSString *)text {
    [[self buttonStop] setEnabled:enabled];
    [[self buttonStop] setTitle:text];
}

// スクリプト名のテキストフィールドの値を設定する
- (void)setScriptName:(NSString *)name {
    [[self textViewScriptName] setStringValue:name];
}

// スクリプト名のテキストフィールドの値を取得する
- (NSString *)getScriptName {
    return [[self textViewScriptName] stringValue];
}

// スクリプトを開くボタンの有効状態を設定する
- (void)enableOpenScriptButton:(BOOL)state {
    [[self buttonOpenScript] setEnabled:state];
}

// スクリプトテキストエディットの有効状態を設定する
- (void)enableScriptTextView:(BOOL)state {
    [[self textViewScript] setEditable:state];
}

// 変数のテキストフィールドの有効状態を設定する
- (void)enableVariableTextView:(BOOL)state {
    [[self textViewVariables] setEditable:state];
}

// 変数の書き込みボタンの有効状態を設定する
- (void)enableVariableUpdateButton:(BOOL)state {
    [[self buttonUpdateVariables] setEnabled:state];
}

// 変数のテキストフィールドの値を設定する
- (void)setVariablesText:(NSString *)text {
    [[self textViewVariables] setString:text];
}

// 変数のテキストフィールドの値を取得する
- (NSString *)getVariablesText {
    return [[self.textViewVariables textStorage] string];
}

///
/// スクリプトのテキストビュー
///

// 実行行を設定する
- (void)setExecLine:(int)line {
    // TODO
}

// スクリプトのテキストビューの内容を更新する
- (void)updateScriptTextView {
    // TODO
}

// スクリプトのテーブルビューをスクロールする
- (void)scrollScriptTextView {
    //int line = get_expanded_line_num();
    // TODO
}

// テキストビューの内容をスクリプトモデルを元に設定する
- (void)updateTextFromScriptModel {
	// 行を連列してスクリプト文字列を作成する
    NSString *text = @"";
	for (int i = 0; i < get_line_count(); i++)
        [text stringByAppendingString:[[NSString alloc] initWithUTF8String:get_line_string_at_line_num(i)]];

	// テキストビューにテキストを設定する
    isFirstChange = TRUE;
	[self.textViewScript setString:text];

	// 複数行の変更があったことを記録する
	isRangedChange = TRUE;
}

// テキストビューの内容を元にスクリプトモデルを更新する
- (void)updateScriptModelFromText {
	// パースエラーをリセットして、最初のパースエラーで通知を行う
	dbg_reset_parse_error_count();

	// リッチエディットのテキストの内容でスクリプトの各行をアップデートする
    const char *ctext= [self.textViewScript.string UTF8String];
    char *text = strdup(ctext);
	int total = (int)strlen(text);
	int lineNum = 0;
	int lineStart = 0;
	while (lineStart < total) {
		// 行を切り出す
		char *lineText = text + lineStart;
		char *lf = strstr(lineText, "\n");
		int lineLen = lf != NULL ? (int)(lf - lineText) : (int)strlen(lineText);
		if (lf != NULL)
			*lf = '\0';

		// 行を更新する
		if (lineNum < get_line_count())
			update_script_line(lineNum, lineText);
		else
			insert_script_line(lineNum, lineText);

		lineNum++;
		lineStart += lineLen;
	}
	free(text);

	// 削除された末尾の行を処理する
	isExecLineChanged = FALSE;
	for (int i = get_line_count() - 1; i >= lineNum; i--)
		if (delete_script_line(i))
			isExecLineChanged = TRUE;
	if (isExecLineChanged)
		[self setTextColorForAllLines];

	// コマンドのパースに失敗した場合
	if (dbg_get_parse_error_count() > 0) {
		// 行頭の'!'を反映するためにテキストを再設定する
		[self updateTextFromScriptModel];
        [self setTextColorForAllLines];
	}
}

// テキストビューの現在の行の内容を元にスクリプトモデルを更新する
- (void)updateScriptModelFromCurrentLineText {
    // TODO
    [self updateScriptModelFromText];
}

- (void)setTextColorForAllLines {
    // すべてのテキスト装飾を削除する
    NSString *text = self.textViewScript.string;
    NSRange allRange = NSMakeRange(0, [text length]);
    [self.textViewScript.textStorage removeAttribute:NSForegroundColorAttributeName range:allRange];
    [self.textViewScript.textStorage removeAttribute:NSBackgroundColorAttributeName range:allRange];

    // 行ごとに処理する
    NSArray *lineArray = [text componentsSeparatedByString:@"\n"];
    int startPos = 0;
    int execLineNum = get_expanded_line_num();
    for (int i = 0; i < execLineNum; i++) {
        NSString *lineText = lineArray[i];
        NSUInteger lineLen = [lineText length];

        // 実行行であれば背景色を設定する
        NSRange lineRange = NSMakeRange(startPos, [lineArray[i] length]);
        NSColor *bgColor = isRunning ? [NSColor redColor] : [NSColor blueColor];
        [self.textViewScript.textStorage addAttribute:NSBackgroundColorAttributeName value:bgColor range:lineRange];

        // コメントを処理する
        if ([lineText characterAtIndex:0] == L'#') {
            // 行全体のテキスト色を変更する
            [self.textViewScript.textStorage addAttribute:NSBackgroundColorAttributeName value:[NSColor grayColor] range:lineRange];
        }
        // ラベルを処理する
        else if ([lineText characterAtIndex:0] == L':') {
            // 行全体のテキスト色を変更する
            [self.textViewScript.textStorage addAttribute:NSBackgroundColorAttributeName value:[NSColor greenColor] range:lineRange];
        }
        // エラー行を処理する
        else if ([lineText characterAtIndex:0] == L'!') {
            // 行全体のテキスト色を変更する
            [self.textViewScript.textStorage addAttribute:NSBackgroundColorAttributeName value:[NSColor redColor] range:lineRange];
        }
        // コマンド行を処理する
        else if ([lineText characterAtIndex:0] == L'@') {
            // コマンド名部分を抽出する
            NSUInteger commandNameLen = [lineText rangeOfString:@" "].location;
            if (commandNameLen == NSNotFound)
                commandNameLen = [lineText length];

            // コマンド名のテキストに色を付ける
            NSRange commandRange = NSMakeRange(startPos, commandNameLen);
            [self.textViewScript.textStorage addAttribute:NSBackgroundColorAttributeName value:[NSColor blueColor] range:commandRange];

            // 引数に色を付ける
            int commandType = get_command_type_from_name([[lineText substringToIndex:commandNameLen] UTF8String]);
            if (commandType != COMMAND_SET && commandType != COMMAND_IF &&
                commandType != COMMAND_UNLESS && commandType != COMMAND_PENCIL &&
                [lineText length] != commandNameLen) {
                // 引数名を灰色にする
                NSUInteger paramStart = startPos + commandNameLen;
                do {
                    NSString *sub = [lineText substringFromIndex:commandNameLen + 1];
                    if ([sub length] == 0)
                        break;

                    // '='を探す
                    NSUInteger eqPos = [sub rangeOfString:@"="].location;
                    if (eqPos == NSNotFound)
                        break;

                    // '='の手前に' 'があればスキップする
                    NSUInteger spacePos = [sub rangeOfString:@" "].location;
                    if (spacePos != NSNotFound && spacePos < eqPos)
                        continue;

                    // 引数名部分のテキスト色を変更する
                    NSRange paramNameRange = NSMakeRange(paramStart, eqPos - paramStart);
                    [self.textViewScript.textStorage addAttribute:NSBackgroundColorAttributeName value:[NSColor grayColor] range:paramNameRange];

                    paramStart += spacePos;
                } while (paramStart < lineLen);
            }
        }
        
        startPos += [lineText length] + 1;
    }
}

//
// 変数のテキストフィールド
//

// 変数のテキストフィールドの内容を更新する
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

        [[self textViewVariables] setString:text];
    }
}

@end

//
// nsmain.cへ提供する機能
//

//
// プロジェクトを初期化する
//
BOOL initProject(void)
{
    // 英語モードかどうかをロケールから決定する
    NSString *lang = [[NSLocale preferredLanguages] objectAtIndex:0];
    isEnglish = [lang hasPrefix:@"ja-"] ? false : true;
    
    // 開くダイアログを作る
    NSOpenPanel *panel= [NSOpenPanel openPanel];
    [panel setAllowedContentTypes:@[[UTType typeWithFilenameExtension:@"suika2project" conformingToType:UTTypeData]]];
    if ([panel runModal] != NSModalResponseOK)
        return FALSE;

    NSString *dir = [[[panel URL] URLByDeletingLastPathComponent] path];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    [fileManager changeCurrentDirectoryPath:dir];

    return TRUE;
}

//
// デバッガを初期化する
//
BOOL initDebugger(void)
{
    assert(debugWindowController == NULL);
    
    // ウィンドウのNibファイルをロードする
    debugWindowController = [[DebugWindowController alloc] initWithWindowNibName:@"DebugWindow"];
    if (debugWindowController == NULL)
        return YES;
    
    // メニューのNibをロードする
    NSBundle *bundle = [NSBundle mainBundle];
    NSArray *objects = [NSArray new];
    [bundle loadNibNamed:@"MainMenu"
                   owner:debugWindowController
         topLevelObjects:&objects];

    // Set delegate.
    [NSApp setDelegate:debugWindowController.view];
    [debugWindowController.window setDelegate:debugWindowController.view];
    [debugWindowController.window makeFirstResponder:debugWindowController.view];

    // ウィンドウを表示する
    [debugWindowController showWindow:debugWindowController];

    /*
    // タイマをセットする
    [NSTimer scheduledTimerWithTimeInterval:1.0/30.0
                                     target:theView
                                   selector:@selector(timerFired:)
                                   userInfo:nil
                                    repeats:YES];
*/
    return YES;
}

//
// デバッガの終了処理を行う
//
void cleanupDebugger(void)
{
    // TODO: destroy something.
}

//
// ヘルパー
//

// UTF-8文字列をNSStringに変換する
static NSString *nsstr(const char *utf8str)
{
    assert(utf8str != NULL);
    NSString *s = [[NSString alloc] initWithUTF8String:utf8str];
    return s;
}

//
// platform.h
//

//
// 再開ボタンが押されたか調べる
//
bool is_continue_pushed(void)
{
    bool ret = isContinuePressed;
    isContinuePressed = false;
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
bool is_stop_pushed(void)
{
    bool ret = isStopPressed;
    isStopPressed = false;
    return ret;
}

//
// 実行するスクリプトファイルが変更されたか調べる
//
bool is_script_opened(void)
{
    bool ret = isOpenScriptPressed;
    isOpenScriptPressed = false;
    return ret;
}

//
// 変更された実行するスクリプトファイル名を取得する
//
const char *get_opened_script(void)
{
    static char script[256];
    snprintf(script, sizeof(script), "%s", [[debugWindowController getScriptName] UTF8String]);
    return script;
}

//
// 実行する行番号が変更されたか調べる
//
bool is_exec_line_changed(void)
{
    bool ret = isExecLineChanged;
    isExecLineChanged = false;
    return ret;
}

//
// 変更された行番号を取得する
//
int get_changed_exec_line(void)
{
    return changedExecLine;
}

//
// コマンドの実行中状態を設定する
//
void on_change_running_state(bool running, bool request_stop)
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
    // 続けるボタンを無効にする
    [debugWindowController setResumeButton:NO text:isEnglish ? @"Continue" : @"続ける"];

    // 次へボタンを無効にする
    [debugWindowController setNextButton:NO text:isEnglish ? @"Next" : @"次へ"];

    // 停止ボタンを無効にする
    [debugWindowController setStopButton:NO text:isEnglish ? @"Stop" : @"停止"];

    // スクリプト選択ボタンを無効にする
    [debugWindowController enableOpenScriptButton:NO];

    // スクリプトのテキストボックスを有効にする
    [debugWindowController enableScriptTextView:NO];

    // 変数のテキストボックスを無効にする
    [debugWindowController enableVariableTextView:NO];

    // 変数の書き込みボタンを無効にする
    [debugWindowController enableVariableUpdateButton:NO];

    // スクリプトを開くメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:1] submenu] itemWithTag:100] setEnabled:NO];

    // 保存メニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:1] submenu] itemWithTag:101] setEnabled:NO];

    // 続けるメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:102] setEnabled:NO];

    // 次へメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:103] setEnabled:NO];

    // 停止メニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:104] setEnabled:NO];

    // 次のエラー箇所へメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:105] setEnabled:NO];

    // 演出メニューを無効にする
    for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:3] submenu] itemArray])
        [item setEnabled:NO];

    // エクスポートメニューを無効にする
    for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:4] submenu] itemArray])
        [item setEnabled:NO];
}

// 実行中のときのビューの状態を設定する
static void setRunningState(void)
{
    // 続けるボタンを無効にする
    [debugWindowController setResumeButton:NO text:isEnglish ? @"Continue" : @"続ける"];

    /* 次へボタンを無効にする */
    [debugWindowController setNextButton:NO text:isEnglish ? @"Next" : @"次へ"];

    /* 停止ボタンを有効にする */
    [debugWindowController setStopButton:TRUE text:isEnglish ? @"Stop" : @"停止"];

    // スクリプトを開くボタンを無効にする
    [debugWindowController enableOpenScriptButton:NO];

    // スクリプトのテキストボックスを有効にする
    [debugWindowController.textViewScript setEditable:YES];

    // 変数のテキストボックスを無効にする
    [debugWindowController enableVariableTextView:NO];

    // 変数の書き込みボタンを無効にする
    [debugWindowController enableVariableUpdateButton:NO];

    // スクリプトを開くメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:1] submenu] itemWithTag:100] setEnabled:NO];

    // 上書き保存メニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:1] submenu] itemWithTag:101] setEnabled:NO];

    // パッケージエクスポートメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:1] submenu] itemWithTag:107] setEnabled:NO];

    // 続けるメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:102] setEnabled:NO];

    // 次へメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:103] setEnabled:NO];

    // 停止メニューを有効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:104] setEnabled:YES];

    // 次のエラー箇所へメニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:105] setEnabled:NO];

    // 演出メニューを無効にする
    for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:3] submenu] itemArray])
        [item setEnabled:NO];

    // エクスポートメニューを無効にする
    for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:4] submenu] itemArray])
        [item setEnabled:NO];
}

// 完全に停止中のときのビューの状態を設定する
static void setStoppedState(void)
{
    // 続けるボタンを有効にする
    [debugWindowController setResumeButton:YES text:isEnglish ? @"Resume" : @"続ける"];

    // 次へボタンを有効にする
    [debugWindowController setNextButton:YES text:isEnglish ? @"Next" : @"次へ"];

    // 停止ボタンを無効にする
    [debugWindowController setStopButton:NO text:isEnglish ? @"Pause" : @"停止"];

    // スクリプト選択ボタンを有効にする
    [debugWindowController enableOpenScriptButton:YES];

    // スクリプトのテキストボックスを有効にする
    [debugWindowController enableScriptTextView:YES];

    // 変数のテキストボックスを有効にする
    [debugWindowController enableVariableTextView:YES];

    // 変数の書き込みボタンを有効にする
    [debugWindowController enableVariableUpdateButton:YES];

    // スクリプトを開くメニューを有効にする
    [[[[[NSApp mainMenu] itemAtIndex:1] submenu] itemWithTag:100] setEnabled:YES];

    // 上書き保存メニューを有効にする
    [[[[[NSApp mainMenu] itemAtIndex:1] submenu] itemWithTag:101] setEnabled:YES];

    // パッケージエクスポートメニューを有効にする
    [[[[[NSApp mainMenu] itemAtIndex:1] submenu] itemWithTag:107] setEnabled:YES];

    // 続けるメニューを有効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:102] setEnabled:YES];

    // 次へメニューを有効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:103] setEnabled:YES];

    // 停止メニューを無効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:104] setEnabled:NO];

    // 次のエラー箇所へメニューを有効にする
    [[[[[NSApp mainMenu] itemAtIndex:2] submenu] itemWithTag:105] setEnabled:YES];

    // 演出メニューを有効にする
    for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:3] submenu] itemArray])
        [item setEnabled:YES];

    // エクスポートメニューを有効にする
    for (NSMenuItem *item in [[[[NSApp mainMenu] itemAtIndex:4] submenu] itemArray])
        [item setEnabled:YES];
}

//
// デバッグ情報を更新する
//
void on_load_script(void)
{
    [debugWindowController setScriptName:nsstr(get_script_file_name())];
    [debugWindowController updateScriptTextView];
}

//
// デバッグ情報を更新する
//
void on_change_position(void)
{
    [debugWindowController setExecLine:get_expanded_line_num()];
}

//
// デバッグ情報を更新する
//
void on_update_variable(void)
{
    [debugWindowController updateVariableTextField];
}
