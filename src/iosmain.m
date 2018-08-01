/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * iOSメインモジュール
 * [Changes]
 *  - 2016/07/29 作成
 */

#import <UIKit/UIKit.h>

//
// AppDelegateのインタフェース
//
@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@property NSTimer *timer;
@end

//
// ViewControllerのインタフェース
//
@interface ViewController : UIViewController
@end

//
// MainViewのインタフェース
//
@interface MainView : UIView
@property int alpha;
- (void)timerFired:(NSTimer *)timer;
@end

//
// タイマオブジェクト
//
NSTimer *theTimer;

//
// AppDelegateの実装
//
@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
}

- (void)applicationWillTerminate:(UIApplication *)application {
}

@end

//
// ViewControllerの実装
//
@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

@end

CGImageRef backImage;
UIImage *bg, *ch;

//
// MainViewの実装
//
@implementation MainView

// タイマコールバック
- (void)timerFired:(NSTimer *)timer {
    printf("timer fired\n");
    [self setNeedsDisplay];
}

- (void)drawRect:(CGRect)rect {
    printf("drawRect %d\n", self.alpha);
    if(theTimer == nil) {
        printf("timer start\n");

        bg = [UIImage imageNamed:@"bg/roof.png"];
        ch = [UIImage imageNamed:@"ch/001-fun.png"];

        // タイマをセットする
        theTimer = [NSTimer scheduledTimerWithTimeInterval:1.0/30.0
                                                    target:self
                                                  selector:@selector(timerFired:)
                                                  userInfo:nil
                                                   repeats:YES];
    }

    CGContextRef context = UIGraphicsGetCurrentContext();
    CGRect myFrame = self.bounds;
    CGContextSetLineWidth(context, 10);
    [[UIColor redColor] set];
    UIRectFrame(myFrame);

//    UIGraphicsBeginImageContextWithOptions(bg.size, NO, 0.0);
    [bg drawInRect:CGRectMake(0.0, 0.0, bg.size.width, bg.size.height)];

    [ch drawInRect:CGRectMake(0.0, 0.0, ch.size.width, ch.size.height)
         blendMode:kCGBlendModeNormal
             alpha:self.alpha/255.0f];
    self.alpha = (self.alpha + 1) % 256;

//    UIImage *newImage = UIGraphicsGetImageFromCurrentImageContext();
 //   UIGraphicsEndImageContext();

//    [newImage drawInRect:rect];
}

@end

//
// メイン
//
int main(int argc, char * argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
