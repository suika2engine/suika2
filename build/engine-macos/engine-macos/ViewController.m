#import "ViewController.h"
#import "GameView.h"
#import "GameRenderer.h"

// Suika2 Base
#import "suika.h"
#import "uimsg.h"

// Standard C
#import <wchar.h>

// Suika2 HAL
#import "aunit.h"

static ViewController *theViewController;

@interface ViewController ()
@end

@implementation ViewController
{
    // The GameView for AppKit
    GameView *_view;

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

    // The control key status.
    BOOL _isControlPressed;

    // The video player objects and status.
    AVPlayer *_avPlayer;
    AVPlayerLayer *_avPlayerLayer;
    BOOL _isVideoPlaying;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
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
    _view = (GameView *)self.view;
    _view.enableSetNeedsDisplay = YES;
    _view.device = MTLCreateSystemDefaultDevice();
    _view.clearColor = MTLClearColorMake(0.0, 0.5, 1.0, 1.0);
    _renderer = [[GameRenderer alloc] initWithMetalKitView:_view andController:self];
    if(!_renderer) {
        NSLog(@"Renderer initialization failed");
        return;
    }
    [_renderer mtkView:_view drawableSizeWillChange:_view.drawableSize];
    _view.delegate = _renderer;
    [self updateViewport:_view.frame.size];

    // Setup a rendering timer.
    [NSTimer scheduledTimerWithTimeInterval:1.0/60.0
                                     target:self
                                   selector:@selector(timerFired:)
                                   userInfo:nil
                                    repeats:YES];
}

- (void)viewDidAppear {
    self.view.window.delegate = self;
    
    // Set the window position and size.
    NSRect screenRect = [[NSScreen mainScreen] visibleFrame];
    NSRect contentRect = NSMakeRect(screenRect.origin.x + (screenRect.size.width - conf_window_width) / 2,
                                    screenRect.origin.y + (screenRect.size.height - conf_window_height) / 2,
                                    conf_window_width,
                                    conf_window_height);
    NSRect windowRect = [self.view.window frameRectForContentRect:contentRect];
    [self.view.window setFrame:windowRect display:TRUE];
    
    // Enable the window maximization.
    if (!conf_window_fullscreen_disable)
        [self.view.window setCollectionBehavior:[self.view.window collectionBehavior] | NSWindowCollectionBehaviorFullScreenPrimary];
    
    // Set the window title.
    [self.view.window setTitle:[[NSString alloc] initWithUTF8String:conf_window_title]];
    
    // Accept keyboard and mouse inputs.
    [self.view.window makeKeyAndOrderFront:nil];
    [self.view.window setAcceptsMouseMovedEvents:YES];
    [self.view.window.delegate self];
    [self.view.window makeFirstResponder:self];

    // Set the app name in the main menu.
    [self setAppName];

    [self updateViewport:_view.frame.size];
}

- (void)setAppName {
    NSMenu *menu = [[[NSApp mainMenu] itemAtIndex:0] submenu];
    [menu setTitle:[[NSString alloc] initWithUTF8String:conf_window_title]];
}

- (void)timerFired:(NSTimer *)timer {
    [_view setNeedsDisplay:TRUE];
}

- (BOOL)windowShouldClose:(id)sender {
    @autoreleasepool {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:[self uiMessage:UIMSG_YES]];
        [alert addButtonWithTitle:[self uiMessage:UIMSG_NO]];
        [alert setMessageText:[self uiMessage:UIMSG_EXIT]];
        [alert setAlertStyle:NSAlertStyleWarning];
        if ([alert runModal] == NSAlertFirstButtonReturn)
            return YES;
        else
            return NO;
    }
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

- (IBAction)onQuit:(id)sender {
    if ([self windowShouldClose:sender])
        [NSApp stop:nil];
}

// フルスクリーンになる前に呼び出される
- (NSSize)window:(NSWindow *)window willUseFullScreenContentSize:(NSSize)proposedSize {
    // 表示位置を更新する
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

    [self setAppName];
}

// フルスクリーンから戻るときに呼び出される
- (void)windowWillExitFullScreen:(NSNotification *)notification {
    _isFullScreen = NO;
    
    // 動画プレーヤレイヤのサイズを元に戻す
    if(_avPlayerLayer != nil)
        [_avPlayerLayer setFrame:NSMakeRect(0, 0, _savedViewFrame.size.width, _savedViewFrame.size.height)];
    
    [self updateViewport:_savedViewFrame.size];
}

- (void)windowDidEnterFullScreen:(NSNotification *)notification {
    [self setAppName];
}

- (void)windowDidLeaveFullScreen:(NSNotification *)notification {
    [self setAppName];
}

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize {
    NSRect contentRect = [self.view.window contentRectForFrameRect:NSMakeRect(0, 0, frameSize.width, frameSize.height)];

    // Keep the original window size if (conf_window_size == 0).
    if (conf_window_resize == 0) {
        return NSMakeSize(conf_window_width, conf_window_height);
    }

    // Keep the aspect ratio if (conf_window_resize == 2).
    if (conf_window_resize == 2) {
        float aspect = (float)conf_window_height / (float)conf_window_width;
        if (self.view.window.frame.size.width != frameSize.width)
            contentRect.size.height = contentRect.size.width * aspect;
        else
            contentRect.size.width = contentRect.size.height / aspect;
    }

    // Save the contentRect.size for windowDidResize.
    _resizeFrame = contentRect.size;

    // Return the window size.
    NSRect windowRect = [self.view.window frameRectForContentRect:NSMakeRect(0, 0, contentRect.size.width, contentRect.size.height)];
    return windowRect.size;
}

- (void)windowDidResize:(NSNotification *)notification {
    [self updateViewport:_resizeFrame];
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
    _screenSize.width *= _view.layer.contentsScale;
    _screenSize.height *= _view.layer.contentsScale;
    newViewSize.width *= _view.layer.contentsScale;
    newViewSize.height *= _view.layer.contentsScale;

    // マージンを計算する
    _screenOffset.x = (newViewSize.width - _screenSize.width) / 2.0f;
    _screenOffset.y = (newViewSize.height - _screenSize.height) / 2.0f;
}

- (void)mouseMoved:(NSEvent *)event {
    NSPoint point = [event locationInWindow];
    int x = (int)((point.x - self.screenOffset.x) * _screenScale);
    int y = (int)((point.y - self.screenOffset.y) * _screenScale);
    on_event_mouse_move(x, conf_window_height - y);
}

// キーボード修飾変化イベント
- (void)flagsChanged:(NSEvent *)theEvent {
    // Controlキーの状態を取得する
    BOOL bit = ([theEvent modifierFlags] & NSEventModifierFlagControl) == NSEventModifierFlagControl;

    // Controlキーの状態が変化した場合は通知する
    if (!_isControlPressed && bit) {
        _isControlPressed = YES;
        on_event_key_press(KEY_CONTROL);
    } else if (_isControlPressed && !bit) {
        _isControlPressed = NO;
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

//
// GameViewControllerProtocol
//

- (void)setTitle:(NSString *)name {
    [self.view.window setTitle:name];
}

- (float)screenScale {
    return _screenScale;
}

- (NSPoint)screenOffset {
    return _screenOffset;
}

- (NSSize)screenSize {
    return _screenSize;
}

- (BOOL)isFullScreen {
    return _isFullScreen;
}

- (void)enterFullScreen {
    if (!_isFullScreen) {
        [self.view.window toggleFullScreen:self.view];
        [self setAppName];
    }
}

- (void)leaveFullScreen {
    if (!_isFullScreen) {
        [self.view.window toggleFullScreen:self.view];
        [self setAppName];
    }
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

@end

//
// HAL
//

//
// セーブディレクトリを作成する
//
bool make_sav_dir(void)
{
    @autoreleasepool {
        if (conf_release) {
            // リリースモードの場合
            NSString *path = NSHomeDirectory();
            path = [path stringByAppendingString:@"/Library/Application Support/"];
            path = [path stringByAppendingString:[[NSString alloc] initWithUTF8String:conf_window_title]];
            path = [path stringByAppendingString:@"/"];
            path = [path stringByAppendingString:[[NSString alloc] initWithUTF8String:SAVE_DIR]];
            [[NSFileManager defaultManager] createDirectoryAtPath:path
                                      withIntermediateDirectories:YES
                                                       attributes:nil
                                                            error:NULL];
            return true;
        }
        
        // 通常モードの場合
        NSString *bundlePath = [[NSBundle mainBundle] bundlePath];
        NSString *basePath = [bundlePath stringByDeletingLastPathComponent];
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
        if (conf_release && dir != NULL && strcmp(dir, SAVE_DIR) == 0) {
            // リリースモードの場合
            assert(fname != NULL);
            NSString *path = NSHomeDirectory();
            path = [path stringByAppendingString:@"/Library/Application Support/"];
            path = [path stringByAppendingString:[[NSString alloc] initWithUTF8String:conf_window_title]];
            path = [path stringByAppendingString:@"/"];
            path = [path stringByAppendingString:[[NSString alloc] initWithUTF8String:dir]];
            path = [path stringByAppendingString:@"/"];
            path = [path stringByAppendingString:[[NSString alloc] initWithUTF8String:fname]];
            char *ret = strdup([path UTF8String]);
            if (ret == NULL) {
                log_memory();
                return NULL;
            }
            return ret;
        }

        // 通常モードの場合
        NSString *bundlePath = [[NSBundle mainBundle] bundlePath];
        NSString *basePath = [bundlePath stringByDeletingLastPathComponent];
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
