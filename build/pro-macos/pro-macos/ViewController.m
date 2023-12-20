// -*- coding: utf-8; indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

#import "ViewController.h"
#import "ViewController.h"
#import "GameView.h"
#import "GameRenderer.h"

#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

// Suika2 Base
#import "suika.h"
#import "pro.h"
#import "package.h"

// Standard C
#import <wchar.h>

// Suika2 HAL
#import "aunit.h"

static ViewController *theViewController;

static void setWaitingState(void);
static void setRunningState(void);
static void setStoppedState(void);

@interface ViewController ()
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
@property (weak) IBOutlet NSView *editorPanel;
@property (weak) IBOutlet NSButton *buttonContinue;
@property (weak) IBOutlet NSButton *buttonNext;
@property (weak) IBOutlet NSButton *buttonStop;
@property (weak) IBOutlet NSTextField *textFieldScriptName;
@property (weak) IBOutlet NSButton *buttonOpenScript;
@property (unsafe_unretained) IBOutlet NSTextView *textViewScript;
@property (unsafe_unretained) IBOutlet NSTextField *textFieldVariables;
@property (weak) IBOutlet NSButton *buttonUpdateVariables;
@end

@implementation ViewController
{
    // The GameRender (common for AppKit and UIKit)
    GameRenderer *_renderer;
    
    // The screen information
    float _screenScale;
    NSSize _screenSize;
    NSPoint _screenOffset;
    
    // The view frame before entering a full screen mode.
    NSRect _savedViewFrame;
    
    // The temporary window frame size on resizing.
    NSSize _resizeFrame;
    
    // The full screen status.
    BOOL _isFullScreen;
    
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

    _isEnglish = [[[NSLocale preferredLanguages] objectAtIndex:0] hasPrefix:@"ja-"] ? false : true;
    theViewController = self;
}

- (void)viewDidAppear {
    self.view.window.delegate = self;

    // Initialize a project.
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
    self.renderView.enableSetNeedsDisplay = YES;
    self.renderView.device = MTLCreateSystemDefaultDevice();
    self.renderView.clearColor = MTLClearColorMake(0.0, 0, 0, 1.0);
    _renderer = [[GameRenderer alloc] initWithMetalKitView:self.renderView andController:self];
    if(!_renderer) {
        NSLog(@"Renderer initialization failed");
        return;
    }
    [_renderer mtkView:self.renderView drawableSizeWillChange:self.renderView.drawableSize];
    self.renderView.delegate = _renderer;
    [self updateViewport:self.renderView.frame.size];

    // Setup a rendering timer.
    [NSTimer scheduledTimerWithTimeInterval:1.0/60.0
                                     target:self
                                   selector:@selector(timerFired:)
                                   userInfo:nil
                                    repeats:YES];

    // Accept keyboard and mouse inputs.
    [self.view.window makeKeyAndOrderFront:nil];
    [self.view.window setAcceptsMouseMovedEvents:YES];
    [self.view.window.delegate self];
    [self.view.window makeFirstResponder:self];

    // Set the window title.
    [self.view.window setTitle:@"Suika2"];

    // Set the script view delegate.
    _textViewScript.delegate = (id<NSTextViewDelegate>)_textViewScript;

    // Update the viewport size.
    [self updateViewport:self.renderView.frame.size];
}

- (void)timerFired:(NSTimer *)timer {
    [self.renderView setNeedsDisplay:TRUE];
}

- (void)windowWillClose:(NSNotification *)notification {
    // Save.
    save_global_data();
    save_seen();
    
    // Exit the event loop.
    [NSApp stop:nil];
    
    // Magic: Post an empty event and make sure to exit the main loop.
    [NSApp postEvent:[NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                        location:NSMakePoint(0, 0)
                                   modifierFlags:0
                                       timestamp:0
                                    windowNumber:0
                                         context:nil
                                         subtype:0
                                           data1:0
                                           data2:0]
             atStart:YES];
}

// フルスクリーンになる前に呼び出される
- (NSSize)window:(NSWindow *)window willUseFullScreenContentSize:(NSSize)proposedSize {
    // 表示位置を更新する
    proposedSize.width -= _editorPanel.frame.size.width;
    [self updateViewport:proposedSize];
    
    // 動画プレーヤレイヤのサイズを更新する
    if (_avPlayerLayer != nil)
        [_avPlayerLayer setFrame:NSMakeRect(_screenOffset.x, _screenOffset.y, _screenSize.width, _screenSize.height)];
    
    // スクリーンサイズを返す
    return proposedSize;
}

// フルスクリーンになるとき呼び出される
- (void)windowWillEnterFullScreen:(NSNotification *)notification {
    _isFullScreen = YES;

    // ウィンドウサイズを保存する
    _savedViewFrame = self.view.frame;
}

// フルスクリーンから戻るときに呼び出される
- (void)windowWillExitFullScreen:(NSNotification *)notification {
    _isFullScreen = NO;
    
    // 動画プレーヤレイヤのサイズを元に戻す
    if(_avPlayerLayer != nil)
        [_avPlayerLayer setFrame:NSMakeRect(0, 0, _savedViewFrame.size.width, _savedViewFrame.size.height)];
    
    [self updateViewport:_savedViewFrame.size];
}

- (void)windowDidResize:(NSNotification *)notification {
    [self updateViewport:self.renderView.frame.size];
}

- (void)updateViewport:(NSSize)newViewSize {
    if (newViewSize.width == 0 || newViewSize.height == 0) {
        _screenScale = 1.0f;
        _screenSize = NSMakeSize(conf_window_width, conf_window_height);
        _screenOffset.x = 0;
        _screenOffset.y = 0;
        return;
    }
    
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
    
    // スケールファクタを乗算する
    _screenSize.width *= self.renderView.layer.contentsScale;
    _screenSize.height *= self.renderView.layer.contentsScale;
    newViewSize.width *= self.renderView.layer.contentsScale;
    newViewSize.height *= self.renderView.layer.contentsScale;

    // マージンを計算する
    _screenOffset.x = (newViewSize.width - _screenSize.width) / 2.0f;
    _screenOffset.y = (newViewSize.height - _screenSize.height) / 2.0f;
}

- (void)mouseMoved:(NSEvent *)event {
    NSPoint point = [self windowPointToScreenPoint:[event locationInWindow]];
    on_event_mouse_move(point.x, point.y);
}

// キーボード修飾変化イベント
- (void)flagsChanged:(NSEvent *)event {
    BOOL newControllPressed = ([event modifierFlags] & NSEventModifierFlagControl) ==
        NSEventModifierFlagControl;
    self.isShiftPressed = ([event modifierFlags] & NSEventModifierFlagShift) ==
        NSEventModifierFlagShift;
    self.isCommandPressed = ([event modifierFlags] & NSEventModifierFlagCommand) ==
        NSEventModifierFlagCommand;

    // Controlキーの状態が変化した場合は通知する
    if (!self.isControlPressed && newControllPressed) {
        self.isControlPressed = YES;
        on_event_key_press(KEY_CONTROL);
    } else if (self.isControlPressed && !newControllPressed) {
        self.isControlPressed = NO;
        on_event_key_release(KEY_CONTROL);
    }
}

// キー押下イベント
- (void)keyDown:(NSEvent *)theEvent {
    if ([theEvent isARepeat])
        return;
    
    int kc = [self convertKeyCode:[theEvent keyCode]];
    if (kc != -1)
        on_event_key_press(kc);
}

// キー解放イベント
- (void)keyUp:(NSEvent *)theEvent {
    int kc = [self convertKeyCode:[theEvent keyCode]];
    if (kc != -1)
        on_event_key_release(kc);
}

// キーコードを変換する
- (int)convertKeyCode:(int)keyCode {
    switch(keyCode) {
        case 49: return KEY_SPACE;
        case 36: return KEY_RETURN;
        case 123: return KEY_LEFT;
        case 124: return KEY_RIGHT;
        case 125: return KEY_DOWN;
        case 126: return KEY_UP;
        case 53: return KEY_ESCAPE;
    }
    return -1;
}

- (NSString *)uiMessage:(int)id {
    return [[NSString alloc] initWithUTF8String:get_ui_message(id)];
}

- (BOOL)initProject {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:_isEnglish ? @"Going to create a new game.\n" : @"新規ゲームを作成します。\n"];
    [alert addButtonWithTitle:_isEnglish ? @"Yes" : @"はい"];
    [alert addButtonWithTitle:_isEnglish ? @"No" : @"いいえ"];
    [alert setAlertStyle:NSAlertStyleInformational];
    if ([alert runModal] == NSAlertFirstButtonReturn)
        return [self createProject];
    
    return [self openProject];
}

- (BOOL)createProject {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseDirectories:YES];
    [panel setCanCreateDirectories:YES];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return NO;
    }
    if ([[NSFileManager defaultManager] isReadableFileAtPath:[[[panel directoryURL] path] stringByAppendingString:@"/conf/config.txt"]]) {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:_isEnglish ? @"The folder you selected in not empty" : @"フォルダが空ではありません。"];
        [alert addButtonWithTitle:@"OK"];
        [alert setAlertStyle:NSAlertStyleCritical];
        [alert runModal];
        return NO;
    }
    [[NSFileManager defaultManager] changeCurrentDirectoryPath:[[panel directoryURL] path]];

    FILE *fp = fopen([[[[panel URL] path] stringByAppendingString:@"/game.suika2project"] UTF8String], "w");
    if (fp == NULL)
        return NO;
    fclose(fp);

    NSAlert *alert;
    alert = [[NSAlert alloc] init];
    [alert setMessageText:_isEnglish ? @"Do you want to use the full screen style?" : @"全画面スタイルにしますか？\n"];
    [alert addButtonWithTitle:_isEnglish ? @"Yes" : @"はい"];
    [alert addButtonWithTitle:_isEnglish ? @"No" : @"いいえ"];
    [alert setAlertStyle:NSAlertStyleInformational];
    if ([alert runModal] == NSAlertFirstButtonReturn) {
        if (_isEnglish)
            return [self copyResourceTemplate:@"nvl"];

        alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:@"縦書きにしますか？"];
        [alert addButtonWithTitle:@"はい"];
        [alert setMessageText:@"いいえ"];
        [alert setAlertStyle:NSAlertStyleInformational];
        if ([alert runModal] == NSAlertFirstButtonReturn)
            return [self copyResourceTemplate:@"nvl-tategaki"];
        else
            return [self copyResourceTemplate:@"nvl"];
    }
    if (_isEnglish)
        return [self copyResourceTemplate:@"english"];
    return [self copyResourceTemplate:@"japanese"];
}

- (BOOL)copyResourceTemplate:(NSString *)from {
    NSArray *subfolderArray = @[@"/anime", @"/bg", @"/bgm", @"/cg", @"/ch", @"/conf", @"/cv", @"/font", @"/gui", @"/mov", @"/rule", @"/se", @"/txt", @"/wms"];
    for (NSString *sub in subfolderArray) {
        NSString *src = [NSString stringWithFormat:@"%@/Contents/Resources/%@%@", [[NSBundle mainBundle] bundlePath], from, sub];
        NSString *dst = [[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:sub];
        if (![[NSFileManager defaultManager] copyItemAtPath:src toPath:dst error:nil])
            return NO;
    }
    return YES;
}

- (BOOL)openProject {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    if (@available(macOS 11.0, *)) {
        [panel setAllowedContentTypes:@[[UTType typeWithFilenameExtension:@"suika2project" conformingToType:UTTypeData]]];
    } else {
        [panel setAllowedFileTypes:@[@"suika2project"]];
    }
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return NO;
    }
    NSString *dir = [[[panel URL] URLByDeletingLastPathComponent] path];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    [fileManager changeCurrentDirectoryPath:dir];
    return YES;
}

//
// GameViewControllerProtocol
//

- (float)screenScale {
    return _screenScale;
}

- (NSPoint)screenOffset {
    return _screenOffset;
}

- (NSSize)screenSize {
    return _screenSize;
}

- (NSPoint)windowPointToScreenPoint:(NSPoint)windowPoint {
    float retinaScale = _renderView.layer.contentsScale;

    int x = (int)(windowPoint.x - (_screenOffset.x / retinaScale)) * _screenScale;
    int y = (int)(windowPoint.y - (_screenOffset.y / retinaScale)) * _screenScale;

    return NSMakePoint(x, conf_window_height - y);
}

- (BOOL)isFullScreen {
    return _isFullScreen;
}

- (void)enterFullScreen {
    if (!_isFullScreen)
        [self.view.window toggleFullScreen:self.view];
}

- (void)leaveFullScreen {
    if (!_isFullScreen)
        [self.view.window toggleFullScreen:self.view];
}

- (BOOL)isVideoPlaying {
    return _isVideoPlaying;
}

- (void)playVideoWithPath:(NSString *)path skippable:(BOOL)isSkippable {
    // プレーヤーを作成する
    NSURL *url = [NSURL URLWithString:[@"file://" stringByAppendingString:path]];
    AVPlayerItem *playerItem = [[AVPlayerItem alloc] initWithURL:url];
    _avPlayer = [[AVPlayer alloc] initWithPlayerItem:playerItem];

    // プレーヤーのレイヤーを作成する
    [self.view setWantsLayer:YES];
    _avPlayerLayer = [AVPlayerLayer playerLayerWithPlayer:_avPlayer];
    [_avPlayerLayer setFrame:theViewController.view.bounds];
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

//
// IB Actions
//

- (IBAction)onAbout:(id)sender {
    [NSApp orderFrontStandardAboutPanel:self];
}

// 続けるボタンが押下されたイベント
- (IBAction) onContinueButton:(id)sender {
    self.isContinuePressed = true;
}

// 次へボタンが押下されたイベント
- (IBAction) onNextButton:(id)sender {
    self.isNextPressed = true;
}

// 停止ボタンが押下されたイベント
- (IBAction) onStopButton:(id)sender {
    self.isStopPressed = true;
}

// スクリプトファイル名の反映ボタンが押下されたイベント
- (IBAction)onOpenScriptButton:(id)sender {
    NSString *basePath = [[NSFileManager defaultManager] currentDirectoryPath];
    NSString *txtPath = [NSString stringWithFormat:@"%@/%@", basePath, @"txt"];
    NSOpenPanel *panel= [NSOpenPanel openPanel];
    [panel setAllowedFileTypes:[NSArray arrayWithObjects:@"txt", @"'TEXT'", nil]];
    [panel setDirectoryURL:[[NSURL alloc] initFileURLWithPath:txtPath]];
    if ([panel runModal] == NSModalResponseOK) {
        NSString *file = [[panel URL] lastPathComponent];
        if ([file hasPrefix:txtPath]) {
                [self setScriptName:file];
                self.isOpenScriptPressed = true;
        }
    }
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

- (IBAction)onQuit:(id)sender {
    [NSApp stop:nil];
}

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
                self.isOpenScriptPressed = true;
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
    [alert addButtonWithTitle:!self.isEnglish ? @"はい" : @"Yes"];
    [alert addButtonWithTitle:!self.isEnglish ? @"いいえ" : @"No"];
    [alert setMessageText:self.isEnglish ?
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
        log_error(self.isEnglish ?
                  "Cannot write to file." : "ファイルに書き込めません。");
        return;
    }

    // 書き出す
    for (int i = 0; i < get_line_count(); i++) {
        int body = fputs(get_line_string_at_line_num(i), fp);
        int lf = fputs("\n", fp);
        if (body < 0 || lf < 0) {
            log_error(self.isEnglish ?
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

    log_info(self.isEnglish ? "No error." : "エラーはありません。");
}

- (void)insertText:(NSString *)command {
    NSString *text = _textViewScript.string;
    NSArray *lines = [text componentsSeparatedByString:@"\n"];
    int cur = (int)_textViewScript.selectedRange.location;
    int lineCount = 0;
    int lineTop = 0;
    for (NSString *line in lines) {
        int lineLen = (int)line.length;
        if (cur >= lineTop && cur <= lineTop + lineLen)
            break;
        lineCount++;
        lineTop += lineLen + 1;
    }

    [_textViewScript insertText:command replacementRange:NSMakeRange(lineTop, 0)];
}

// メッセージの挿入
- (IBAction)onMenuMessage:(id)sender {
    if (self.isEnglish)
        [self insertText:@"Edit this message and press return."];
    else
        [self insertText:@"この行のメッセージを編集して改行してください。"];
}

// セリフの挿入
- (IBAction)onMenuLine:(id)sender {
    if (self.isEnglish)
        [self insertText:@"*Name*Edit this line and press return."];
    else
        [self insertText:@"名前「このセリフを編集して改行してください。」"];
}

// セリフ(ボイスつき)の挿入
- (IBAction)onMenuLineWithVoice:(id)sender {
    if (self.isEnglish)
        [self insertText:@"*Name*001.ogg*Edit this line and press return."];
    else
        [self insertText:@"*名前*001.ogg*このセリフを編集して改行してください。"];
}

// 背景変更の挿入
- (IBAction)onMenuBackground:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/bg"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@bg file=%@ duration=1.0", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@背景 ファイル=%@ 秒=1.0", file]];
}

// 背景のみ変更の挿入
- (IBAction)onMenuBackgroundOnly:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/bg"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@chsx bg=%@ duration=1.0", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@場面転換X 背景=%@ 秒=1.0", file]];
}

// 左キャラの表示
- (IBAction)onMenuShowLeftCharacter:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/ch"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@ch position=left file=%@ duration=1.0", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@キャラ 位置=左 ファイル=%@ 秒=1.0", file]];
}

// 左キャラの非表示
- (IBAction)onMenuHideLeftCharacter:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@ch position=left file=none duration=1.0"];
    else
        [self insertText:@"@キャラ 位置=左 ファイル=なし 秒=1.0"];
}

// 左中央キャラの表示
- (IBAction)onMenuShowLeftCenterCharacter:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/ch"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@ch position=left-center file=%@ duration=1.0", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@キャラ 位置=左中央 ファイル=%@ 秒=1.0", file]];
}

// 左中央キャラの非表示
- (IBAction)onMenuHideLeftCenterCharacter:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@ch position=left-center file=none duration=1.0"];
    else
        [self insertText:@"@キャラ 位置=左中央 ファイル=なし 秒=1.0"];
}

// 中央キャラの表示
- (IBAction)onMenuShowCenterCharacter:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/ch"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@ch position=center file=%@ duration=1.0", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@キャラ 位置=中央 ファイル=%@ 秒=1.0", file]];
}

// 中央キャラの非表示
- (IBAction)onMenuHideCenterCharacter:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@ch position=center file=none duration=1.0"];
    else
        [self insertText:@"@キャラ 位置=中央 ファイル=なし 秒=1.0"];
}

// 右中央キャラの表示
- (IBAction)onMenuShowRightwCenterCharacter:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/ch"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@ch position=right-center file=%@ duration=1.0", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@キャラ 位置=右中央 ファイル=%@ 秒=1.0", file]];
}

// 右中央キャラの非表示
- (IBAction)onMenuHideRightCenterCharacter:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@ch position=right-center file=none duration=1.0"];
    else
        [self insertText:@"@キャラ 位置=右中央 ファイル=なし 秒=1.0"];
}

// 右キャラの表示
- (IBAction)onMenuShowRightCharacter:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/ch"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@ch position=right file=%@ duration=1.0", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@キャラ 位置=右 ファイル=%@ 秒=1.0", file]];
}

// 右キャラの非表示
- (IBAction)onMenuHideRightCharacter:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@ch position=right file=none duration=1.0"];
    else
        [self insertText:@"@キャラ 位置=右 ファイル=なし 秒=1.0"];
}

// 複数キャラの変更
- (IBAction)onMenuChsx:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@chsx left=stay center=stay right=stay duration=1.0"];
    else
        [self insertText:@"@場面転換X left=stay center=stay right=stay duration=1.0"];
}

// BGM再生
- (IBAction)onMenuBgmPlay:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/bgm"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@bgm file=%@", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@音楽 ファイル=%@", file]];
}

// BGM停止
- (IBAction)onMenuBgmStop:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@bgm file=stop"];
    else
        [self insertText:@"@音楽 ファイル=停止"];
}

// BGMボリューム
- (IBAction)onMenuBgmVolume:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@vol track=bgm volume=1.0"];
    else
        [self insertText:@"@音量 トラック=bgm 音量=1.0"];
}

// SE再生
- (IBAction)onMenuSePlay:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/se"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@se file=%@", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@効果音 ファイル=%@", file]];
}

// SE停止
- (IBAction)onMenuSeStop:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@se file=stop"];
    else
        [self insertText:@"@効果音 ファイル=停止"];
}

// SEボリューム
- (IBAction)onMenuSeVolume:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@vol track=se volume=1.0"];
    else
        [self insertText:@"@音量 トラック=se 音量=1.0"];
}

// ボイス再生
- (IBAction)onMenuVoicePlay:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/cv"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@se file=%@ voice", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@効果音 ファイル=%@ voice", file]];
}

// ボイス停止
- (IBAction)onMenuVoiceStop:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@se file=stop voice"];
    else
        [self insertText:@"@効果音 ファイル=停止 voice"];
}

// ボイスボリューム
- (IBAction)onMenuVoiceVolume:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@vol track=voice volume=1.0"];
    else
        [self insertText:@"@音量 トラック=voice 音量=1.0"];
}

// ビデオ
- (IBAction)onMenuVideo:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/mov"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@video file=%@", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@動画 ファイル=%@", file]];
}

// 選択肢1
- (IBAction)onMenuChoose1:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@choose Label1 \"Option1\""];
    else
        [self insertText:@"@選択肢 ラベル1 \"選択肢１\""];
}

// 選択肢2
- (IBAction)onMenuChoose2:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@choose Label1 \"Option1\" Label2 \"Option2\""];
    else
        [self insertText:@"@選択肢 ラベル1 \"選択肢１\" ラベル2 \"選択肢２\""];
}

// 選択肢3
- (IBAction)onMenuChoose3:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@choose Label1 \"Option1\" Label2 \"Option2\" Label3 \"Option3\""];
    else
        [self insertText:@"@選択肢 ラベル1 \"選択肢１\" ラベル2 \"選択肢２\" ラベル3 \"選択肢３\""];
}

// クリック
- (IBAction)onMenuClick:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@click"];
    else
        [self insertText:@"@クリック待ち"];
}

// 時間待ち
- (IBAction)onMenuWait:(id)sender {
    if (self.isEnglish)
        [self insertText:@"@wait duration=1.0"];
    else
        [self insertText:@"@時間待ち 秒=1.0"];
}

// gui
- (IBAction)onMenuGUI:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/gui"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@gui file=%@", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@メニュー ファイル=%@", file]];
}

// wms
- (IBAction)onMenuWMS:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/wms"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@wms file=%@", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@スクリプト ファイル=%@", file]];
}

// load
- (IBAction)onMenuLoad:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.directoryURL = [NSURL fileURLWithPath:[[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingString:@"/txt"]];
    if ([panel runModal] != NSModalResponseOK) {
        [NSApp stop:nil];
        return;
    }
    NSString *file = [[[panel URL] path] lastPathComponent];
    if (self.isEnglish)
        [self insertText:[NSString stringWithFormat:@"@load file=%@", file]];
    else
        [self insertText:[NSString stringWithFormat:@"@シナリオ ファイル=%@", file]];
}

- (IBAction)onMenuExportForDesktop:(id)sender {
    if (!create_package("")) {
        log_info("Export error.");
        return;
    }
   
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *exportPath = [NSString stringWithFormat:@"%@/export-desktop", [fileManager currentDirectoryPath]];
    [fileManager removeItemAtURL:[NSURL fileURLWithPath:exportPath] error:nil];
    if (![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:exportPath] withIntermediateDirectories:NO attributes:nil error:nil]) {
        log_warn("mkdir error.");
        return;
    }

    if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/data01.arc", [fileManager currentDirectoryPath]]
                            toPath:[NSString stringWithFormat:@"%@/export-desktop/data01.arc", [fileManager currentDirectoryPath]]
                               error:nil]) {
        log_warn("Copy error (1).");
        return;
    }

    NSArray *appArray = @[@"suika-mac.zip", @"suika.exe"];
    for (NSString *sub in appArray) {
        if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/Contents/Resources/%@", [[NSBundle  mainBundle] bundlePath], sub]
                                  toPath:[NSString stringWithFormat:@"%@/export-desktop/%@", [fileManager currentDirectoryPath], sub]
                                   error:nil]) {
            log_warn("Copy error (2).");
            return;
        }
    }

    log_info(_isEnglish ? "Successflully exported" : "エクスポートに成功しました。");

    [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:[[fileManager currentDirectoryPath] stringByAppendingString:@"/export-desktop"]]];
}

- (IBAction)onMenuExportForWeb:(id)sender {
    if (!create_package("")) {
        log_info("Export error.");
        return;
    }

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *exportPath = [NSString stringWithFormat:@"%@/export-web", [fileManager currentDirectoryPath]];
    [fileManager removeItemAtURL:[NSURL fileURLWithPath:exportPath] error:nil];
    if (![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:exportPath] withIntermediateDirectories:NO attributes:nil error:nil]) {
        log_warn("mkdir error.");
        return;
    }

    if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/data01.arc", [fileManager currentDirectoryPath]]
                            toPath:[NSString stringWithFormat:@"%@/data01.arc", exportPath]
                             error:nil]) {
        log_warn("Copy error (1).");
        return;
    }

    NSArray *appArray = @[@"index.html", @"index.js", @"index.wasm"];
    for (NSString *sub in appArray) {
        if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/Contents/Resources/%@", [[NSBundle  mainBundle] bundlePath], sub]
                                  toPath:[NSString stringWithFormat:@"%@/%@", exportPath, sub]
                                   error:nil]) {
            log_warn("Copy error (2).");
            return;
        }
    }
    
    log_info(_isEnglish ? "Successflully exported" : "エクスポートに成功しました。");
    
    [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:[[fileManager currentDirectoryPath] stringByAppendingString:@"/export-web"]]];
}

// パッケージのエクスポートが押下されたイベント
- (IBAction)onMenuExportForIOS:(id)sender {
    if (!create_package("")) {
        log_info("Export error.");
        return;
    }

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *exportPath = [NSString stringWithFormat:@"%@/export-ios", [fileManager currentDirectoryPath]];
    [fileManager removeItemAtURL:[NSURL fileURLWithPath:exportPath] error:nil];
    if (![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:exportPath] withIntermediateDirectories:NO attributes:nil error:nil]) {
        log_warn("mkdir error.");
        return;
    }

    NSArray *appArray = @[@"libroot", @"src", @"engine-ios", @"engine-ios.xcodeproj"];
    for (NSString *sub in appArray) {
        if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/Contents/Resources/ios-src/%@", [[NSBundle  mainBundle] bundlePath], sub]
                                  toPath:[NSString stringWithFormat:@"%@/export-ios/%@", [fileManager currentDirectoryPath], sub]
                                   error:nil]) {
            log_warn("Copy error (1).");
            return;
        }
    }

    if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/data01.arc", [fileManager currentDirectoryPath]]
                              toPath:[NSString stringWithFormat:@"%@/engine-ios/data01.arc", exportPath]
                               error:nil]) {
        log_warn("Copy error (2).");
        return;
    }

    log_info(_isEnglish ? "Successflully exported" : "エクスポートに成功しました。");
    
    [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:[[fileManager currentDirectoryPath] stringByAppendingString:@"/export-ios"]]];
}

- (IBAction)onMenuExportForAndroid:(id)sender {
    if (!create_package("")) {
        log_info("Export error.");
        return;
    }

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *exportPath = [NSString stringWithFormat:@"%@/export-android", [fileManager currentDirectoryPath]];
    [fileManager removeItemAtURL:[NSURL fileURLWithPath:exportPath] error:nil];
    if (![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:exportPath] withIntermediateDirectories:NO attributes:nil error:nil]) {
        log_warn("mkdir error (1).");
        return;
    }

    NSArray *appArray = @[@"app", @"gradle", @"gradlew", @"settings.gradle", @"build.gradle", @"gradle.properties", @"gradlew.bat"];
    for (NSString *sub in appArray) {
        if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/Contents/Resources/android-src/%@", [[NSBundle  mainBundle] bundlePath], sub]
                                  toPath:[NSString stringWithFormat:@"%@/%@", exportPath, sub]
                                   error:nil]) {
            log_warn("Copy error (1).");
            return;
        }
    }

    if (![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:[exportPath stringByAppendingString:@"/app/src/main/assets"]] withIntermediateDirectories:YES attributes:nil error:nil]) {
        log_warn("mkdir error (2).");
        return;
    }

    NSArray *subfolderArray = @[@"anime", @"bg", @"bgm", @"cg", @"ch", @"conf", @"cv", @"font", @"gui", @"mov", @"rule", @"se", @"txt", @"wms"];
    for (NSString *sub in subfolderArray) {
        if (![fileManager copyItemAtPath:[NSString stringWithFormat:@"%@/%@", [fileManager currentDirectoryPath], sub]
                                  toPath:[NSString stringWithFormat:@"%@/app/src/main/assets/%@", exportPath, sub]
                                   error:nil]) {
            log_warn("Copy error (2).");
            return;
        }
    }
 
    log_info(_isEnglish ? "Successflully exported" : "エクスポートに成功しました。");
    
    [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:[[fileManager currentDirectoryPath] stringByAppendingString:@"/export-android"]]];
}

// パッケージのエクスポートが押下されたイベント
- (IBAction)onMenuExportPackage:(id)sender {
    if (!create_package("")) {
        log_info("Export error.");
        return;
    }

    log_info(_isEnglish ? "Successflully exported" : "エクスポートに成功しました。");
    
    [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:[[NSFileManager defaultManager] currentDirectoryPath]]];
}

- (IBAction)onHelp:(id)sender {
    if (_isEnglish) {
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://suika2.com/en/doc/"]];
    } else {
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://suika2.com/wiki/?%E3%83%89%E3%82%AD%E3%83%A5%E3%83%A1%E3%83%B3%E3%83%88"]];
    }
}

//
// ウィンドウ/ビューの設定/取得
//

// ウィンドウのタイトルを設定する
- (void)setTitle:(NSString *)title {
    [self.view.window setTitle:title];
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
    [self.textFieldScriptName setStringValue:name];
}

// スクリプト名のテキストフィールドの値を取得する
- (NSString *)getScriptName {
    return [[self textFieldScriptName] stringValue];
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
    [[self textFieldVariables] setEditable:state];
}

// 変数の書き込みボタンの有効状態を設定する
- (void)enableVariableUpdateButton:(BOOL)state {
    [self.buttonUpdateVariables  setEnabled:state];
}

// 変数のテキストフィールドの値を設定する
- (void)setVariablesText:(NSString *)text {
    self.textFieldVariables.stringValue = text;
}

// 変数のテキストフィールドの値を取得する
- (NSString *)getVariablesText {
    return self.textFieldVariables.stringValue;
}

///
/// スクリプトのテキストビュー
///

// 実行行を設定する
- (void)setExecLine:(int)line {
    [self selectScriptLine:line];
    [self setTextColorForAllLines];
    
    // Scroll.
    [self scrollToCurrentLine];
}

- (void)scrollToCurrentLine {
    NSRange caretRange = NSMakeRange(_textViewScript.selectedRange.location, 0);
    NSLayoutManager *layoutManager = [_textViewScript layoutManager];
    NSRange glyphRange = [layoutManager glyphRangeForCharacterRange:caretRange actualCharacterRange:nil];
    NSRect glyphRect = [layoutManager boundingRectForGlyphRange:glyphRange inTextContainer:[_textViewScript textContainer]];
    [_textViewScript scrollRectToVisible:glyphRect];
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
    self.textViewScript.string = text;

    // 複数行の変更があったことを記録する
    _isRangedChange = TRUE;
}

// テキストビューの内容を元にスクリプトモデルを更新する
- (void)updateScriptModelFromText {
    // パースエラーをリセットして、最初のパースエラーで通知を行う
    dbg_reset_parse_error_count();

    // リッチエディットのテキストの内容でスクリプトの各行をアップデートする
    NSString *text = _textViewScript.string;
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
            NSColor *bgColor = self.isRunning ?
                [NSColor colorWithRed:0xff/255.0f green:0xc0/255.0f blue:0xc0/255.0f alpha:1.0f] :
                [NSColor colorWithRed:0xc0/255.0f green:0xc0/255.0f blue:0xff/255.0f alpha:1.0f];
            [self.textViewScript.textStorage addAttribute:NSBackgroundColorAttributeName value:bgColor range:lineRange];
        }

        // コメントを処理する
        if ([lineText characterAtIndex:0] == L'#') {
            // 行全体のテキスト色を変更する
            [self.textViewScript.textStorage
                addAttribute:NSForegroundColorAttributeName
                       value:[NSColor colorWithRed:0.5f green:0.5f blue:0.5f alpha:1.0f]
                       range:lineRange];
        }
        // ラベルを処理する
        else if ([lineText characterAtIndex:0] == L':') {
            // 行全体のテキスト色を変更する
            [self.textViewScript.textStorage
                addAttribute:NSForegroundColorAttributeName
                       value:[NSColor colorWithRed:1.0f green:0 blue:0 alpha:1.0f]
                       range:lineRange];
        }
        // エラー行を処理する
        else if ([lineText characterAtIndex:0] == L'!') {
            // 行全体のテキスト色を変更する
            [self.textViewScript.textStorage
                addAttribute:NSForegroundColorAttributeName
                       value:[NSColor colorWithRed:1.0f green:0 blue:0 alpha:1.0f]
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
                       value:[NSColor colorWithRed:0 green:0 blue:1.0f alpha:1.0f]
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
                               value:[NSColor colorWithRed:0xc0/255.0f green:0xf0/255.0f blue:0xc0/255.0f alpha:1.0f]
                               range:paramNameRange];

                    paramStart += spacePos;
                } while (paramStart < lineLen);
            }
        }
        
        startPos += [lineText length] + 1;
    }
}

- (void)onScriptShiftEnter {
    self.changedExecLine = [self scriptCursorLine];
    self.isExecLineChanged = YES;
    self.isNextPressed = YES;
}

- (void)onScriptRangedChange {
    _isRangedChange = YES;
}

- (void)onScriptChange {
    if (_isRangedChange) {
        [self updateScriptModelFromText];
        [self setTextColorForAllLines];
    }
}

- (int)scriptCursorLine {
    NSString *text = _textViewScript.string;
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
    NSString *text = _textViewScript.string;
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

        self.textFieldVariables.stringValue = text;
    }
}

@end


//
// Main HAL
//


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

    // アラートを表示する
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_INFO)]];
    NSString *text = [[NSString alloc] initWithUTF8String:buf];
    if (![text canBeConvertedToEncoding:NSUTF8StringEncoding])
        text = @"(invalid utf-8 string)";
    [alert setInformativeText:text];
    [alert runModal];

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

    // アラートを表示する
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_WARN)]];
    NSString *text = [[NSString alloc] initWithUTF8String:buf];
    if (![text canBeConvertedToEncoding:NSUTF8StringEncoding])
        text = @"(invalid utf-8 string)";
    [alert setInformativeText:text];
    [alert runModal];

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

    // アラートを表示する
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_ERROR)]];
    NSString *text = [[NSString alloc] initWithUTF8String:buf];
    if (![text canBeConvertedToEncoding:NSUTF8StringEncoding])
        text = @"(invalid utf-8 string)";
    [alert setInformativeText:text];
    [alert runModal];

    return true;
}

//
// セーブディレクトリを作成する
//
bool make_sav_dir(void)
{
    @autoreleasepool {
        NSString *basePath = [[NSFileManager defaultManager] currentDirectoryPath];
        NSString *savePath = [NSString stringWithFormat:@"%@/%s", basePath, SAVE_DIR];
        NSError *error;
        [[NSFileManager defaultManager] createDirectoryAtPath:savePath
                                  withIntermediateDirectories:NO
                                                   attributes:nil
                                                        error:&error];
        return true;
    }
}

//
// データファイルのディレクトリ名とファイル名を指定して有効なパスを取得する
//
char *make_valid_path(const char *dir, const char *fname)
{
    @autoreleasepool {
        NSString *basePath = [[NSFileManager defaultManager] currentDirectoryPath];
        NSString *filePath;
        if (dir != NULL) {
            if (fname != NULL)
                filePath = [NSString stringWithFormat:@"%@/%s/%s", basePath, dir, fname];
            else
                filePath = [NSString stringWithFormat:@"%@/%s", basePath, dir];
        } else {
            if (fname != NULL)
                filePath = [NSString stringWithFormat:@"%@/%s", basePath, fname];
            else
                filePath = basePath;
        }
        const char *cstr = [filePath UTF8String];
        char *ret = strdup(cstr);
        if (ret == NULL) {
            log_memory();
            return NULL;
        }
        return ret;
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
    // 続けるボタンを無効にする
    [theViewController setResumeButton:NO text:theViewController.isEnglish ? @"Continue" : @"続ける"];

    // 次へボタンを無効にする
    [theViewController setNextButton:NO text:theViewController.isEnglish ? @"Next" : @"次へ"];

    // 停止ボタンを無効にする
    [theViewController setStopButton:NO text:theViewController.isEnglish ? @"Stop" : @"停止"];

    // スクリプト選択ボタンを無効にする
    [theViewController enableOpenScriptButton:NO];

    // スクリプトのテキストボックスを有効にする
    [theViewController enableScriptTextView:NO];

    // 変数のテキストボックスを無効にする
    [theViewController enableVariableTextView:NO];

    // 変数の書き込みボタンを無効にする
    [theViewController enableVariableUpdateButton:NO];

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
    [theViewController setResumeButton:NO text:theViewController.isEnglish ? @"Continue" : @"続ける"];

    /* 次へボタンを無効にする */
    [theViewController setNextButton:NO text:theViewController.isEnglish ? @"Next" : @"次へ"];

    /* 停止ボタンを有効にする */
    [theViewController setStopButton:TRUE text:theViewController.isEnglish ? @"Stop" : @"停止"];

    // スクリプトを開くボタンを無効にする
    [theViewController enableOpenScriptButton:NO];

    // スクリプトのテキストボックスを有効にする
    [theViewController.textViewScript setEditable:YES];

    // 変数のテキストボックスを無効にする
    [theViewController enableVariableTextView:NO];

    // 変数の書き込みボタンを無効にする
    [theViewController enableVariableUpdateButton:NO];

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
    [theViewController setResumeButton:YES text:theViewController.isEnglish ? @"Resume" : @"続ける"];

    // 次へボタンを有効にする
    [theViewController setNextButton:YES text:theViewController.isEnglish ? @"Next" : @"次へ"];

    // 停止ボタンを無効にする
    [theViewController setStopButton:NO text:theViewController.isEnglish ? @"Pause" : @"停止"];

    // スクリプト選択ボタンを有効にする
    [theViewController enableOpenScriptButton:YES];

    // スクリプトのテキストボックスを有効にする
    [theViewController enableScriptTextView:YES];

    // 変数のテキストボックスを有効にする
    [theViewController enableVariableTextView:YES];

    // 変数の書き込みボタンを有効にする
    [theViewController enableVariableUpdateButton:YES];

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
