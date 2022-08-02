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
#import <sys/time.h>

#import "suika.h"
#import "nsmain.h"
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

// ログファイル
#ifndef USE_DEBUGGER
static FILE *logFp;
#endif

// 前方参照
static BOOL initWindow(void);
static void cleanupWindow(void);
#ifndef USE_DEBUGGER
static BOOL openLog(void);
static void closeLog(void);
#endif

//
// ビュー
//

@interface SuikaView ()
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

// ビューが作成されるときに呼び出される
- (id)initWithFrame:(NSRect)frame {
    // メニューのタイトルを変更する https://stackoverflow.com/questions/4965466/set-titles-of-items-in-my-apps-main-menu
    NSMenu *menu = [[[NSApp mainMenu] itemAtIndex:0] submenu];
    NSString *title = [[NSString alloc] initWithUTF8String:
#ifdef USE_DEBUGGER
        "Suika2 Pro for Creators"
#else
        conf_window_title
#endif
    ];
    [menu setTitle:[title stringByAppendingString:@"\x1b"]];

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
    GLint vsync = GL_TRUE;
    [[self openGLContext] setValues:&vsync forParameter:NSOpenGLCPSwapInterval];

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
    opengl_start_rendering();

    // フレーム描画イベントを実行する
    int x = 0, y = 0, w = 0, h = 0;
    if (!on_event_frame(&x, &y, &w, &h))
        isFinished = YES;

    // OpenGLの描画を終了する
    opengl_end_rendering();

    // drawRectの呼び出しを予約する
    [self setNeedsDisplay:YES];
}

// 描画イベント
- (void)drawRect:(NSRect)rect {
    [super drawRect:rect];

    if (isFinished)
        return;

    [[self openGLContext] flushBuffer];
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
    BOOL bit = ([theEvent modifierFlags] & NSControlKeyMask) ==
        NSControlKeyMask;
    
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

    // スクリーンサイズを返す
    return NSMakeSize(width, height);
}

// フルスクリーンから戻るときに呼び出される
- (void)windowWillEnterFullScreen:(NSNotification *)notification {
    // ウィンドウサイズを保存する
    savedFrame = [theWindow frame];
}

// フルスクリーンから戻るときに呼び出される
- (void)windowWillExitFullScreen:(NSNotification *)notification {
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
        [alert addButtonWithTitle:!conf_i18n ? @"はい" : @"Yes"];
        [alert addButtonWithTitle:!conf_i18n ? @"いいえ" : @"No"];
        [alert setMessageText:[[NSString alloc] initWithUTF8String:
                                                    conf_ui_msg_quit]];
        [alert setAlertStyle:NSWarningAlertStyle];
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

// First Responderとなるか
- (BOOL)acceptsFirstResponder {
    return YES;
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
                        if (initDebugWindow())
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
    // すでにオープン済みの場合、成功とする
    if (logFp != NULL)
        return TRUE;

    // .appバンドルのパスを取得する
    NSString *bundlePath = [[NSBundle mainBundle] bundlePath];

    // .appバンドルの1つ上のディレクトリのパスを取得する
    NSString *basePath = [bundlePath stringByDeletingLastPathComponent];

    // ログのパスを生成する
    const char *path = [[NSString stringWithFormat:@"%@/%s", basePath,
                                  LOG_FILE] UTF8String];

    // ログをオープンする
    logFp = fopen(path, "w");
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

#ifndef USE_DEBUGGER
    // メニューバーを作成する
    NSMenu *menuBar = [NSMenu new];
    NSMenuItem *appMenuItem = [NSMenuItem new];
    [menuBar addItem:appMenuItem];
    [NSApp setMainMenu:menuBar];

    // アプリケーションのメニューを作成する
    //  - 最初のタイマイベントでアプリケーション名を変更する
    id appMenu = [NSMenu new];
    id quitMenuItem = [[NSMenuItem alloc]
                          initWithTitle:!conf_i18n ? @"終了する" : @"Quit"
                                 action:@selector(performClose:)
                          keyEquivalent:@"q"];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];
#endif

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
                               styleMask:NSTitledWindowMask |
                                         NSClosableWindowMask |
                                         NSMiniaturizableWindowMask
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

//
// platform.hの実装
//

//
// セーブディレクトリを作成する
//
bool make_sav_dir(void)
{
    @autoreleasepool {
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
	return true;
}

//
// データファイルのディレクトリ名とファイル名を指定して有効なパスを取得する
//
char *make_valid_path(const char *dir, const char *fname)
{
    char *ret;

    @autoreleasepool {
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
    [alert setMessageText:!conf_i18n ? @"情報" : @"Information"];
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
    [alert setMessageText:!conf_i18n ? @"情報" : @"Information"];
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
    [alert setMessageText:!conf_i18n ? @"エラー" : @"Error"];
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
// 画面にイメージをルールつきでレンダリングする
//
void render_image_rule(struct image * RESTRICT src_img,
                       struct image * RESTRICT template_img,
                       int threshold)
{
    opengl_render_image_rule(src_img, template_img, threshold);
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
        [alert addButtonWithTitle:!conf_i18n ? @"はい" : @"Yes"];
        [alert addButtonWithTitle:!conf_i18n ? @"いいえ" : @"No"];
        [alert setMessageText:
                   [[NSString alloc] initWithUTF8String:conf_ui_msg_quit]];
        [alert setAlertStyle:NSWarningAlertStyle];
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
        [alert addButtonWithTitle:!conf_i18n ? @"はい" : @"Yes"];
        [alert addButtonWithTitle:!conf_i18n ? @"いいえ" : @"No"];
        [alert setMessageText:
                   [[NSString alloc] initWithUTF8String:conf_ui_msg_title]];
        [alert setAlertStyle:NSWarningAlertStyle];
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
        [alert addButtonWithTitle:!conf_i18n ? @"はい" : @"Yes"];
        [alert addButtonWithTitle:!conf_i18n ? @"いいえ" : @"No"];
        [alert setMessageText:
                   [[NSString alloc] initWithUTF8String:conf_ui_msg_delete]];
        [alert setAlertStyle:NSWarningAlertStyle];
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
	// stub
	return true;
}

//
// ビデオを停止する
//
void stop_video(void)
{
    // stub
}

//
// ビデオが再生中か調べる
//
bool is_video_playing(void)
{
    // stub
    return false;
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
