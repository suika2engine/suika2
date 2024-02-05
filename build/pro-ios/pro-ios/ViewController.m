/* -*- coding: utf-8; indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

#import "ViewController.h"
#import "GameView.h"
#import "GameRenderer.h"

// Suika2 Base
#import "suika.h"

// Suika2 HAL
#import "aunit.h"

static ViewController *theViewController;

static void setWaitingState(void);
static void setRunningState(void);
static void setStoppedState(void);

@interface ViewController ()

// iCloud Drive Path
@property NSString *iCloudDrivePath;
// Status
@property BOOL isEnglish;
@property BOOL isRunning;
@property BOOL isContinuePressed;
@property BOOL isNextPressed;
@property BOOL isStopPressed;
@property BOOL isOpenScriptPressed;
@property BOOL isExecLineChanged;
@property int changedExecLine;

// View
@property (strong) IBOutlet GameView *renderView;

// IBOutlet
@property (weak) IBOutlet UIButton *buttonContinue;
@property (weak) IBOutlet UIButton *buttonNext;
@property (weak) IBOutlet UIButton *buttonStop;
@property (weak) IBOutlet UIButton *buttonUpdate;
@property (weak) IBOutlet UITextField *textFieldScriptName;
@property (weak) IBOutlet UIButton *buttonOpenScript;
@property (unsafe_unretained) IBOutlet UITextView *textViewScript;
@property (unsafe_unretained) IBOutlet UITextField *textFieldVariables;
@property (weak) IBOutlet UIButton *buttonUpdateVariables;

@end

@implementation ViewController
{
    // The GameView for AppKit
    GameView *_view;
    
    // The GameRender (common for AppKit and UIKit)
    GameRenderer *_renderer;
    
    // The screen information
    float _screenScale;
    CGSize _screenSize;
    CGPoint _screenOffset;
    
    // The video player objects and status.
    AVPlayer *_avPlayer;
    AVPlayerLayer *_avPlayerLayer;
    BOOL _isVideoPlaying;
    
    // Edit
    BOOL _isFirstChange;
    BOOL _isRangedChange;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    theViewController = self;

    // Initialize the project.
    if (![self initProject])
        exit(1);
    
    // Initialize the Suika2 engine.
    init_locale_code();
    if(!init_file())
        exit(1);
    if(!init_conf())
        exit(1);
    if(!init_aunit())
        exit(1);
    if(!on_event_init())
        exit(1);
    
    // Create an MTKView.
    _view = (GameView *)self.renderView;
    _view.enableSetNeedsDisplay = YES;
    _view.device = MTLCreateSystemDefaultDevice();
    _view.clearColor = MTLClearColorMake(0.0, 0, 0, 1.0);
    _renderer = [[GameRenderer alloc] initWithMetalKitView:_view andController:self];
    if(!_renderer) {
        NSLog(@"Renderer initialization failed");
        return;
    }
    [_renderer mtkView:_view drawableSizeWillChange:_view.drawableSize];
    _view.delegate = _renderer;
    [self updateViewport:_view.frame.size];

    // Set multi-touch.
    self.view.multipleTouchEnabled = YES;

    // Set the UITextViewDelegate.
    [self.textViewScript setDelegate:self];
    
    // Setup a rendering timer.
    [NSTimer scheduledTimerWithTimeInterval:1.0/15.0
                                     target:self
                                   selector:@selector(timerFired:)
                                   userInfo:nil
                                    repeats:YES];
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    [self updateViewport:_view.frame.size];
}

- (void)updateViewport:(CGSize)newViewSize {
    if (newViewSize.width == 0 || newViewSize.height == 0)
        return;

    // ゲーム画面のアスペクト比を求める
    float aspect = (float)conf_window_height / (float)conf_window_width;

    // 1. 横幅優先で高さを仮決めする
    _screenSize.width = newViewSize.width;
    _screenSize.height = _screenSize.width * aspect;
    _screenScale = (float)conf_window_width / _screenSize.width;
    
    // 2. 高さが足りなければ、縦幅優先で横幅を決める
    if(_screenSize.height > newViewSize.height) {
        _screenSize.height = newViewSize.height;
        _screenSize.width = _screenSize.height / aspect;
        _screenScale = (float)conf_window_height / _screenSize.height;
    }
    
    // マウス座標用のマージンとスケールを計算する
    _view.left = (newViewSize.width - _screenSize.width) / 2.0f;
    _view.top = (newViewSize.height - _screenSize.height) / 2.0f;
    _view.scale = _screenScale;

    // MTKView用にスケールファクタを乗算する
    _screenScale *= _view.layer.contentsScale;
    _screenSize.width *= _view.layer.contentsScale;
    _screenSize.height *= _view.layer.contentsScale;
    _screenOffset.x = _view.left * _view.layer.contentsScale;
    _screenOffset.y = _view.top * _view.layer.contentsScale;
}

- (void)timerFired:(NSTimer *)timer {
    [_view setNeedsDisplay];
}

//
// Project
//

- (BOOL)initProject {
    self.iCloudDrivePath = [[[NSFileManager defaultManager] URLForUbiquityContainerIdentifier:nil] path];
    if (self.iCloudDrivePath == nil) {
        UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"エラー"
                                       message:@"iCloud Driveが有効ではありません。"
                                       preferredStyle:UIAlertControllerStyleAlert];
        UIAlertAction *defaultAction = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction * action) { exit(0); }];
        [alert addAction:defaultAction];
        [self presentViewController:alert animated:YES completion:nil];
    }
    self.iCloudDrivePath = [self.iCloudDrivePath stringByAppendingString:@"/Documents"];

    NSError *error;
    if (![[NSFileManager defaultManager] isReadableFileAtPath:[self.iCloudDrivePath stringByAppendingString:@"/conf/config.txt"]]) {
        if(![[NSFileManager defaultManager] createDirectoryAtPath:self.iCloudDrivePath
                                      withIntermediateDirectories:YES
                                                       attributes:nil
                                                            error:&error]) {
            NSLog(@"createDirectoryAtPath error: %@", error);
            return false;
        }

        NSArray *subfolderArray = @[@"anime",
                                    @"bg",
                                    @"bgm",
                                    @"cg",
                                    @"ch",
                                    @"conf",
                                    @"cv",
                                    @"font",
                                    @"gui",
                                    @"mov",
                                    @"rule",
                                    @"se",
                                    @"txt",
                                    @"wms"];
        for (NSString *sub in subfolderArray) {
            NSString *src = [NSString stringWithFormat:@"%@/japanese/%@", [[NSBundle mainBundle] bundlePath], sub];
            NSString *dst = [NSString stringWithFormat:@"%@/%@", self.iCloudDrivePath, sub];
            [[NSFileManager defaultManager] removeItemAtPath:dst error:&error];
            [[NSFileManager defaultManager] copyItemAtPath:src toPath:dst error:&error];
        }
    }

    return YES;
}

//
// GameViewControllerProtocol
//

- (float)screenScale {
    return _screenScale;
}

- (CGPoint)screenOffset {
    return _screenOffset;
}

- (CGSize)screenSize {
    return _screenSize;
}

- (CGPoint)windowPointToScreenPoint:(CGPoint)windowPoint {
    return windowPoint;
}

- (BOOL)isVideoPlaying {
    return _isVideoPlaying;
}

- (void)playVideoWithPath:(NSString *)path skippable:(BOOL)isSkippable {
    // プレーヤーを作成する
    AVPlayerItem *playerItem = [[AVPlayerItem alloc] initWithURL:[NSURL fileURLWithPath:path]];
    _avPlayer = [[AVPlayer alloc] initWithPlayerItem:playerItem];

    // プレーヤーのレイヤーを作成する
    _avPlayerLayer = [AVPlayerLayer playerLayerWithPlayer:_avPlayer];
    [_avPlayerLayer setFrame:self.view.bounds];
    [self.view.layer addSublayer:_avPlayerLayer];

    // 再生を開始する
    [_avPlayer play];

    // 再生終了の通知を送るようにする
    [NSNotificationCenter.defaultCenter addObserver:self
                                           selector:@selector(onPlayEnd:)
                                               name:AVPlayerItemDidPlayToEndTimeNotification
                                             object:playerItem];

    _isVideoPlaying = YES;
}

- (void)onPlayEnd:(NSNotification *)notification {
    [_avPlayer replaceCurrentItemWithPlayerItem:nil];
    _isVideoPlaying = NO;
}

- (void)stopVideo {
    if (_avPlayer != nil) {
        [_avPlayer replaceCurrentItemWithPlayerItem:nil];
        _isVideoPlaying = NO;
        _avPlayer = nil;
        _avPlayerLayer = nil;
    }
}

- (void)setTitle:(NSString *)title {
}

- (BOOL)isFullScreen {
    return NO;
}

- (void)enterFullScreen {
}

- (void)leaveFullScreen {
}

//
// ウィンドウ/ビューの設定/取得
//

// 続けるボタンを設定する
- (void)setResumeButton:(BOOL)enabled {
    [[self buttonContinue] setEnabled:enabled];
}

// 次へボタンを設定する
- (void)setNextButton:(BOOL)enabled {
    [[self buttonNext] setEnabled:enabled];
}

// 停止ボタンを設定する
- (void)setStopButton:(BOOL)enabled {
    [[self buttonStop] setEnabled:enabled];
}

// スクリプト名のテキストフィールドの値を設定する
- (void)setScriptName:(NSString *)name {
    [self.textFieldScriptName setText:name];
}

// スクリプト名のテキストフィールドの値を取得する
- (NSString *)getScriptName {
    return [[self textFieldScriptName] text];
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
    [[self textFieldVariables] setEnabled:state];
}

// 変数の書き込みボタンの有効状態を設定する
- (void)enableVariableUpdateButton:(BOOL)state {
    [self.buttonUpdateVariables  setEnabled:state];
}

// 変数のテキストフィールドの値を設定する
- (void)setVariablesText:(NSString *)text {
    [self.textFieldVariables setText:text];
}

// 変数のテキストフィールドの値を取得する
- (NSString *)getVariablesText {
    return [self.textFieldVariables text];
}

///
/// スクリプトのテキストビュー
///

// 実行行を設定する
- (void)setExecLine:(int)line {
    [self selectScriptLine:line];
    [self setTextColorForAllLines];
    [self scrollToCurrentLine];
}

// スクリプトのテキストビューの内容を更新する
- (void)updateScriptTextView {
    [self updateTextFromScriptModel];
    [self setTextColorForAllLines];
}

// テキストビューの内容をスクリプトモデルを元に設定する
- (void)updateTextFromScriptModel {
    // 行を連列してスクリプト文字列を作成する
    NSString *text = @"";
    for (int i = 0; i < get_line_count(); i++) {
        text = [text stringByAppendingString:[[NSString alloc] initWithUTF8String:get_line_string_at_line_num(i)]];
        text = [text stringByAppendingString:@"\n"];
    }

    // テキストビューにテキストを設定する
    _isFirstChange = TRUE;
    [self.textViewScript setText:text];
}

// テキストビューの内容を元にスクリプトモデルを更新する
- (void)updateScriptModelFromText {
    // パースエラーをリセットして、最初のパースエラーで通知を行う
    dbg_reset_parse_error_count();

    // リッチエディットのテキストの内容でスクリプトの各行をアップデートする
    NSString *text = _textViewScript.text;
    NSArray *lines = [text componentsSeparatedByString:@"\n"];
    int lineNum = 0;
    for (NSString *line in lines) {
        if (lineNum < get_line_count())
            update_script_line(lineNum, [line UTF8String]);
        else
            insert_script_line(lineNum, [line UTF8String]);
        lineNum++;
    }

    // 削除された末尾の行を処理する
    self.isExecLineChanged = FALSE;
    for (int i = get_line_count() - 1; i >= lineNum; i--)
        if (delete_script_line(i))
            self.isExecLineChanged = TRUE;
    if (self.isExecLineChanged)
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
    NSString *text = self.textViewScript.text;
    NSRange allRange = NSMakeRange(0, [text length]);
    [self.textViewScript.textStorage removeAttribute:NSForegroundColorAttributeName range:allRange];
    [self.textViewScript.textStorage removeAttribute:NSBackgroundColorAttributeName range:allRange];

    // 行ごとに処理する
    NSArray *lineArray = [text componentsSeparatedByString:@"\n"];
    int startPos = 0;
    int execLineNum = get_expanded_line_num();
    for (int i = 0; i < lineArray.count; i++) {
        NSString *lineText = lineArray[i];
        NSUInteger lineLen = [lineText length];
        if (lineLen == 0) {
            startPos++;
            continue;
        }

        NSRange lineRange = NSMakeRange(startPos, [lineArray[i] length]);

        // 実行行であれば背景色を設定する
        if (i == execLineNum) {
            UIColor *bgColor = self.isRunning ?
                [UIColor colorWithRed:0xff/255.0f green:0xc0/255.0f blue:0xc0/255.0f alpha:1.0f] :
                [UIColor colorWithRed:0xc0/255.0f green:0xc0/255.0f blue:0xff/255.0f alpha:1.0f];
            [self.textViewScript.textStorage addAttribute:NSBackgroundColorAttributeName value:bgColor range:lineRange];
        }

        // コメントを処理する
        if ([lineText characterAtIndex:0] == L'#') {
            // 行全体のテキスト色を変更する
            [self.textViewScript.textStorage
                addAttribute:NSForegroundColorAttributeName
                       value:[UIColor colorWithRed:0.5f green:0.5f blue:0.5f alpha:1.0f]
                       range:lineRange];
        }
        // ラベルを処理する
        else if ([lineText characterAtIndex:0] == L':') {
            // 行全体のテキスト色を変更する
            [self.textViewScript.textStorage
                addAttribute:NSForegroundColorAttributeName
                       value:[UIColor colorWithRed:1.0f green:0 blue:0 alpha:1.0f]
                       range:lineRange];
        }
        // エラー行を処理する
        else if ([lineText characterAtIndex:0] == L'!') {
            // 行全体のテキスト色を変更する
            [self.textViewScript.textStorage
                addAttribute:NSForegroundColorAttributeName
                       value:[UIColor colorWithRed:1.0f green:0 blue:0 alpha:1.0f]
                       range:lineRange];
        }
        // コマンド行を処理する
        else if ([lineText characterAtIndex:0] == L'@') {
            // コマンド名部分を抽出する
            NSUInteger commandNameLen = [lineText rangeOfString:@" "].location;
            if (commandNameLen == NSNotFound)
                commandNameLen = [lineText length];

            // コマンド名のテキストに色を付ける
            NSRange commandRange = NSMakeRange(startPos, commandNameLen);
            [self.textViewScript.textStorage
                addAttribute:NSForegroundColorAttributeName
                       value:[UIColor colorWithRed:0 green:0 blue:1.0f alpha:1.0f]
                       range:commandRange];

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
                    NSRange paramNameRange = NSMakeRange(paramStart, eqPos + 2);
                    [self.textViewScript.textStorage addAttribute:NSForegroundColorAttributeName
                               value:[UIColor colorWithRed:0xc0/255.0f green:0xf0/255.0f blue:0xc0/255.0f alpha:1.0f]
                               range:paramNameRange];

                    paramStart += spacePos;
                } while (paramStart < lineLen);
            }
        }
        
        startPos += [lineText length] + 1;
    }
}

- (int)scriptCursorLine {
    NSString *text = _textViewScript.text;
    NSArray *lines = [text componentsSeparatedByString:@"\n"];
    int cur = (int)_textViewScript.selectedRange.location;
    int lineCount = 0;
    int lineTop = 0;
    for (NSString *line in lines) {
        int lineLen = (int)line.length;
        if (cur >= lineTop && cur <= lineTop + lineLen)
            return lineCount;
        lineCount++;
        lineTop += lineLen + 1;
    }
    return 0;
}

- (void)selectScriptLine:(int)lineToSelect {
    NSString *text = _textViewScript.text;
    NSArray *lines = [text componentsSeparatedByString:@"\n"];
    int lineCount = 0;
    int lineTop = 0;
    for (NSString *line in lines) {
        int lineLen = (int)line.length;
        if (lineCount == lineToSelect) {
            _textViewScript.selectedRange = NSMakeRange(lineTop, lineLen);
            return;
        }
        lineCount++;
        lineTop += lineLen + 1;
    }
}

- (void)scrollToCurrentLine {
    [_textViewScript scrollRangeToVisible:NSMakeRange(_textViewScript.selectedRange.location, 0)];
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

        [self.textFieldVariables setText:text];
    }
}

- (IBAction)onContinueButton:(id)sender {
    self.isContinuePressed = true;
}

- (IBAction)onNextButton:(id)sender {
    self.isNextPressed = true;
}

- (IBAction)onStopButton:(id)sender {
    self.isStopPressed = true;
}

- (IBAction)onUpdateButton:(id)sender {
    [self updateScriptModelFromText];
    
    self.changedExecLine = [self scriptCursorLine];
    self.isExecLineChanged = YES;

    save_script();
}

@end

//
// HAL
//

static FILE *fp;

//
// INFOログを出力する
//
bool log_info(const char *s, ...)
{
    char buf[1024];
    va_list ap;
    
    va_start(ap, s);
    vsnprintf(buf, sizeof(buf), s, ap);
    va_end(ap);

    NSLog(@"%s", buf);

    if (fp == NULL)
        fp = fopen(make_valid_path(NULL, "log.txt"), "w");
    if (fp != NULL)
        fprintf(fp, "%s\n", buf);

    return true;
}

//
// WARNログを出力する
//
bool log_warn(const char *s, ...)
{
    char buf[1024];
    va_list ap;

    va_start(ap, s);
    vsnprintf(buf, sizeof(buf), s, ap);
    va_end(ap);

    NSLog(@"%s", buf);

    if (fp == NULL)
        fp = fopen(make_valid_path(NULL, "log.txt"), "w");
    if (fp != NULL)
        fprintf(fp, "%s\n", buf);

    return true;
}

//
// Errorログを出力する
//
bool log_error(const char *s, ...)
{
    char buf[1024];
    va_list ap;

    va_start(ap, s);
    vsnprintf(buf, sizeof(buf), s, ap);
    va_end(ap);

    NSLog(@"%s", buf);

    if (fp == NULL)
        fp = fopen(make_valid_path(NULL, "log.txt"), "w");
    if (fp != NULL)
        fprintf(fp, "%s\n", buf);

    return true;
}

//
// セーブディレクトリを作成する
//
bool make_sav_dir(void)
{
    @autoreleasepool {
        NSFileManager *manager = [NSFileManager defaultManager];
        NSError *error;
        if (![manager createDirectoryAtPath:[theViewController.iCloudDrivePath stringByAppendingString:@"/sav"]
                withIntermediateDirectories:YES
                                 attributes:nil
                                      error:&error]) {
            NSLog(@"createDirectoryAtPath error: %@", error);
            return false;
        }
        return true;
    }
}

//
// データファイルのディレクトリ名とファイル名を指定して有効なパスを取得する
//
char *make_valid_path(const char *dir, const char *fname)
{
    @autoreleasepool {
        if(dir != NULL && fname != NULL) {
            NSString *path = [NSString stringWithFormat:@"%@/%s/%s",
                              theViewController.iCloudDrivePath,
                              dir,
                              fname];
            const char *cstr = [path UTF8String];
            return strdup(cstr);
        }
        if(dir == NULL && fname != NULL) {
            NSString *path = [NSString stringWithFormat:@"%@/%s",
                              theViewController.iCloudDrivePath,
                              fname];
            const char *cstr = [path UTF8String];
            return strdup(cstr);
        }
        if(dir != NULL && fname == NULL) {
            NSString *path = [NSString stringWithFormat:@"%@/%s",
                              theViewController.iCloudDrivePath,
                              fname];
            const char *cstr = [path UTF8String];
            return strdup(cstr);
        }

        assert(0);
        return NULL;
    }
}

//
// Pro HAL
//

bool is_continue_pushed(void)
{
    bool ret = theViewController.isContinuePressed;
    theViewController.isContinuePressed = false;
    return ret;
}

bool is_next_pushed(void)
{
    bool ret = theViewController.isNextPressed;
    theViewController.isNextPressed = false;
    return ret;
}

//
// 停止ボタンが押されたか調べる
//
bool is_stop_pushed(void)
{
    bool ret = theViewController.isStopPressed;
    theViewController.isStopPressed = false;
    return ret;
}

//
// 実行するスクリプトファイルが変更されたか調べる
//
bool is_script_opened(void)
{
    bool ret = theViewController.isOpenScriptPressed;
    theViewController.isOpenScriptPressed = false;
    return ret;
}

//
// 変更された実行するスクリプトファイル名を取得する
//
const char *get_opened_script(void)
{
    static char script[256];
    snprintf(script, sizeof(script), "%s", [[theViewController getScriptName] UTF8String]);
    return script;
}

//
// 実行する行番号が変更されたか調べる
//
bool is_exec_line_changed(void)
{
    bool ret = theViewController.isExecLineChanged;
    theViewController.isExecLineChanged = false;
    return ret;
}

//
// 変更された行番号を取得する
//
int get_changed_exec_line(void)
{
    return theViewController.changedExecLine;
}

//
// コマンドの実行中状態を設定する
//
void on_change_running_state(bool running, bool request_stop)
{
    // 実行状態を保存する
    theViewController.isRunning = running ? YES : NO;

    // 停止によりコマンドの完了を待機中のとき
    if(request_stop) {
        setWaitingState();
        [theViewController setTextColorForAllLines];
        return;
    }

    // 実行中のとき
    if(running) {
        setRunningState();
        [theViewController setTextColorForAllLines];
        return;
    }

    // 完全に停止中のとき
    setStoppedState();
    [theViewController setTextColorForAllLines];
}

// 停止によりコマンドの完了を待機中のときのビューの状態を設定する
static void setWaitingState(void)
{
    theViewController.buttonContinue.enabled = NO;
    theViewController.buttonNext.enabled = NO;
    theViewController.buttonStop.enabled = NO;
    theViewController.buttonUpdate.enabled = NO;
    theViewController.buttonOpenScript.enabled = NO;
    theViewController.textViewScript.editable = NO;
    theViewController.textFieldVariables.enabled = NO;
    theViewController.buttonUpdateVariables.enabled = NO;
}

// 実行中のときのビューの状態を設定する
static void setRunningState(void)
{
    theViewController.buttonContinue.enabled = NO;
    theViewController.buttonNext.enabled = NO;
    theViewController.buttonStop.enabled = YES;
    theViewController.buttonUpdate.enabled = NO;
    theViewController.buttonOpenScript.enabled = NO;
    theViewController.textViewScript.editable = NO;
    theViewController.textFieldVariables.enabled = NO;
    theViewController.buttonUpdateVariables.enabled = NO;
}

// 完全に停止中のときのビューの状態を設定する
static void setStoppedState(void)
{
    theViewController.buttonContinue.enabled = YES;
    theViewController.buttonNext.enabled = YES;
    theViewController.buttonStop.enabled = NO;
    theViewController.buttonUpdate.enabled = YES;
    theViewController.buttonOpenScript.enabled = YES;
    theViewController.textViewScript.editable = YES;
    theViewController.textFieldVariables.enabled = YES;
    theViewController.buttonUpdateVariables.enabled = YES;
}

void on_load_script(void)
{
    [theViewController setScriptName:[[NSString alloc] initWithUTF8String:get_script_file_name()]];
    [theViewController updateScriptTextView];
}

void on_change_position(void)
{
    [theViewController setExecLine:get_expanded_line_num()];
}

void on_update_variable(void)
{
    [theViewController updateVariableTextField];
}
