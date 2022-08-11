// -*- tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changed]
 *  - 2021/08/21 Created.
 */

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <sys/time.h>    // gettimeofday()

#import "suika.h"
#import "aunit.h"
#import "glrender.h"

//
// Interfaces
//

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@end

@interface GameViewController : UIViewController
{
    BOOL animating;
    NSInteger animationFrameInterval;
    CADisplayLink *displayLink;
}
@end

@interface OpenGLView : UIView
{
    CAEAGLLayer* _eaglLayer;
    EAGLContext* _context;
    GLuint _renderBuffer;
    GLuint _frameBuffer;

    int _left, _top;
    float _scale;

    BOOL _isTouch;
    float _touchStartX, _touchStartY, _touchLastY;
}
- (void)render;
@end

//
// main()
//

int main(int argc, char * argv[]) {
    NSString * appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}

//
// AppDelegate
//

@interface AppDelegate ()
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
    pause_sound();
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    resume_sound();
}

@end

//
// GameViewController
//

@implementation GameViewController

- (void)awakeFromNib
{
    [super awakeFromNib];

    animating = FALSE;
    animationFrameInterval = 1;
    displayLink = nil;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    self.view.multipleTouchEnabled = YES;
}

- (void)viewWillAppear:(BOOL)animated
{
    [self startAnimation];

    [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [self stopAnimation];
    
    [super viewWillDisappear:animated];
}

- (void)startAnimation
{
    if (!animating)
    {
        CADisplayLink *aDisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawFrame)];
        [aDisplayLink setFrameInterval:animationFrameInterval];
        [aDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        displayLink = aDisplayLink;
        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        [displayLink invalidate];
        displayLink = nil;
        animating = FALSE;
    }
}

- (void)drawFrame
{
    [(OpenGLView *)self.view render];
}

@end

//
// OpenGLView
//

@interface OpenGLView()
- (void)setupBuffers;
- (void)destroyBuffers;
@end

@implementation OpenGLView

+ (Class)layerClass {
    return [CAEAGLLayer class];
}

- (void)awakeFromNib
{
    [super awakeFromNib];

    // ビューの設定を行う
    self.opaque = NO;
    self.backgroundColor = [UIColor clearColor];

    // レイヤの設定を行う
    _eaglLayer = (CAEAGLLayer*) self.layer;
    _eaglLayer.opaque = NO;
    _eaglLayer.drawableProperties =
        [NSDictionary dictionaryWithObjectsAndKeys:
                          [NSNumber numberWithBool:NO],
                      kEAGLDrawablePropertyRetainedBacking,
                      kEAGLColorFormatRGBA8,
                      kEAGLDrawablePropertyColorFormat,
                      nil];

    // コンテキストの設定を行う
    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:_context];

    // data01.arcを読み込む
    if(!init_file()) {
        NSLog(@"File error.");
        exit(1);
    }

    // コンフィグファイルをパースする
    if(!init_conf()) {
        NSLog(@"Config error.");
        exit(1);
    }

    // オーディオを初期化する
    if(!init_aunit()) {
        NSLog(@"Audio error.");
        exit(1);
    }

    // Suika2のOpenGL ESレンダラを初期化する
    if(!init_opengl()) {
        NSLog(@"Init OpenGL error.");
        exit(1);
    }

    // 初期化イベントを処理する
    if(!on_event_init()) {
        NSLog(@"Init event error.");
        exit(1);
    }
}

- (void)layoutSubviews
{
    [EAGLContext setCurrentContext:_context];
    [self destroyBuffers];
    [self setupBuffers];

    // ゲーム画面のアスペクト比を求める
    float aspect = (float)conf_window_height / (float)conf_window_width;

    // ビューのサイズを取得する
    float vw = self.bounds.size.width * self.contentScaleFactor;
    float vh = self.bounds.size.height * self.contentScaleFactor;

    // 横幅優先で高さを仮決めする
    float w = vw;
    float h = vw * aspect;
    _scale = (float)conf_window_width / w;

    // 高さが足りなければ、縦幅優先で横幅を決める
    if(h > vh) {
        h = vh;
        w = vh / aspect;
        _scale = (float)conf_window_height / h;
    }

    // スクリーンの原点を決める
    float x = (self.bounds.size.width * self.contentScaleFactor - w) / 2.0f;
    float y = (self.bounds.size.height * self.contentScaleFactor - h) / 2.0f;

    glViewport(x, y, w, h);

    _left = (int)x;
    _top = (int)y;
}

- (void)render
{
    // レンダリングを開始する
    opengl_start_rendering();

    // フレーム描画イベントを実行する
    //  - 更新領域 x, y, w, h は無視する
    int x, y, w, h;
    bool cont = on_event_frame(&x, &y, &w, &h);

    // レンダリングを終了する
    opengl_end_rendering();

    [_context presentRenderbuffer:GL_RENDERBUFFER];

    // スクリプトの末尾まで実行し終わった場合
    if(!cont) {
        // グローバル変数を保存する
        save_global_data();

        // 既読フラグを保存する
        save_seen();

        // 初期スクリプトをロードする
        if(!load_script("init.txt")) {
            NSLog(@"load_script error.");
            exit(1);
        }
    }
}

- (void)setupBuffers
{
    glGenRenderbuffers(1, &_renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffer);
    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:_eaglLayer];

    glGenFramebuffers(1, &_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, _renderBuffer);
}

- (void)destroyBuffers
{
    if(_frameBuffer != 0) {
        glDeleteFramebuffers(1, &_frameBuffer);
        _frameBuffer = 0;
    }
    if(_renderBuffer != 0) {
        glDeleteRenderbuffers(1, &_renderBuffer);
        _renderBuffer = 0;
    }
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = [[event allTouches] anyObject];
    CGPoint touchLocation = [touch locationInView:self];

    _isTouch = YES;
    _touchStartX = (touchLocation.x - _left) * _scale;
    _touchStartY = (touchLocation.y - _top) * _scale;
    _touchLastY = _touchStartY;

    on_event_mouse_move((int)_touchStartX, (int)_touchStartY);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = [[event allTouches] anyObject];
    CGPoint touchLocation = [touch locationInView:self];

    float touchX = (touchLocation.x - _left) * _scale;
    float touchY = (touchLocation.y - _top) * _scale;

    const float LINE_HEIGHT = 10;
    float delta = touchY - _touchLastY;
    if(delta > LINE_HEIGHT) {
        on_event_key_press(KEY_DOWN);
        on_event_key_release(KEY_DOWN);
    } else if(delta < -LINE_HEIGHT) {
        on_event_key_press(KEY_UP);
        on_event_key_release(KEY_UP);
    }

    _touchLastY = touchY;

    on_event_mouse_move((int)touchX, (int)touchY);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = [[event allTouches] anyObject];
    CGPoint touchLocation = [touch locationInView:self];

    float touchEndX = (touchLocation.x - _left) * _scale;
    float touchEndY = (touchLocation.y - _top) * _scale;

    if([[event allTouches] count] == 1)
        on_event_mouse_press(MOUSE_LEFT, (int)touchEndX, (int)touchEndY);
    else
        on_event_mouse_press(MOUSE_RIGHT, (int)touchEndX, (int)touchEndY);

    _isTouch = NO;
}

@end

//
// platform.hの実装
//

//
// セーブディレクトリを作成する
//
bool make_sav_dir(void)
{
    @autoreleasepool {
        NSString *path = [NSString stringWithFormat:@"%@%@",
                          NSHomeDirectory(),
                          @"/Library/Application Support/sav"];
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
            NSString *path = [NSString stringWithFormat:@"%@%@%s",
                              NSHomeDirectory(),
                              @"/Library/Application Support/sav/",
                              fname];
            const char *cstr = [path UTF8String];
            return strdup(cstr);
        }

        // data01.arcにしか対応しない
        if(dir != NULL || strcmp(fname, "data01.arc") != 0)
            return strdup("dummy");

        // data01.arcのパスを返す
        NSString *path = [[NSBundle mainBundle] pathForResource:@"data01" ofType:@"arc"];
        const char *cstr = [path UTF8String];
        return strdup(cstr);
    }
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

    NSLog(@"Suika2: Info: %s", buf);

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

    NSLog(@"Suika2: Warn: %s", buf);

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

    NSLog(@"Suika2: Error: %s", buf);

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
    assert(*locked_pixels == NULL);

    if (!opengl_lock_texture(width, height, pixels, locked_pixels, texture))
        return false;

    return true;
}

//
// テクスチャをアンロックする
//
void unlock_texture(int width, int height, pixel_t *pixels,
                    pixel_t **locked_pixels, void **texture)
{
    assert(*locked_pixels != NULL);

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
    opengl_render_image(dst_left, dst_top, src_image, width, height,
                        src_left, src_top, alpha, bt);
}

//
// イメージをルールつきでレンダリングする
//
void render_image_rule(struct image * RESTRICT src_img,
                       struct image * RESTRICT rule_img,
                       int threshold)
{
	opengl_render_image_rule(src_img, rule_img, threshold);
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
    return true;
}

//
// タイトルに戻るダイアログを表示する
//
bool title_dialog(void)
{
    return true;
}

//
// 削除ダイアログを表示する
//
bool delete_dialog(void)
{
	return true;
}

//
// 初期設定ダイアログを表示する
//
bool default_dialog(void)
{
	return true;
}

//
// ビデオを再生する
//
bool play_video(const char *fname, bool is_skippable)
{
	UNUSED_PARAMETER(fname);
	UNUSED_PARAMETER(is_skippable);

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
}
