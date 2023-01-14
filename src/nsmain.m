// -*- tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changed]
 *  - 2016/06/15 Created.
 *  - 2020/06/11 Add debugger.
 */

#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>
#import <sys/time.h>

#import <wchar.h>

#import "suika.h"
#import "nsmain.h"
#import "uimsg.h"
#import "aunit.h"
#import "glrender.h"

#ifdef SSE_VERSIONING
#import "x86.h"
#endif

#ifdef USE_DEBUGGER
#import "nsdebug.h"
#endif

#define GL_SILENCE_DEPRECATION
#import <OpenGL/gl3.h>

// ウィンドウ
static NSWindow *theWindow;

// ビュー
static SuikaView *theView;

// フルスクリーン状態
static BOOL isFullScreen;

// ログファイル
#ifndef USE_DEBUGGER
static FILE *logFp;
#endif

// 動画プレーヤー
static AVPlayer *player;

// 動画プレーヤーのレイヤー
static AVPlayerLayer *playerLayer;

// 動画再生状態
static BOOL isMoviePlaying;

// 前方参照
static BOOL initWindow(void);
static void cleanupWindow(void);
static NSString *NSStringFromWcs(const wchar_t *wcs);
#ifndef USE_DEBUGGER
static BOOL openLog(void);
static void closeLog(void);
#endif

//
// メインビュー
//

@interface SuikaView ()
- (IBAction)onQuit:(id)sender;
@end

@implementation SuikaView

// フルスクリーン時の拡大率(マウス座標の計算に利用)
float screenScale;

// フルスクリーンになる前のウィンドウ位置とサイズ
NSRect savedFrame;

// 終了処理の必要があるか
BOOL isFinished;

// コントロールキーが押下されているか
BOOL isControlPressed;

// 描画の準備ができているか
BOOL isRedrawPrepared;

// 動画再生を利用する
+ (Class)layerClass {
    return AVPlayerLayer.class;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // メニューのXibをロードする
    NSBundle *bundle = [NSBundle mainBundle];
    NSArray *objects = [NSArray new];
    [bundle loadNibNamed:@"MainMenu"
                   owner:theView
         topLevelObjects:&objects];

    // メニューのタイトルを変更す
    NSMenu *menu = [[[NSApp mainMenu] itemAtIndex:0] submenu];
    [menu setTitle:[[NSString alloc] initWithUTF8String:conf_window_title]];
}

// ビューが作成されるときに呼び出される
- (id)initWithFrame:(NSRect)frame {
    // OpenGLコンテキストを作成する
    NSOpenGLPixelFormatAttribute pixelFormatAttributes[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFANoRecovery,
        0
    };
    NSOpenGLPixelFormat *pixelFormat =
        [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
    self = [super initWithFrame:frame pixelFormat:pixelFormat];
    [[self openGLContext] makeCurrentContext];

    // VSYNC待ちを有効にする
    // GLint vsync = GL_TRUE;
    // [[self openGLContext] setValues:&vsync forParameter:NSOpenGLCPSwapInterval];

    // 初回描画におけるちらつきを抑える
    // (これがないとAMD Radeonでは一瞬赤になる)
    if (conf_window_white)
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    else
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // スクリーン拡大率を設定する
    screenScale = 1.0f;

    // OpenGLの初期化を行う
    if (!init_opengl()) {
        isFinished = YES;
        log_error("Failed to initialize OpenGL.");
        return self;
    }

    // アプリケーション本体の初期化を行う
    if (!on_event_init()) {
        isFinished = YES;
        return self;
    }

    return self;
}

// タイマコールバック
- (void)timerFired:(NSTimer *)timer {
    // 終了する場合
    if (isFinished) {
        // タイマを停止する
        [timer invalidate];

        // イベントループを終了する
        [NSApp stop:nil];

        // Magic: 空のイベントを1つポストしてイベントループから抜ける
        NSEvent* event =
            [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                               location:NSMakePoint(0, 0)
                          modifierFlags:0
                              timestamp:0
                           windowNumber:0
                                context:nil
                                subtype:0
                                  data1:0
                                  data2:0];
        [NSApp postEvent:event atStart:YES];
        return;
    }

    // OpenGLの描画を開始する
    if (!isMoviePlaying)
        opengl_start_rendering();

    // フレーム描画イベントを実行する
    int x = 0, y = 0, w = 0, h = 0;
    if (!on_event_frame(&x, &y, &w, &h))
        isFinished = YES;

    // OpenGLの描画を終了する
    if (!isMoviePlaying) {
        opengl_end_rendering();
        isRedrawPrepared = YES;

        // drawRectの呼び出しを予約する
        [self setNeedsDisplay:YES];
    }
}

// 描画イベント
- (void)drawRect:(NSRect)rect {
    [super drawRect:rect];
    if (isFinished)
        return;
    if (isMoviePlaying)
        return;

    if (!isRedrawPrepared) {
        if (conf_window_white)
            [[NSColor whiteColor] setFill];
        else
            [[NSColor blackColor] setFill];
        NSRectFill(rect);
        return;
    }

    [[self openGLContext] flushBuffer];

    isRedrawPrepared = NO;
}

// マウス押下イベント
- (void)mouseDown:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= screenScale;
    pos.y *= screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;
        
	on_event_mouse_press(MOUSE_LEFT, (int)pos.x,
                         conf_window_height - (int)pos.y);
}

// マウス解放イベント
- (void)mouseUp:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= screenScale;
    pos.y *= screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

	on_event_mouse_release(MOUSE_LEFT, (int)pos.x,
                           conf_window_height - (int)pos.y);
}

// マウス右ボタン押下イベント
- (void)rightMouseDown:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= screenScale;
    pos.y *= screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;
        
	on_event_mouse_press(MOUSE_RIGHT, (int)pos.x,
                         conf_window_height - (int)pos.y);
}

// マウス右ボタン解放イベント
- (void)rightMouseUp:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= screenScale;
    pos.y *= screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

	on_event_mouse_release(MOUSE_RIGHT, (int)pos.x,
                           conf_window_height - (int)pos.y);
}

// マウス移動イベント
- (void)mouseMoved:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= screenScale;
    pos.y *= screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

	on_event_mouse_move((int)pos.x, conf_window_height - (int)pos.y);
}

// マウスドラッグイベント
- (void)mouseDragged:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= screenScale;
    pos.y *= screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

	on_event_mouse_move((int)pos.x, conf_window_height - (int)pos.y);
}

// マウスホイールイベント
- (void)scrollWheel:(NSEvent *)theEvent {
    if ([theEvent deltaY] > 0) {
        on_event_key_press(KEY_UP);
        on_event_key_release(KEY_UP);
    } else if ([theEvent deltaY] < 0) {
        on_event_key_press(KEY_DOWN);
        on_event_key_release(KEY_DOWN);
    }
}

// キーボード修飾変化イベント
- (void)flagsChanged:(NSEvent *)theEvent {
    // Controlキーの状態を取得する
    BOOL bit = ([theEvent modifierFlags] & NSEventModifierFlagControl) ==
    NSEventModifierFlagControl;
    
    // Controlキーの状態が変化した場合は通知する
    if (!isControlPressed && bit) {
        isControlPressed = YES;
        on_event_key_press(KEY_CONTROL);
    } else if (isControlPressed && !bit) {
        isControlPressed = NO;
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
    case KC_SPACE:
        return KEY_SPACE;
    case KC_RETURN:
        return KEY_RETURN;
    case KC_UP:
        return KEY_UP;
    case KC_DOWN:
        return KEY_DOWN;
    case KC_ESCAPE:
        return KEY_ESCAPE;
    case KC_C:
        return KEY_C;
    }
    return -1;
}

// フルスクリーンになる前に呼び出される
- (NSSize)window:(NSWindow *)window
willUseFullScreenContentSize:(NSSize)proposedSize {
    UNUSED_PARAMETER(window);

    // ゲーム画面のアスペクト比を求める
    float aspect = (float)conf_window_height / (float)conf_window_width;

    // 横幅優先で高さを仮決めする
    float width = proposedSize.width;
    float height = width * aspect;
    screenScale = (float)conf_window_width / width;

    // 高さが足りなければ、縦幅優先で横幅を決める
    if(height > proposedSize.height) {
        height = proposedSize.height;
        width = proposedSize.height / aspect;
        screenScale = (float)conf_window_height / height;
    }

    // 動画プレーヤレイヤのサイズを更新する
    if(playerLayer != nil)
        [playerLayer setFrame:NSMakeRect(0, 0, width, height)];

    // スクリーンサイズを返す
    return NSMakeSize(width, height);
}

// フルスクリーンになるとき呼び出される
- (void)windowWillEnterFullScreen:(NSNotification *)notification {
    isFullScreen = YES;

    // ウィンドウサイズを保存する
    savedFrame = [theWindow frame];
}

// フルスクリーンから戻るときに呼び出される
- (void)windowWillExitFullScreen:(NSNotification *)notification {
    isFullScreen = NO;

    // 動画プレーヤレイヤのサイズを元に戻す
    if(playerLayer != nil) {
        [playerLayer setFrame:NSMakeRect(0, 0,
                                         savedFrame.size.width,
                                         savedFrame.size.height)];
    }

    [theWindow setFrame:savedFrame display:YES animate:NO];
    screenScale = 1.0f;
}

// 閉じるボタンイベント
- (BOOL)windowShouldClose:(id)sender {
    UNUSED_PARAMETER(sender);

#ifdef USE_DEBUGGER
    return YES;
#else
    @autoreleasepool {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSStringFromWcs(get_ui_message(UIMSG_YES))];
        [alert addButtonWithTitle:NSStringFromWcs(get_ui_message(UIMSG_NO))];
        [alert setMessageText:NSStringFromWcs(get_ui_message(UIMSG_EXIT))];
        [alert setAlertStyle:NSAlertStyleWarning];
        if ([alert runModal] == NSAlertFirstButtonReturn)
            return YES;
        else
            return NO;
    }
#endif
}

// ウィンドウが閉じられるイベント
- (void)windowWillClose:(NSNotification *)notification {
    // メインループから抜ける
    [NSApp stop:nil];
}

// 最後のウィンドウが閉じられたあと終了するか
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app {
    UNUSED_PARAMETER(app);
    return YES;
}

// Quitが選択されたか
- (IBAction)onQuit:(id)sender {
    if ([self windowShouldClose:sender]) {
        // メインループから抜ける
        [NSApp stop:nil];
    }
}

// First Responderとなるか
- (BOOL)acceptsFirstResponder {
    return YES;
}

// 動画再生が完了したとき通知される
- (void)onPlayEnd:(NSNotification *)notification {
    [player replaceCurrentItemWithPlayerItem:nil];
    isMoviePlaying = NO;
}
@end

//
// メイン
//
int main()
{
#ifdef SSE_VERSIONING
	// ベクトル命令の対応を確認する
    x86_check_cpuid_flags();
#endif

    @autoreleasepool {
        // ロケールを初期化する
        init_locale_code();

#ifdef USE_DEBUGGER
        // スタートアップファイル/ラインを取得する
        if (!getStartupPosition())
            return 1;
#endif

        // パッケージの初期化処理を行う
        if (init_file()) {
            // コンフィグの初期化処理を行う
            if (init_conf()) {
                // オーディオユニットの初期化処理を行う
                if (init_aunit()) {
                    // ウィンドウを作成する
                    if (initWindow()) {
#ifdef USE_DEBUGGER
                        // デバッグウィンドウを作成する
                        if (initDebugWindow()) {
#else
                        {
#endif
                            // メインループを実行する
                            [NSApp activateIgnoringOtherApps:YES];
                            [NSApp run];

                            // アプリケーション本体の終了処理を行う
                            on_event_cleanup();

                            // OpenGLの終了処理を行う
                            cleanup_opengl();
                        }
                    }

                    // ウィンドウの終了処理を行う
                    cleanupWindow();
                }

                // オーディオユニットの終了処理を行う
                cleanup_aunit();
            }

            // コンフィグの終了処理を行う
            cleanup_conf();
        }

        // パッケージの終了処理を行う
        cleanup_file();

#ifndef USE_DEBUGGER
        // ログをクローズする
        closeLog();
#endif
    }

    [NSApp terminate:nil];
	return 0;
}

#ifndef USE_DEBUGGER
// ログをオープンする
static BOOL openLog(void)
{
    const char *cpath;

    // すでにオープン済みの場合、成功とする
    if (logFp != NULL)
        return TRUE;

    if (conf_release && conf_window_title != NULL) {
        NSString *path = NSHomeDirectory();
        path = [path stringByAppendingString:@"/Library/Application Support/"];
        path = [path stringByAppendingString:[[NSString alloc] initWithUTF8String:conf_window_title]];
        [[NSFileManager defaultManager] createDirectoryAtPath:path
                                  withIntermediateDirectories:YES
                                                   attributes:nil
                                                        error:NULL];
        path = [path stringByAppendingString:@"/"];
        path = [path stringByAppendingString:[[NSString alloc] initWithUTF8String:LOG_FILE]];
        cpath = [path UTF8String];
    } else {
        // .appバンドルのパスを取得する
        NSString *bundlePath = [[NSBundle mainBundle] bundlePath];

        // .appバンドルの1つ上のディレクトリのパスを取得する
        NSString *basePath = [bundlePath stringByDeletingLastPathComponent];

        // ログのパスを生成する
        cpath = [[NSString stringWithFormat:@"%@/%s", basePath, LOG_FILE] UTF8String];
    }

    // ログをオープンする
    logFp = fopen(cpath, "w");
    if (logFp == NULL) {
        // 失敗
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Error"];
        [alert setInformativeText:@"Cannot open log file."];
        [alert runModal];
        return NO;
    }

    // 成功
    return YES;
}

// ログをクローズする
static void closeLog(void)
{
    // ログをクローズする
    if (logFp != NULL)
        fclose(logFp);
}
#endif

// ウィンドウの初期化処理を行う
static BOOL initWindow(void)
{
    // アプリケーションの初期化処理を行う
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    // メインスクリーンの位置とサイズを取得する
    NSRect sr = [[NSScreen mainScreen] visibleFrame];

    // ウィンドウの座標を計算する
    NSRect cr = NSMakeRect(sr.origin.x +
                           (sr.size.width - conf_window_width) / 2,
                           sr.origin.y +
                           (sr.size.height - conf_window_height) / 2,
                           conf_window_width,
                           conf_window_height);

    // ウィンドウを作成する
    theWindow = [[NSWindow alloc]
                     initWithContentRect:cr
                               styleMask:NSWindowStyleMaskTitled |
                                         NSWindowStyleMaskClosable  |
                                         NSWindowStyleMaskMiniaturizable
                                 backing:NSBackingStoreBuffered
                                   defer:NO];
#ifndef USE_DEBUGGER
    if (!conf_window_fullscreen_disable) {
        [theWindow setCollectionBehavior:
                       [theWindow collectionBehavior] |
                   NSWindowCollectionBehaviorFullScreenPrimary];
    }
#endif
    [theWindow setTitle:[[NSString alloc]
                            initWithUTF8String:conf_window_title]];
    [theWindow makeKeyAndOrderFront:nil];
    [theWindow setAcceptsMouseMovedEvents:YES];

    // ビューを作成する
    theView = [[SuikaView alloc] init];
    [theWindow setContentView:theView];
    [theWindow makeFirstResponder:theView];

    // デリゲートを設定する
    [NSApp setDelegate:theView];
    [theWindow setDelegate:theView];

    // タイマをセットする
    [NSTimer scheduledTimerWithTimeInterval:1.0/30.0
                                     target:theView
                                   selector:@selector(timerFired:)
                                   userInfo:nil
                                    repeats:YES];

    // Hack: コマンドラインから起動された際にメニューを有効にする
    ProcessSerialNumber psn = {0, kCurrentProcess};
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);

    return YES;
}

// ウィンドウの終了処理を行う
static void cleanupWindow(void)
{
    // TODO: destroy theView and theWindow
}

// ワイド文字列をNSStringに変換する
static NSString *NSStringFromWcs(const wchar_t *wcs)
{
    return [[NSString alloc] initWithBytes:wcs
                                    length:wcslen(wcs) * sizeof(*wcs)
                                  encoding:NSUTF32LittleEndianStringEncoding];
}

//
// platform.hの実装
//

//
// セーブディレクトリを作成する
//
bool make_sav_dir(void)
{
    @autoreleasepool {
        if (conf_release) {
            NSString *path = NSHomeDirectory();
            path = [path stringByAppendingString:@"/Library/Application Support/"];
            path = [path stringByAppendingString:[[NSString alloc] initWithUTF8String:conf_window_title]];
            path = [path stringByAppendingString:@"/"];
            path = [path stringByAppendingString:[[NSString alloc] initWithUTF8String:SAVE_DIR]];
            [[NSFileManager defaultManager] createDirectoryAtPath:path
                                      withIntermediateDirectories:YES
                                                       attributes:nil
                                                            error:NULL];
        } else {
            // .appバンドルのパスを取得する
            NSString *bundlePath = [[NSBundle mainBundle] bundlePath];

            // .appバンドルの1つ上のディレクトリのパスを取得する
            NSString *basePath = [bundlePath stringByDeletingLastPathComponent];

            // savディレクトリのパスを作成する
            NSString *savePath = [NSString stringWithFormat:@"%@/%s", basePath,
                                           SAVE_DIR];

            // savディレクトリを作成する
            NSError *error;
            [[NSFileManager defaultManager] createDirectoryAtPath:savePath
                                      withIntermediateDirectories:NO
                                                       attributes:nil
                                                            error:&error];
        }
    }
	return true;
}

//
// データファイルのディレクトリ名とファイル名を指定して有効なパスを取得する
//
char *make_valid_path(const char *dir, const char *fname)
{
    char *ret;

    assert(fname != NULL);

    @autoreleasepool {
        if (conf_release && dir != NULL && strcmp(dir, SAVE_DIR) == 0) {
            NSString *path = NSHomeDirectory();
            path = [path stringByAppendingString:@"/Library/Application Support/"];
            path = [path stringByAppendingString:[[NSString alloc] initWithUTF8String:conf_window_title]];
            path = [path stringByAppendingString:@"/"];
            path = [path stringByAppendingString:[[NSString alloc] initWithUTF8String:dir]];
            path = [path stringByAppendingString:@"/"];
            path = [path stringByAppendingString:[[NSString alloc] initWithUTF8String:fname]];
            ret = strdup([path UTF8String]);
        } else {
            // .appバンドルのパスを取得する
            NSString *bundlePath = [[NSBundle mainBundle] bundlePath];

            // .appバンドルの1つ上のディレクトリのパスを取得する
            NSString *basePath = [bundlePath stringByDeletingLastPathComponent];

            // ファイルのパスを作成する
            NSString *filePath;
            if (dir != NULL)
                filePath = [NSString stringWithFormat:@"%@/%s/%s", basePath, dir,
                                     fname];
            else
                filePath = [NSString stringWithFormat:@"%@/%s", basePath, fname];

            const char *cstr = [filePath UTF8String];
            ret = strdup(cstr);
        }
    }

    if (ret == NULL) {
        log_memory();
        return NULL;
    }
    return ret;
}

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
    [alert setMessageText:NSStringFromWcs(get_ui_message(UIMSG_INFO))];
    [alert setInformativeText:[[NSString alloc] initWithUTF8String:buf]];
    [alert runModal];

#ifndef USE_DEBUGGER
    // ログファイルに出力する
    if (!openLog())
        return false;
    if (logFp != NULL) {
        fprintf(stderr, "%s", buf);
        fprintf(logFp, "%s", buf);
        fflush(logFp);
        if (ferror(logFp))
            return false;
    }
#endif

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
    [alert setMessageText:NSStringFromWcs(get_ui_message(UIMSG_WARN))];
    [alert setInformativeText:[[NSString alloc] initWithUTF8String:buf]];
    [alert runModal];

#ifndef USE_DEBUGGER
    // ログファイルに出力する
    if (!openLog())
        return false;
    if (logFp != NULL) {
        fprintf(stderr, "%s", buf);
        fprintf(logFp, "%s", buf);
        fflush(logFp);
        if (ferror(logFp))
            return false;
    }
#endif

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
    [alert setMessageText:NSStringFromWcs(get_ui_message(UIMSG_ERROR))];
    [alert setInformativeText:[[NSString alloc] initWithUTF8String:buf]];
    [alert runModal];

#ifndef USE_DEBUGGER
    // ログファイルに出力する
    if (!openLog())
        return false;
    assert(logFp != NULL);
    fprintf(stderr, "%s", buf);
    fprintf(logFp, "%s", buf);
    fflush(logFp);
    if (ferror(logFp))
        return false;
#endif

    return true;
}

//
// UTF-8のメッセージをネイティブの文字コードに変換する
//  - 変換の必要がないので引数をそのまま返す
//
const char *conv_utf8_to_native(const char *utf8_message)
{
	assert(utf8_message != NULL);
	return utf8_message;
}

//
// GPUを使うか調べる
//
bool is_gpu_accelerated(void)
{
    return true;
}

//
// OpenGLが有効か調べる
//
bool is_opengl_enabled(void)
{
	return true;
}

//
// テクスチャをロックする
//
bool lock_texture(int width, int height, pixel_t *pixels,
                  pixel_t **locked_pixels, void **texture)
{
    return opengl_lock_texture(width, height, pixels, locked_pixels, texture);;
}

//
// テクスチャをアンロックする
//
void unlock_texture(int width, int height, pixel_t *pixels,
                    pixel_t **locked_pixels, void **texture)
{
    opengl_unlock_texture(width, height, pixels, locked_pixels, texture);
}

//
// テクスチャを破棄する
//
void destroy_texture(void *texture)
{
    opengl_destroy_texture(texture);
}

//
// イメージをレンダリングする
//
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
                  int width, int height, int src_left, int src_top, int alpha,
                  int bt)
{
    opengl_render_image(dst_left, dst_top, src_image, width, height, src_left,
                        src_top, alpha, bt);
}

//
// 画面にイメージをルール付きでレンダリングする
//
void render_image_rule(struct image * RESTRICT src_img,
                       struct image * RESTRICT template_img,
                       int threshold)
{
    opengl_render_image_rule(src_img, template_img, threshold);
}

//
// 画面にイメージをルール付き(メルト)でレンダリングする
//
void render_image_melt(struct image * RESTRICT src_img,
                       struct image * RESTRICT template_img,
                       int threshold)
{
    opengl_render_image_melt(src_img, template_img, threshold);
}

//
// タイマをリセットする
//
void reset_stop_watch(stop_watch_t *t)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    *t = (stop_watch_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

//
// タイマのラップをミリ秒単位で取得する
//
int get_stop_watch_lap(stop_watch_t *t)
{
    struct timeval tv;
    stop_watch_t end;

    gettimeofday(&tv, NULL);

    end = (stop_watch_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);

    if (end < *t) {
        reset_stop_watch(t);
            return 0;
    }

    return (int)(end - *t);
}

//
// 終了ダイアログを表示する
//
bool exit_dialog(void)
{
    @autoreleasepool {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSStringFromWcs(get_ui_message(UIMSG_YES))];
        [alert addButtonWithTitle:NSStringFromWcs(get_ui_message(UIMSG_NO))];
        [alert setMessageText:NSStringFromWcs(get_ui_message(UIMSG_EXIT))];
        [alert setAlertStyle:NSAlertStyleWarning];
        if ([alert runModal] == NSAlertFirstButtonReturn)
            return true;
        return false;
    }
}    

//
// タイトルに戻るダイアログを表示する
//
bool title_dialog(void)
{
    @autoreleasepool {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSStringFromWcs(get_ui_message(UIMSG_YES))];
        [alert addButtonWithTitle:NSStringFromWcs(get_ui_message(UIMSG_NO))];
        [alert setMessageText:NSStringFromWcs(get_ui_message(UIMSG_TITLE))];
        [alert setAlertStyle:NSAlertStyleWarning];
        if ([alert runModal] == NSAlertFirstButtonReturn)
            return true;
        return false;
    }
}

//
// 削除ダイアログを表示する
//
bool delete_dialog(void)
{
    @autoreleasepool {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSStringFromWcs(get_ui_message(UIMSG_YES))];
        [alert addButtonWithTitle:NSStringFromWcs(get_ui_message(UIMSG_NO))];
        [alert setMessageText:NSStringFromWcs(get_ui_message(UIMSG_DELETE))];
        [alert setAlertStyle:NSAlertStyleWarning];
        if ([alert runModal] == NSAlertFirstButtonReturn)
            return true;
        return false;
    }
}

//
// 上書きダイアログを表示する
//
bool overwrite_dialog(void)
{
    @autoreleasepool {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSStringFromWcs(get_ui_message(UIMSG_YES))];
        [alert addButtonWithTitle:NSStringFromWcs(get_ui_message(UIMSG_NO))];
        [alert setMessageText:NSStringFromWcs(get_ui_message(UIMSG_OVERWRITE))];
        [alert setAlertStyle:NSAlertStyleWarning];
        if ([alert runModal] == NSAlertFirstButtonReturn)
            return true;
        return false;
    }
}

//
// 初期設定ダイアログを表示する
//
bool default_dialog(void)
{
    @autoreleasepool {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSStringFromWcs(get_ui_message(UIMSG_YES))];
        [alert addButtonWithTitle:NSStringFromWcs(get_ui_message(UIMSG_NO))];
        [alert setMessageText:NSStringFromWcs(get_ui_message(UIMSG_DEFAULT))];
        [alert setAlertStyle:NSAlertStyleWarning];
        if ([alert runModal] == NSAlertFirstButtonReturn)
            return true;
        return false;
    }
}

//
// ビデオを再生する
//
bool play_video(const char *fname, bool is_skippable)
{
    // OpenGLのレンダリングを完了させる
    opengl_end_rendering();

    // パスを生成する
    char *path = make_valid_path(MOV_DIR, fname);
    assert(path != NULL);
    NSString *nsPath = [[NSString alloc] initWithUTF8String:path];
    free(path);
    nsPath = [nsPath stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLPathAllowedCharacterSet]];
    NSURL *url = [NSURL URLWithString:[@"file://" stringByAppendingString:nsPath]];

    // プレーヤーを作成する
    AVPlayerItem *playerItem = [[AVPlayerItem alloc] initWithURL:url];
    player = [[AVPlayer alloc] initWithPlayerItem:playerItem];

    // プレーヤーのレイヤーを作成する
    [theView setWantsLayer:YES];
    playerLayer = [AVPlayerLayer playerLayerWithPlayer:player];
    [playerLayer setFrame:theView.bounds];
    [theView.layer addSublayer:playerLayer];

    // 再生を開始する
    [player play];

    // 再生終了の通知を送るようにする
    [NSNotificationCenter.defaultCenter addObserver:theView
                                           selector:@selector(onPlayEnd:)
                                               name:AVPlayerItemDidPlayToEndTimeNotification
                                             object:playerItem];

    isMoviePlaying = YES;
    return true;
}

//
// ビデオを停止する
//
void stop_video(void)
{
    if (player != nil)
        [player replaceCurrentItemWithPlayerItem:nil];
    isMoviePlaying = NO;
    player = nil;
    playerLayer = nil;

    // OpenGLのレンダリングを開始する
    opengl_start_rendering();
}

//
// ビデオが再生中か調べる
//
bool is_video_playing(void)
{
    return isMoviePlaying == YES;
}

//
// ウィンドウタイトルを更新する
//
void update_window_title(void)
{
    @autoreleasepool {
        // ウィンドウタイトルを取得する
        NSString *windowTitle = [[NSString alloc]
                                    initWithUTF8String:conf_window_title];

        // 章タイトルを取得する
        NSString *chapterTitle = [[NSString alloc]
                                     initWithUTF8String:get_chapter_name()];

        // セパレータを取得する
        NSString *sep;
        if (conf_window_title_separator == NULL) {
            sep = @" ";
        } else {
            sep = [[NSString alloc] initWithUTF8String:
                                        conf_window_title_separator];
        }

        // タイトルを連結する
        NSString *s = [windowTitle stringByAppendingString:sep];
        s = [s stringByAppendingString:chapterTitle];

        // ウィンドウのタイトルを設定する
        [theWindow setTitle:s];
    }
}

//
// フルスクリーンモードがサポートされるか調べる
//
bool is_full_screen_supported(void)
{
    return true;
}

//
// フルスクリーンモードであるか調べる
//
bool is_full_screen_mode(void)
{
    return isFullScreen;
}

//
// フルスクリーンモードを開始する
//
void enter_full_screen_mode(void)
{
    if (!isFullScreen)
        [theWindow toggleFullScreen:theView];
}

///
// フルスクリーンモードを終了する
//
void leave_full_screen_mode(void)
{
    if (isFullScreen)
        [theWindow toggleFullScreen:theView];
}

//
// システムのロケールを取得する
//
const char *get_system_locale(void)
{
    NSString *language = [[NSLocale preferredLanguages] objectAtIndex:0];
    if ([language hasPrefix:@"ja"])
        return "ja";
    if ([language hasPrefix:@"en"])
        return "en";
    if ([language hasPrefix:@"fr"])
        return "fr";
    if ([language hasPrefix:@"de"])
        return "de";
    if ([language hasPrefix:@"es"])
        return "es";
    if ([language hasPrefix:@"it"])
        return "it";
    if ([language hasPrefix:@"el"])
        return "el";
    if ([language hasPrefix:@"ru"])
        return "ru";
    if ([language hasPrefix:@"zh-Hans"])
        return "zh";
    if ([language hasPrefix:@"zh-Hant"])
		return "tw";
	return "other";
}
