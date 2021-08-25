//
//  main.m
//  suika
//
//  Created by tabata on 2021/08/21.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"

#import <sys/time.h>    // gettimeofday()

#import "suika.h"
#import "aunit.h"

int main(int argc, char * argv[]) {
    if(!init_file()) {
        NSLog(@"File error.");
        return 1;
    }

    if(!init_conf()) {
        NSLog(@"Config error.");
    }
    
    if(!init_aunit()) {
        NSLog(@"Audio error.");
        return 1;
    }

    if(!on_event_init()) {
        NSLog(@"Init event error.");
        return 1;
    }

    NSString * appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
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
        NSString *path = [NSString stringWithFormat:@"%@%@",
                          NSHomeDirectory(),
                          @"/Library/Application Support/sav"];
        NSFileManager *manager = [NSFileManager defaultManager];
        NSError *error;
        if(![manager createDirectoryAtPath:path
               withIntermediateDirectories:NO
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
// テクスチャをロックする
//
bool lock_texture(int width, int height, pixel_t *pixels,
                  pixel_t **locked_pixels, void **texture)
{
    assert(*locked_pixels == NULL);

    // TODO: Implement
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

    // TODO: Implement
    *locked_pixels = NULL;
}

//
// テクスチャを破棄する
//
void destroy_texture(void *texture)
{
    // TODO: Implement
}

//
// イメージをレンダリングする
//
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
                  int width, int height, int src_left, int src_top, int alpha,
                  int bt)
{
    // TODO: Implement
}

//
// イメージをマスク描画でレンダリングする
//
void render_image_mask(int dst_left, int dst_top,
                       struct image * RESTRICT src_image,
                       int width, int height, int src_left, int src_top,
                       int mask)
{
    // TODO: Implement
}

//
// 画面をクリアする
//
void render_clear(int left, int top, int width, int height, pixel_t color)
{
    // TODO: Implement
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

/* タイトルに戻るダイアログを表示する */
bool title_dialog(void)
{
    return true;
}
