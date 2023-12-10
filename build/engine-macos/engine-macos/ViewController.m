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
    GameView *_view;
    GameRenderer *_renderer;
    float _screenScale, _left, _top;
    NSRect _savedFrame;
    BOOL _isFullScreen;
    BOOL _isControlPressed;
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
    NSRect sr = [[NSScreen mainScreen] visibleFrame];
    NSRect cr = NSMakeRect(sr.origin.x + (sr.size.width - conf_window_width) / 2,
                           sr.origin.y + (sr.size.height - conf_window_height) / 2,
                           conf_window_width,
                           conf_window_height);
    [self.view.window setFrame:cr display:TRUE];
    
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
    NSSize size = [self layout:proposedSize];
    
    // 動画プレーヤレイヤのサイズを更新する
    if (_avPlayerLayer != nil)
        [_avPlayerLayer setFrame:NSMakeRect(0, 0, size.width, size.height)];
    
    // スクリーンサイズを返す
    return size;
}

// フルスクリーンになるとき呼び出される
- (void)windowWillEnterFullScreen:(NSNotification *)notification {
    _isFullScreen = YES;
    
    // ウィンドウサイズを保存する
    _savedFrame = self.view.window.frame;
}

// フルスクリーンから戻るときに呼び出される
- (void)windowWillExitFullScreen:(NSNotification *)notification {
    _isFullScreen = NO;
    
    // 動画プレーヤレイヤのサイズを元に戻す
    if(_avPlayerLayer != nil)
        [_avPlayerLayer setFrame:NSMakeRect(0, 0, _savedFrame.size.width, _savedFrame.size.height)];
    
    [self layout:_savedFrame.size];
}

- (NSString *)uiMessage:(int)id {
    return [[NSString alloc] initWithUTF8String:get_ui_message(id)];
}

- (NSSize)layout:(NSSize)size {
    // ゲーム画面のアスペクト比を求める
    float aspect = (float)conf_window_height / (float)conf_window_width;
    
    // 横幅優先で高さを仮決めする
    float width = size.width;
    float height = width * aspect;
    _screenScale = (float)conf_window_width / width;
    
    // 高さが足りなければ、縦幅優先で横幅を決める
    if(height > size.height) {
        height = size.height;
        width = size.height / aspect;
        _screenScale = (float)conf_window_height / height;
    }
    
    // マージンを計算する
    _left = (size.width - width) / 2.0f;
    _top = (size.height - height) / 2.0f;
    
    return NSMakeSize(width, height);
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

// マウス押下イベント
- (void)mouseDown:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= _screenScale;
    pos.y *= _screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

    on_event_mouse_press(MOUSE_LEFT, (int)pos.x, conf_window_height - (int)pos.y);
}

// マウス解放イベント
- (void)mouseUp:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= _screenScale;
    pos.y *= _screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

    on_event_mouse_release(MOUSE_LEFT, (int)pos.x, conf_window_height - (int)pos.y);
}

// マウス右ボタン押下イベント
- (void)rightMouseDown:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= _screenScale;
    pos.y *= _screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

    on_event_mouse_press(MOUSE_RIGHT, (int)pos.x, conf_window_height - (int)pos.y);
}

// マウス右ボタン解放イベント
- (void)rightMouseUp:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= _screenScale;
    pos.y *= _screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

    on_event_mouse_release(MOUSE_RIGHT, (int)pos.x, conf_window_height - (int)pos.y);
}

// マウス移動イベント
- (void)mouseMoved:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= _screenScale;
    pos.y *= _screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

    on_event_mouse_move((int)pos.x, conf_window_height - (int)pos.y);
}

// マウスドラッグイベント
- (void)mouseDragged:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= _screenScale;
    pos.y *= _screenScale;
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

@end
