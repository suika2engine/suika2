#import "ViewController.h"
#import "GameView.h"
#import "GameRenderer.h"

// Suika2 Base
#import "suika.h"

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
    CGSize _screenSize;
    CGPoint _screenOffset;

    // The video player objects and status.
    AVPlayer *_avPlayer;
    AVPlayerLayer *_avPlayerLayer;
    BOOL _isVideoPlaying;
}

- (void)viewDidLoad {
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

    // Setup a rendering timer.
    [NSTimer scheduledTimerWithTimeInterval:1.0/60.0
                                     target:self
                                   selector:@selector(timerFired:)
                                   userInfo:nil
                                    repeats:YES];
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

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    [self updateViewport:_view.frame.size];
}

- (void)timerFired:(NSTimer *)timer {
    [_view setNeedsDisplay];
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

@end

//
// HAL
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

    NSLog(@"%s", buf);
    
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

    return true;
}

//
// セーブディレクトリを作成する
//
bool make_sav_dir(void)
{
    @autoreleasepool {
        NSString *path = [NSString stringWithFormat:@"%@/%@/%s/sav",
                          NSHomeDirectory(),
                          @"/Library/Application Support",
                          conf_window_title];
        NSFileManager *manager = [NSFileManager defaultManager];
        NSError *error;
        if(![manager createDirectoryAtPath:path
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
        // セーブファイルの場合
        if(dir != NULL && strcmp(dir, "sav") == 0) {
            NSString *path = [NSString stringWithFormat:@"%@/%@/%s/sav/%s",
                              NSHomeDirectory(),
                              @"/Library/Application Support",
                              conf_window_title,
                              fname];
            const char *cstr = [path UTF8String];
            return strdup(cstr);
        }

        // data01.arcの場合
        if(dir == NULL && strcmp(fname, "data01.arc") == 0) {
            // data01.arcのパスを返す
            NSString *path = [[NSBundle mainBundle] pathForResource:@"data01" ofType:@"arc"];
            const char *cstr = [path UTF8String];
            return strdup(cstr);
        }

        // 動画の場合
        if(dir != NULL && strcmp(dir, MOV_DIR) == 0) {
            // 動画のパスを返す
            *strstr(fname, ".") = '\0';
            NSString *basename = [[NSString alloc] initWithUTF8String:fname];
            NSString *path = [[NSBundle mainBundle] pathForResource:basename ofType:@"mp4"];
            const char *cstr = [path UTF8String];
            return strdup(cstr);
        }

        // その他のファイルは読み込めない
        return strdup("dummy");
    }
}
