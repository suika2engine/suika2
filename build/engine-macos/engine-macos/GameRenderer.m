@import simd;
@import MetalKit;

#import "GameRenderer.h"
#import "GameShaderTypes.h"
#import "GameViewControllerProtocol.h"

static id<GameViewControllerProtocol> theViewController;
static MTKView *theMTKView;
static id<MTLDevice> theDevice;
static id<MTLRenderPipelineState> theNormalPipelineState;
static id<MTLRenderPipelineState> theCopyPipelineState;
static id<MTLRenderPipelineState> theDimPipelineState;
static id<MTLRenderPipelineState> theRulePipelineState;
static id<MTLRenderPipelineState> theMeltPipelineState;
static id<MTLCommandBuffer> theCommandBuffer;
static id<MTLBlitCommandEncoder> theBlitEncoder;
static id<MTLRenderCommandEncoder> theRenderEncoder;
static NSSize theViewportSize;
static NSMutableArray *thePurgeArray;
static NSMutableArray *theDelayLoadArray;
static dispatch_semaphore_t in_flight_semaphore;

static BOOL runSuika2Frame(void);

@interface GameRenderer ()
@end

@implementation GameRenderer
{
    id<MTLCommandQueue> _commandQueue;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView andController:(nonnull id<GameViewControllerProtocol>)controller;{
    NSError *error = NULL;

    self = [super init];
    if(self == nil)
        return nil;

    theMTKView = mtkView;
    theDevice = mtkView.device;
    theViewController = controller;

    // Load shaders.
    id<MTLLibrary> defaultLibrary = [theDevice newDefaultLibrary];

    // Construct a normal shader pipeline.
    MTLRenderPipelineDescriptor *normalPipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    normalPipelineStateDescriptor.label = @"Normal Texturing Pipeline";
    normalPipelineStateDescriptor.vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
    normalPipelineStateDescriptor.fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentNormalShader"];
    normalPipelineStateDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat;
    normalPipelineStateDescriptor.colorAttachments[0].blendingEnabled = TRUE;
    normalPipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    normalPipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    normalPipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
    normalPipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    normalPipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    normalPipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor =  MTLBlendFactorOne;
    theNormalPipelineState = [theDevice newRenderPipelineStateWithDescriptor:normalPipelineStateDescriptor error:&error];
    NSAssert(theNormalPipelineState, @"Failed to create pipeline state: %@", error);

    // Construct a copy shader pipeline.
    MTLRenderPipelineDescriptor *copyPipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    copyPipelineStateDescriptor.label = @"Copy Texturing Pipeline";
    copyPipelineStateDescriptor.vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
    copyPipelineStateDescriptor.fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentCopyShader"];
    copyPipelineStateDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat;
    copyPipelineStateDescriptor.colorAttachments[0].blendingEnabled = FALSE;
    theCopyPipelineState = [theDevice newRenderPipelineStateWithDescriptor:copyPipelineStateDescriptor error:&error];
    NSAssert(theCopyPipelineState, @"Failed to create pipeline state: %@", error);

    // Construct a dim shader pipeline.
    MTLRenderPipelineDescriptor *dimPipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    dimPipelineStateDescriptor.label = @"Dim Texturing Pipeline";
    dimPipelineStateDescriptor.vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
    dimPipelineStateDescriptor.fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentDimShader"];
    dimPipelineStateDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat;
    dimPipelineStateDescriptor.colorAttachments[0].blendingEnabled = TRUE;
    dimPipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    dimPipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    dimPipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
    dimPipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    dimPipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    dimPipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor =  MTLBlendFactorOne;
    theDimPipelineState = [theDevice newRenderPipelineStateWithDescriptor:dimPipelineStateDescriptor error:&error];
    NSAssert(theDimPipelineState, @"Failed to create pipeline state: %@", error);

    // Construct a rule shader pipeline.
    MTLRenderPipelineDescriptor *rulePipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    rulePipelineStateDescriptor.label = @"Rule Texturing Pipeline";
    rulePipelineStateDescriptor.vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
    rulePipelineStateDescriptor.fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentRuleShader"];
    rulePipelineStateDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat;
    rulePipelineStateDescriptor.colorAttachments[0].blendingEnabled = TRUE;
    rulePipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    rulePipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    rulePipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
    rulePipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    rulePipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    theRulePipelineState = [theDevice newRenderPipelineStateWithDescriptor:rulePipelineStateDescriptor error:&error];
    NSAssert(theRulePipelineState, @"Failed to create pipeline state: %@", error);

    // Construct a melt shader pipeline.
    MTLRenderPipelineDescriptor *meltPipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    meltPipelineStateDescriptor.label = @"Melt Texturing Pipeline";
    meltPipelineStateDescriptor.vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
    meltPipelineStateDescriptor.fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentMeltShader"];
    meltPipelineStateDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat;
    meltPipelineStateDescriptor.colorAttachments[0].blendingEnabled = TRUE;
    meltPipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    meltPipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    meltPipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
    meltPipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    meltPipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    theMeltPipelineState = [theDevice newRenderPipelineStateWithDescriptor:meltPipelineStateDescriptor error:&error];
    NSAssert(theMeltPipelineState, @"Failed to create pipeline state: %@", error);

    // Create a command queue.
    _commandQueue = [theDevice newCommandQueue];

    // Create a delay load texture array for images loaded before the first rendering.
    theDelayLoadArray = [NSMutableArray array];

    // TODO:
    in_flight_semaphore = dispatch_semaphore_create(1);

    return self;
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    theViewportSize.width = size.width;
    theViewportSize.height = size.height;
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    if(view.currentRenderPassDescriptor == nil)
        return;
    
    // Create a command buffer.
    theCommandBuffer = [_commandQueue commandBuffer];
    theCommandBuffer.label = @"MyCommand";

    // Nil-ify the encoders.
    theBlitEncoder = nil;
    theRenderEncoder = nil;

    // Create textures for images that are loaded before the first rendering.
    if (theDelayLoadArray != nil) {
        for (struct image *img in theDelayLoadArray)
            unlock_texture(img);
        theDelayLoadArray = nil;
    }

    dispatch_semaphore_wait(in_flight_semaphore, DISPATCH_TIME_FOREVER);
    __block dispatch_semaphore_t block_sema = in_flight_semaphore;
    [theCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
         dispatch_semaphore_signal(block_sema);
    }];

    // Create an array for delayed texture loading.
    theDelayLoadArray = [NSMutableArray array];

    // Create an array for textures to be destroyed.
    thePurgeArray = [NSMutableArray array];
    
    // Run a Suika2 frame event and do rendering.
    if(!runSuika2Frame())
        exit(0);

    // End an encoding.
    if (theBlitEncoder != nil)
        [theBlitEncoder endEncoding];
    if (theRenderEncoder != nil)
        [theRenderEncoder endEncoding];

    // Schedule a rendering to the current drawable.
    [theCommandBuffer presentDrawable:view.currentDrawable];
    
    // Push the command buffer to the GPU.
    [theCommandBuffer commit];
    
    // Synchronize.
    [theCommandBuffer waitUntilCompleted];
    
    // Set destroyed textures purgeable.
    for(id<MTLTexture> tex in thePurgeArray)
        [tex setPurgeableState:MTLPurgeableStateEmpty];
}

@end

//
// Suika2 HAL (an implementation of platform.h)
//

// Suika2 Base
#import "suika.h"
#import "uimsg.h"

// Standard C
#import <wchar.h>

// POSIX
#import <sys/time.h>

// Suika2 HAL implementation
#import "aunit.h"
#import "GameRenderer.h"

// Forward declarations
static FILE *openLog(void);
static NSString *NSStringFromWcs(const wchar_t *wcs);
static void drawPrimitives(int dst_left, int dst_top, struct image *src_image,
                           int width, int height, int src_left, int src_top, int alpha,
                           struct image *rule_image, id<MTLRenderPipelineState> pipeline);

//
// Run a Suika2 frame.
//
static BOOL runSuika2Frame(void)
{
    if(!on_event_frame()) {
        save_global_data();
        save_seen();
        return FALSE;
    }

    return TRUE;
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
    FILE *fp = openLog();
    if (fp != NULL) {
        fprintf(stderr, "%s", buf);
        fprintf(fp, "%s", buf);
        fflush(fp);
    }

    // アラートを表示する
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:NSStringFromWcs(get_ui_message(UIMSG_INFO))];
    NSString *text = [[NSString alloc] initWithUTF8String:buf];
    if (![text canBeConvertedToEncoding:NSUTF8StringEncoding])
        text = @"(invalid utf-8 string)";
    [alert setInformativeText:text];
    [alert runModal];

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
    FILE *fp = openLog();
    if (fp != NULL) {
        fprintf(stderr, "%s", buf);
        fprintf(fp, "%s", buf);
        fflush(fp);
    }

    // アラートを表示する
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:NSStringFromWcs(get_ui_message(UIMSG_WARN))];
    NSString *text = [[NSString alloc] initWithUTF8String:buf];
    if (![text canBeConvertedToEncoding:NSUTF8StringEncoding])
        text = @"(invalid utf-8 string)";
    [alert setInformativeText:text];
    [alert runModal];

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

    // ログファイルに出力する
    FILE *fp = openLog();
    if (fp != NULL) {
        fprintf(stderr, "%s", buf);
        fprintf(fp, "%s", buf);
        fflush(fp);
    }

    // アラートを表示する
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:NSStringFromWcs(get_ui_message(UIMSG_ERROR))];
    NSString *text = [[NSString alloc] initWithUTF8String:buf];
    if (![text canBeConvertedToEncoding:NSUTF8StringEncoding])
        text = @"(invalid utf-8 string)";
    [alert setInformativeText:text];
    [alert runModal];

    return true;
}

// ログをオープンする
static FILE *openLog(void)
{
    static FILE *fp = NULL;
    const char *cpath;

    // すでにオープン済みの場合、成功とする
    if (fp != NULL)
        return fp;

    // リリースモードの場合
    if (conf_release && conf_window_title != NULL) {
        // "Aplication Support"の下にウィンドウタイトルのフォルダを作成して、その下にログファイルを作成する
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
        fp = fopen(cpath, "w");
        if (fp == NULL) {
            NSAlert *alert = [[NSAlert alloc] init];
            [alert setMessageText:@"Error"];
            [alert setInformativeText:@"Cannot open log file."];
            [alert runModal];
        }
        return fp;
    }

    // .appバンドルのあるディレクトリにログファイルを作成する
    NSString *bundlePath = [[NSBundle mainBundle] bundlePath];
    NSString *basePath = [bundlePath stringByDeletingLastPathComponent];
    cpath = [[NSString stringWithFormat:@"%@/%s", basePath, LOG_FILE] UTF8String];
    fp = fopen(cpath, "w");
    if (fp == NULL) {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Error"];
        [alert setInformativeText:@"Cannot open log file."];
        [alert runModal];
    }
    return fp;
}

// ワイド文字列をNSStringに変換する
static NSString *NSStringFromWcs(const wchar_t *wcs)
{
    return [[NSString alloc] initWithBytes:wcs
                                    length:wcslen(wcs) * sizeof(*wcs)
                                  encoding:NSUTF32LittleEndianStringEncoding];
}

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
    return false;
}

//
// テクスチャをロックする
//
bool lock_texture(struct image *img)
{
    return true;
}

//
// テクスチャをアンロックする
//
void unlock_texture(struct image *img)
{
    if (theCommandBuffer == nil) {
        *locked_pixels = NULL;
        [theDelayLoadArray addObject:img];
        return;
    }

    MTLRegion region = {{ 0, 0, 0 },  {width, height, 1}};
    id<MTLTexture> tex = nil;

    if (*texture == NULL) {
        // For the first time, create a texture.
        MTLTextureDescriptor *textureDescriptor = [[MTLTextureDescriptor alloc] init];
        textureDescriptor.pixelFormat = MTLPixelFormatBGRA8Unorm;
        textureDescriptor.width = width;
        textureDescriptor.height = height;
        tex = [theDevice newTextureWithDescriptor:textureDescriptor];
        *texture = (__bridge void * _Nonnull)(tex);
        CFBridgingRetain(tex);
    } else {
        tex = (__bridge id<MTLTexture>)(*texture);
    }

    // Upload the pixels.
    assert(theRenderEncoder == nil);
    if (theBlitEncoder == nil) {
        theBlitEncoder = [theCommandBuffer blitCommandEncoder];
        theBlitEncoder.label = @"Texture Encoder";
    }
    [tex replaceRegion:region mipmapLevel:0 withBytes:*locked_pixels bytesPerRow:width * 4];
    [theBlitEncoder synchronizeResource:tex];
}

//
// テクスチャを破棄する
//
void destroy_texture(void *texture)
{
    if (texture != NULL) {
        id<MTLTexture> tex = (__bridge id<MTLTexture>)texture;
        assert(tex != nil);
        [thePurgeArray addObject:tex];
        CFBridgingRelease(texture);
    }
}

//
// Render an image to the screen with "copy" fragment shader.
//
void render_image_copy(int dst_left, int dst_top, struct image * RESTRICT src_image, int width, int height, int src_left, int src_top)
{
    drawPrimitives(dst_left, dst_top, src_image, width, height, src_left, src_top, alpha, NULL, theCopyPipelineState);
}

//
// Render an image to the screen.
//
void render_image_alpha(int dst_left, int dst_top, struct image * RESTRICT src_image, int width, int height, int src_left, int src_top, int alpha, int bt)
{
    id<MTLRenderPipelineState> pipeline;

    if (bt == BLEND_NONE) {
        alpha = 255;
        pipeline = theCopyPipelineState;
    } else {
        pipeline = theNormalPipelineState;
    }

    drawPrimitives(dst_left, dst_top, src_image, width, height, src_left, src_top, alpha, NULL,  pipeline);
}

//
// Render an image to the screen with dimming.
//
void render_image_dim(int dst_left, int dst_top,
                      struct image * RESTRICT src_image,
                      int width, int height, int src_left, int src_top)
{
    drawPrimitives(dst_left, dst_top, src_image, width, height, src_left, src_top, 0, NULL, theDimPipelineState);
}

//
// Render an image to the screen with 1-bit rule image.
//
void render_image_rule(struct image * RESTRICT src_img,
                       struct image * RESTRICT rule_img,
                       int threshold)
{
    drawPrimitives(0, 0, src_img, get_image_width(src_img), get_image_height(src_img), 0, 0, threshold, NULL, theRulePipelineState);
}

//
// Render an image to the screen with 8-bit rule image.
//
void render_image_melt(struct image * RESTRICT src_img,
                       struct image * RESTRICT rule_img,
                       int threshold)
{
    drawPrimitives(0, 0, src_img, get_image_width(src_img), get_image_height(src_img), 0, 0, threshold, NULL, theMeltPipelineState);
}

//
// Draw a rectangle with a specified pipeline.
//
static void drawPrimitives(int dst_left, int dst_top, struct image *src_image,
                           int width, int height, int src_left, int src_top, int alpha,
                           struct image *rule_image, id<MTLRenderPipelineState> pipeline)
{
    float pos[24];

    // Get the viewport size.
    float hw = (float)conf_window_width / 2.0f;
    float hh = (float)conf_window_height / 2.0f;

    // Get the texture size.
    float tw = (float)get_image_width(src_image);
    float th = (float)get_image_height(src_image);
    
    // Set the left top vertex.
    pos[0] = ((float)dst_left - hw) / hw;   // X (-1.0 to 1.0, left to right)
    pos[1] = -((float)dst_top - hh) / hh;   // Y (-1.0 to 1.0, bottom to top)
    pos[2] = (float)src_left / tw;          // U (0.0 to 1.0, left to right)
    pos[3] = (float)src_top / th;           // V (0.0 to 1.0, top to bottom)
    pos[4] = (float)alpha / 255.0f;         // Alpha (0.0 to 1.0)
    pos[5] = 0;                             // Padding for a 64-bit boundary

    // Set the right top vertex.
    pos[6] = ((float)dst_left + (float)width - hw) / hw;    // X (-1.0 to 1.0, left to right)
    pos[7] = -((float)dst_top - hh) / hh;                   // Y (-1.0 to 1.0, bottom to top)
    pos[8] = (float)(src_left + width) / tw;                // U (0.0 to 1.0, left to right)
    pos[9] = (float)(src_top) / th;                         // V (0.0 to 1.0, top to bottom)
    pos[10] = (float)alpha / 255.0f;                        // Alpha (0.0 to 1.0)
    pos[11] = 0;                                            // Padding for a 64-bit boundary
    
    // Set the left bottom vertex.
    pos[12] = ((float)dst_left - hw) / hw;                  // X (-1.0 to 1.0, left to right)
    pos[13] = -((float)dst_top + (float)height - hh) / hh;  // Y (-1.0 to 1.0, bottom to top)
    pos[14] = (float)src_left / tw;                         // U (0.0 to 1.0, left to right)
    pos[15] = (float)(src_top + height) / th;               // V (0.0 to 1.0, top to bottom)
    pos[16] = (float)alpha / 255.0f;                        // Alpha (0.0 to 1.0)
    pos[17] = 0;                                            // Padding for a 64-bit boundary

    // Set the right bottom vertex.
    pos[18] = ((float)dst_left + (float)width - hw) / hw;   // X (-1.0 to 1.0, left to right)
    pos[19] = -((float)dst_top + (float)height - hh) / hh;  // Y (-1.0 to 1.0, bottom to top)
    pos[20] = (float)(src_left + width) / tw;               // U (0.0 to 1.0, left to right)
    pos[21] = (float)(src_top + height) / th;               // V (0.0 to 1.0, top to bottom)
    pos[22] = (float)alpha / 255.0f;                        // Alpha (0.0 to 1.0)
    pos[23] = 0;                                            // Padding for a 64-bit boundary

    // Upload textures if they are pending.
    if (theBlitEncoder != nil) {
        [theBlitEncoder endEncoding];
        theBlitEncoder = nil;
    }

    // Draw two triangles.
    if (theRenderEncoder == nil) {
        theRenderEncoder = [theCommandBuffer renderCommandEncoderWithDescriptor:theMTKView.currentRenderPassDescriptor];
        theRenderEncoder.label = @"MyRenderEncoder";
    }
    [theRenderEncoder setRenderPipelineState:pipeline];
    id<MTLTexture> tex1 = (__bridge id<MTLTexture> _Nullable)(get_texture_object(src_image));
    id<MTLTexture> tex2 = rule_image != NULL ? (__bridge id<MTLTexture> _Nullable)(get_texture_object(src_image)) : nil;
    [theRenderEncoder setVertexBytes:pos length:sizeof(pos) atIndex:GameVertexInputIndexVertices];
    [theRenderEncoder setFragmentTexture:tex1 atIndex:GameTextureIndexBaseColor];
    if (tex2 != nil)
        [theRenderEncoder setFragmentTexture:tex2 atIndex:GameTextureIndexRuleLevel];
    [theRenderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
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
    // パスを生成する
    char *cpath = make_valid_path(MOV_DIR, fname);
    assert(cpath != NULL);
    NSString *path = [[NSString alloc] initWithUTF8String:cpath];
    free(cpath);
    path = [path stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLPathAllowedCharacterSet]];

    // ビデオを再生する
    [theViewController playVideoWithPath:path skippable:is_skippable ? YES : NO];

    return true;
}

//
// ビデオを停止する
//
void stop_video(void)
{
    [theViewController stopVideo];
}

//
// ビデオが再生中か調べる
//
bool is_video_playing(void)
{
    return [theViewController isVideoPlaying] ? true : false;
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
        //[theView.window setTitle:s];
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
    return [theViewController isFullScreen] ? true : false;
}

//
// フルスクリーンモードを開始する
//
void enter_full_screen_mode(void)
{
//    [theView enterFullScreen];
}

///
// フルスクリーンモードを終了する
//
void leave_full_screen_mode(void)
{
//    [theView leaveFullScreen];
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
