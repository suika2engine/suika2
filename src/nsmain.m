// -*- tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changed]
 *  - 2016/06/15 Created.
 */

#import <Cocoa/Cocoa.h>

#import "sys/time.h"

#import "suika.h"
#import "aunit.h"

#ifdef SSE_VERSIONING
#import "x86.h"
#endif

//
// キーコード
//
enum {
    KC_SPACE = 49,
    KC_RETURN = 36,
    KC_UP = 126,
    KC_DOWN = 125,
};

//
// 背景イメージ
//
static struct image *backImage;

//
// 背景イメージのピクセル
//
static unsigned char *backImagePixels;

//
// コントロールキーの状態
//
static BOOL isControlPressed;

//
// ログファイル
//
FILE *logFp;

//
// ウィンドウ
//
NSWindow *theWindow;

//
// ビュー
//
@interface SuikaView : NSView <NSWindowDelegate, NSApplicationDelegate>
@end
SuikaView *theView;

//
// 前方参照
//
static BOOL initWindow(void);
static BOOL initBackImage(void);
static BOOL openLog(void);
static void closeLog(void);

//
// メイン
//
int main()
{
#ifdef SSE_VERSIONING
	// ベクトル命令の対応を確認する
    x86_check_cpuid_flags();
#endif

#if !__has_feature(objc_arc)
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#else
    @autoreleasepool {
#endif

    // ログをオープンする
    if (openLog()) {
        // パッケージの初期化処理を行う
        if (init_file()) {
            // コンフィグの初期化処理を行う
            if (init_conf()) {
                // オーディオユニットの初期化処理を行う
                if(init_aunit()) {
                    // 背景イメージの作成を行う
                    if(initBackImage()) {
                        // アプリケーション本体の初期化を行う
                        if(on_event_init()) {
                            // ウィンドウを作成する
                            if(initWindow()) {
                                // メインループを実行する
                                [NSApp activateIgnoringOtherApps:YES];
                                [NSApp run];

                                // アプリケーション本体の終了処理を行う
                                on_event_cleanup();
                            }

                            // TODO: How to destroy theView?
                            [theWindow close];
                        }

                        // 背景イメージの破棄を行う
                        destroy_image(backImage);
                    }

                    // オーディオユニットの終了処理を行う
                    cleanup_aunit();
                }

                // コンフィグの終了処理を行う
                cleanup_conf();
            }

            // パッケージの終了処理を行う
            cleanup_file();
        }

        // ログをクローズする
        closeLog();
    }

#if !__has_feature(objc_arc)
    [pool release];
#else
    }
#endif

	return 0;
}

// ログをオープンする
static BOOL openLog(void)
{
    // .appバンドルのパスを取得する
    NSString *bundlePath = [[NSBundle mainBundle] bundlePath];

    // .appバンドルの1つ上のディレクトリのパスを取得する
    NSString *basePath = [bundlePath stringByDeletingLastPathComponent];

    // ログのパスを生成する
    const char *path = [[NSString stringWithFormat:@"%@/%s", basePath,
                                  LOG_FILE] UTF8String];

    // ログをオープンする
    logFp = fopen(path, "w");
    if(logFp == NULL) {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Error"];
        [alert setInformativeText:@"Cannot open log file."];
        [alert runModal];
#if !__has_feature(objc_arc)
        [alert autorelease];
#endif
        return NO;
    }
    return YES;
}

// ログをクローズする
static void closeLog(void)
{
    // ログをクローズする
    if(logFp != NULL)
        fclose(logFp);
}

// イメージの作成を行う
static BOOL initBackImage(void)
{
#ifndef SSE_VERSIONING
    backImagePixels = malloc((size_t)(conf_window_width * conf_window_height *
                             4));
    if (backImagePixels == NULL) {
        [NSApp terminate:nil];
        return false;
    }
#else
    if (posix_memalign((void **)&backImagePixels, SSE_ALIGN,
                       (size_t)(conf_window_width * conf_window_height * 4))
        != 0) {
        return NO;
    }
#endif

    backImage = create_image_with_pixels(conf_window_width, conf_window_height,
                                         (pixel_t *)backImagePixels);
	if(conf_window_white) {
        lock_image(backImage);
		clear_image_white(backImage);
        unlock_image(backImage);
    }

    return YES;
}

// ウィンドウの初期化処理を行う
static BOOL initWindow(void)
{
    // アプリケーションの初期化処理を行う
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    // メニューバーを作成する
    NSMenu *menuBar = [NSMenu new];
    NSMenuItem *appMenuItem = [NSMenuItem new];
    [menuBar addItem:appMenuItem];
    [NSApp setMainMenu:menuBar];

    // アプリケーションのメニューを作成する
    id appMenu = [NSMenu new];
    id quitMenuItem = [[NSMenuItem alloc]
                          initWithTitle:[NSString stringWithFormat:@"%@ %s",
                                                  @"Quit", conf_window_title]
                                 action:@selector(performClose:)
                          keyEquivalent:@"q"];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];

    // ウィンドウを作成する
    theWindow = [[NSWindow alloc]
                     initWithContentRect:NSMakeRect(0, 0,
                                                    conf_window_width,
                                                    conf_window_height)
                               styleMask:NSTitledWindowMask |
                                         NSClosableWindowMask |
                                         NSMiniaturizableWindowMask
                                 backing:NSBackingStoreBuffered
                                   defer:NO];
    [theWindow setCollectionBehavior:
                   [theWindow collectionBehavior] |
                   NSWindowCollectionBehaviorFullScreenPrimary];
    [theWindow cascadeTopLeftFromPoint:NSMakePoint(20,20)];
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
    NSTimer *timer = [NSTimer
                         scheduledTimerWithTimeInterval:1.0/30.0
                                                 target:theView
                                               selector:@selector(timerFired:)
                                               userInfo:nil
                                                repeats:YES];

    // Hack: コマンドラインから起動された際にメニューを有効にする
    ProcessSerialNumber psn = {0, kCurrentProcess};
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);

#if !__has_feature(objc_arc)
    [menuBar autorelease];
    [appMenuItem autorelease];
    [appMenu autorelease];
    [appMenuItem autorelease];
    [theWindow autorelease];
    [theView autorelease];
    [timer autorelease];
#else
    (void)timer;
#endif

    return YES;
}

//
// platform.hの実装
//

//
// セーブディレクトリを作成する
//
bool make_sav_dir(void)
{
#if !__has_feature(objc_arc)
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#else
    @autoreleasepool {
#endif
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
#if !__has_feature(objc_arc)
    [pool release];
#else
    }
#endif
	return true;
}

//
// データファイルのディレクトリ名とファイル名を指定して有効なパスを取得する
//
char *make_valid_path(const char *dir, const char *fname)
{
    char *ret;

#if !__has_feature(objc_arc)
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#else
    @autoreleasepool {
#endif
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

#if !__has_feature(objc_arc)
    [pool release];
#else
    }
#endif

    if(ret == NULL) {
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

    // ログファイルに出力する
    if (logFp != NULL) {
        fprintf(stderr, "%s", buf);
        fprintf(logFp, "%s", buf);
        fflush(logFp);
        if (ferror(logFp))
            return false;
    }
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

    // ログファイルに出力する
    if (logFp != NULL) {
        fprintf(stderr, "%s", buf);
        fprintf(logFp, "%s", buf);
        fflush(logFp);
        if (ferror(logFp))
            return false;
    }
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
    [alert setMessageText:conf_language == NULL ? @"エラー" : @"Error"];
    [alert setInformativeText:[[NSString alloc] initWithUTF8String:buf]];
    [alert runModal];

    // ログファイルに出力する
    if (logFp != NULL) {
        fprintf(stderr, "%s", buf);
        fprintf(logFp, "%s", buf);
        fflush(logFp);
        if (ferror(logFp))
            return false;
    }
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
// テクスチャをロックする
//
bool lock_texture(int width, int height, pixel_t *pixels,
                  pixel_t **locked_pixels, void **texture)
{
	assert(*locked_pixels == NULL);

	*locked_pixels = pixels;

    return true;
}

//
// テクスチャをアンロックする
//
void unlock_texture(int width, int height, pixel_t *pixels,
                    pixel_t **locked_pixels, void **texture)
{
	assert(*locked_pixels != NULL);

	*locked_pixels = NULL;
}

//
// テクスチャを破棄する
//
void destroy_texture(void *texture)
{
    UNUSED_PARAMETER(texture);
}

//
// イメージをレンダリングする
//
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
                  int width, int height, int src_left, int src_top, int alpha,
                  int bt)
{
    draw_image(backImage, dst_left, dst_top, src_image, width, height,
               src_left, src_top, alpha, bt);
}

//
// イメージをマスク描画でレンダリングする
//
void render_image_mask(int dst_left, int dst_top,
                       struct image * RESTRICT src_image,
                       int width, int height, int src_left, int src_top,
                       int mask)
{
    draw_image_mask(backImage, dst_left, dst_top, src_image, width, height,
                    src_left, src_top, mask);
}

//
// 画面をクリアする
//
void render_clear(int left, int top, int width, int height, pixel_t color)
{
    clear_image_color_rect(backImage, left, top, width, height, color);
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
    NSAlert *alert = [[NSAlert alloc] init];
#if !__has_feature(objc_arc)
    [alert autorelease];
#endif
    [alert addButtonWithTitle:conf_language == NULL ? @"はい" : @"Yes"];
    [alert addButtonWithTitle:conf_language == NULL ? @"いいえ" : @"No"];
    [alert setMessageText:conf_language == NULL ? @"終了しますか？" :
               @"Quit?"];
    [alert setAlertStyle:NSWarningAlertStyle];
    if([alert runModal] == NSAlertFirstButtonReturn)
        return true;
    return false;
}    

//
// タイトルに戻るダイアログを表示する
//
bool title_dialog(void)
{
    NSAlert *alert = [[NSAlert alloc] init];
#if !__has_feature(objc_arc)
    [alert autorelease];
#endif
    [alert addButtonWithTitle:conf_language == NULL ? @"はい" : @"Yes"];
    [alert addButtonWithTitle:conf_language == NULL ? @"いいえ" : @"No"];
    [alert setMessageText:conf_language == NULL ? @"タイトルへ戻りますか？" :
               @"Are you sure you want to go to title?"];
    [alert setAlertStyle:NSWarningAlertStyle];
    if([alert runModal] == NSAlertFirstButtonReturn)
        return true;
    return false;
}

//
// ビューの実装
//
@implementation SuikaView

// タイマコールバック
- (void)timerFired:(NSTimer *)timer {
    UNUSED_PARAMETER(timer);
    
#if !__has_feature(objc_arc)
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#else
    @autoreleasepool {
#endif

    // フレーム描画イベントを実行する
    int x = 0, y = 0, w = 0, h = 0;
    lock_image(backImage);
    if (!on_event_frame(&x, &y, &w, &h)) {
        // グローバルデータを保存する
        save_global_data();

        // 既読フラグを保存する
        save_seen();

        [NSApp terminate:nil];
        return;
    }
    unlock_image(backImage);

    // 描画範囲があればウィンドウへの再描画を行う
    if (w != 0 && h != 0)
        [self setNeedsDisplay:YES];
    
#if !__has_feature(objc_arc)
    [pool release];
#else
    }
#endif

    // FIXME:
    // [self setNeedsDisplayInRect:NSMakeRect(x, y, w, h)];
}

- (void)drawRect:(NSRect)rect {
    // 描画範囲がない場合
    if (rect.size.width == 0 || rect.size.height == 0)
        return;
    
    // 描画オブジェクトの解放用にプールを作成する
#if !__has_feature(objc_arc)
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#else
    @autoreleasepool {
#endif

        // NSBitmapImageRepを作成する
        unsigned char *array[1];
        array[0] = backImagePixels;
        NSBitmapImageRep *rep =
        [[NSBitmapImageRep alloc]
         initWithBitmapDataPlanes:array
                       pixelsWide:conf_window_width
                       pixelsHigh:conf_window_height
                    bitsPerSample:8
                  samplesPerPixel:4
                         hasAlpha:YES
                         isPlanar:NO
                   colorSpaceName:NSDeviceRGBColorSpace
                      bytesPerRow:conf_window_width * 4
                    bitsPerPixel:32];
        assert(rep != NULL);

        // NSImageに変換する
        NSImage *img = [[NSImage alloc] initWithSize:rep.size];
        assert(img != NULL);
        [img addRepresentation:rep];
        
        // 描画を行う
        [img drawAtPoint:NSMakePoint(0, 0)
                fromRect:NSMakeRect(0, 0,
                                    conf_window_width,
                                    conf_window_height)
               operation:NSCompositeCopy
                fraction:1.0];

#if !__has_feature(objc_arc)
        [img autorelease];
        [rep autorelease];
        [pool release];
#else
    }
#endif
}

- (void)mouseDown:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    if(pos.x < 0 && pos.x >= conf_window_width)
        return;
    if(pos.y < 0 && pos.y >= conf_window_height)
        return;
        
	on_event_mouse_press(MOUSE_LEFT, (int)pos.x,
                         conf_window_height - (int)pos.y);
}

- (void)mouseUp:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    if(pos.x < 0 && pos.x >= conf_window_width)
        return;
    if(pos.y < 0 && pos.y >= conf_window_height)
        return;

	on_event_mouse_release(MOUSE_LEFT, (int)pos.x,
                           conf_window_height - (int)pos.y);
}

- (void)rightMouseDown:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    if(pos.x < 0 && pos.x >= conf_window_width)
        return;
    if(pos.y < 0 && pos.y >= conf_window_height)
        return;
        
	on_event_mouse_press(MOUSE_RIGHT, (int)pos.x,
                         conf_window_height - (int)pos.y);
}

- (void)rightMouseUp:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    if(pos.x < 0 && pos.x >= conf_window_width)
        return;
    if(pos.y < 0 && pos.y >= conf_window_height)
        return;

	on_event_mouse_release(MOUSE_RIGHT, (int)pos.x,
                           conf_window_height - (int)pos.y);
}

- (void)mouseMoved:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    if(pos.x < 0 && pos.x >= conf_window_width)
        return;
    if(pos.y < 0 && pos.y >= conf_window_height)
        return;

	on_event_mouse_move((int)pos.x, conf_window_height - (int)pos.y);
}

- (void)scrollWheel:(NSEvent *)theEvent {
    if([theEvent deltaY] > 0) {
        on_event_key_press(KEY_UP);
        on_event_key_release(KEY_UP);
    } else if([theEvent deltaY] < 0) {
        on_event_key_press(KEY_DOWN);
        on_event_key_release(KEY_DOWN);
    }
}

- (void)flagsChanged:(NSEvent *)theEvent {
    // Controlキーの状態を取得する
    BOOL bit = ([theEvent modifierFlags] & NSControlKeyMask) ==
    NSControlKeyMask;
    
    // Controlキーの状態が変化した場合は通知する
    if(!isControlPressed && bit) {
        isControlPressed = YES;
        on_event_key_press(KEY_CONTROL);
    } else if(isControlPressed && !bit) {
        isControlPressed = NO;
        on_event_key_release(KEY_CONTROL);
    }
}

- (void)keyDown:(NSEvent *)theEvent {
    if([theEvent isARepeat])
        return;

    int kc = [self convertKeyCode:[theEvent keyCode]];
    if(kc != -1)
        on_event_key_press(kc);
}

- (void)keyUp:(NSEvent *)theEvent {
    int kc = [self convertKeyCode:[theEvent keyCode]];
    if(kc != -1)
        on_event_key_release(kc);
}

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
    }
    return -1;
}

- (NSSize)window:(NSWindow *)window
willUseFullScreenContentSize:(NSSize)proposedSize {
    UNUSED_PARAMETER(window);
    UNUSED_PARAMETER(proposedSize);

    NSSize modSize;
    modSize.width = conf_window_width;
    modSize.height = conf_window_height;

    return modSize;
}

- (BOOL)windowShouldClose:(id)sender {
    UNUSED_PARAMETER(sender);

    NSAlert *alert = [[NSAlert alloc] init];
#if !__has_feature(objc_arc)
    [alert autorelease];
#endif
    [alert addButtonWithTitle:conf_language == NULL ? @"はい" : @"Yes"];
    [alert addButtonWithTitle:conf_language == NULL ? @"いいえ" : @"No"];
    [alert setMessageText:conf_language == NULL ? @"終了しますか？" :
        @"Quit?"];
    [alert setAlertStyle:NSWarningAlertStyle];
    if([alert runModal] == NSAlertFirstButtonReturn) {
        // グローバルデータを保存する
        save_global_data();

        // 既読フラグを保存する
        save_seen();
        return YES;
    } else {
        return NO;
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app {
    UNUSED_PARAMETER(app);
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

@end
