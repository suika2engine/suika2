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


@interface ViewController ()
@end

@implementation ViewController
{
    GameView *_view;
    GameRenderer *_renderer;
    vector_uint2 _viewportSize;
    float _screenScale, _left, _top;
    NSRect _savedFrame;
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
    _renderer = [[GameRenderer alloc] initWithMetalKitView:_view];
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
    if(self.playerLayer != nil)
        [self.playerLayer setFrame:NSMakeRect(0, 0, size.width, size.height)];

    // スクリーンサイズを返す
    return size;
}

// フルスクリーンになるとき呼び出される
- (void)windowWillEnterFullScreen:(NSNotification *)notification {
    self.isFullscreen = YES;

    // ウィンドウサイズを保存する
    _savedFrame = self.view.window.frame;
}

// フルスクリーンから戻るときに呼び出される
- (void)windowWillExitFullScreen:(NSNotification *)notification {
    self.isFullscreen = NO;

    // 動画プレーヤレイヤのサイズを元に戻す
    if(self.playerLayer != nil)
        [self.playerLayer setFrame:NSMakeRect(0, 0, _savedFrame.size.width, _savedFrame.size.height)];

    [self layout:_savedFrame.size];
}

- (NSString *)uiMessage:(int)id {
    const wchar_t *wcs = get_ui_message(id);
    return [[NSString alloc] initWithBytes:wcs
                                    length:wcslen(wcs) * sizeof(*wcs)
                                  encoding:NSUTF32LittleEndianStringEncoding];
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
@end
