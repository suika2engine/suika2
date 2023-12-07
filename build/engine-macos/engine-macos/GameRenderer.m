@import simd;
@import MetalKit;

#import "GameRenderer.h"
#import "GameShaderTypes.h"

static id<MTLDevice> theDevice;
static id<MTLRenderPipelineState> theNormalPipelineState;
static id<MTLRenderPipelineState> theCopyPipelineState;
static id<MTLRenderPipelineState> theDimPipelineState;
static id<MTLRenderPipelineState> theRulePipelineState;
static id<MTLRenderPipelineState> theMeltPipelineState;
static id<MTLCommandBuffer> theCommandBuffer;
static id<MTLBlitCommandEncoder> theBlitEncoder;
static id<MTLRenderCommandEncoder> theRenderEncoder;

static MTKView *theMTKView;
static NSSize theViewportSize;
static NSMutableArray *thePurgeArray;
static dispatch_semaphore_t in_flight_semaphore;

static BOOL runSuika2Frame(void);

@interface GameRenderer ()
@end

@implementation GameRenderer
{
    id<MTLCommandQueue> commandQueue;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView {
    NSError *error = NULL;

    self = [super init];
    if(self == nil)
        return nil;

    theDevice = mtkView.device;
    
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
    commandQueue = [theDevice newCommandQueue];

    in_flight_semaphore = dispatch_semaphore_create(1);

    return self;
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    theViewportSize.width = size.width;
    theViewportSize.height = size.height;
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    theMTKView = view;
    if(view.currentRenderPassDescriptor == nil)
        return;
    
    // Create a command buffer.
    theCommandBuffer = [commandQueue commandBuffer];
    theCommandBuffer.label = @"MyCommand";

    // Nil-ify the encoders.
    theBlitEncoder = nil;
    theRenderEncoder = nil;

    dispatch_semaphore_wait(in_flight_semaphore, DISPATCH_TIME_FOREVER);
    __block dispatch_semaphore_t block_sema = in_flight_semaphore;
    [theCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
         dispatch_semaphore_signal(block_sema);
    }];

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
bool lock_texture(int width, int height, pixel_t *pixels, pixel_t **locked_pixels, void **texture)
{
    assert(*locked_pixels == NULL);
    *locked_pixels = pixels;
    return true;
}

//
// テクスチャをアンロックする
//
void unlock_texture(int width, int height, pixel_t *pixels, pixel_t **locked_pixels, void **texture)
{
    if (theCommandBuffer == nil) {
        *locked_pixels = NULL;
        return;
    }

    MTLRegion region = {{ 0, 0, 0 },  {width, height, 1}};
    id<MTLTexture> tex = nil;

    if (*texture == NULL) {
        // For the first time, create a texture.
        MTLTextureDescriptor *textureDescriptor = [[MTLTextureDescriptor alloc] init];
        textureDescriptor.pixelFormat = MTLPixelFormatRGBA8Unorm;
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

    *locked_pixels = NULL;
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
// イメージをレンダリングする
//
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
                  int width, int height, int src_left, int src_top, int alpha,
                  int bt)
{
    float pos[24];

    // If the texture is not created.
    if (get_texture_object(src_image) == NULL) {
        pixel_t *locked_pixels = get_image_pixels(src_image);
        void *texobj = NULL;
        unlock_texture(get_image_width(src_image), get_image_height(src_image), get_image_pixels(src_image), &locked_pixels, &texobj);
        set_texture_object(src_image, texobj);
    }
    
    // Get the viewport size.
    float hw = (float)conf_window_width / 2.0f;
    float hh = (float)conf_window_height / 2.0f;

    // Get the texture size.
    float tw = (float)get_image_width(src_image);
    float th = (float)get_image_height(src_image);

    if (bt == BLEND_NONE)
        alpha = 255;
    
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

    // Upload textures.
    if (theBlitEncoder != nil) {
        [theBlitEncoder endEncoding];
        theBlitEncoder = nil;
    }

    // Draw.
    if (theRenderEncoder == nil) {
        theRenderEncoder = [theCommandBuffer renderCommandEncoderWithDescriptor:theMTKView.currentRenderPassDescriptor];
        theRenderEncoder.label = @"MyRenderEncoder";
    }
    id<MTLTexture> tex = (__bridge id<MTLTexture> _Nullable)(get_texture_object(src_image));
    if (bt == BLEND_NONE)
        [theRenderEncoder setRenderPipelineState:theCopyPipelineState];
    else
        [theRenderEncoder setRenderPipelineState:theNormalPipelineState];
    [theRenderEncoder setVertexBytes:pos length:sizeof(pos) atIndex:GameVertexInputIndexVertices];
    [theRenderEncoder setFragmentTexture:tex atIndex:GameTextureIndexBaseColor];
    [theRenderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
}

//
// イメージを暗くレンダリングする
//
void render_image_dim(int dst_left, int dst_top,
                      struct image * RESTRICT src_image,
                      int width, int height, int src_left, int src_top)
{
    float pos[20];

    // If the texture is not created.
    if (get_texture_object(src_image) == NULL) {
        pixel_t *locked_pixels = get_image_pixels(src_image);
        void *texobj = NULL;
        unlock_texture(get_image_width(src_image), get_image_height(src_image), get_image_pixels(src_image), &locked_pixels, &texobj);
        set_texture_object(src_image, texobj);
    }

    // Get the viewport size.
    float hw = (float)conf_window_width / 2.0f;
    float hh = (float)conf_window_height / 2.0f;

    // Get the texture size.
    float tw = (float)get_image_width(src_image);
    float th = (float)get_image_height(src_image);

    // Set the left top vertex.
    pos[0] = ((float)dst_left - hw) / hw;
    pos[1] = -((float)dst_top - hh) / hh;
    pos[2] = (float)src_left / tw;
    pos[3] = (float)src_top / th;
    pos[4] = 1.0f;

    // Set the right top vertex.
    pos[5] = ((float)dst_left + (float)width - hw) / hw;
    pos[6] = -((float)dst_top - hh) / hh;
    pos[7] = (float)(src_left + width) / tw;
    pos[8] = (float)(src_top) / th;
    pos[9] = 1.0f;

    // Set the left bottom vertex.
    pos[10] = ((float)dst_left - hw) / hw;
    pos[11] = -((float)dst_top + (float)height - hh) / hh;
    pos[12] = (float)src_left / tw;
    pos[13] = (float)(src_top + height) / th;
    pos[14] = 1.0f;

    // Set the right bottom vertex.
    pos[15] = ((float)dst_left + (float)width - hw) / hw;
    pos[16] = -((float)dst_top + (float)height - hh) / hh;
    pos[17] = (float)(src_left + width) / tw;
    pos[18] = (float)(src_top + height) / th;
    pos[19] = 1.0f;

    // Draw.
    id<MTLRenderCommandEncoder> renderEncoder = [theCommandBuffer renderCommandEncoderWithDescriptor:theMTKView.currentRenderPassDescriptor];
    renderEncoder.label = @"MyRenderEncoder";
    [renderEncoder setViewport:(MTLViewport){0.0, 0.0, theViewportSize.width, theViewportSize.height, -1.0, 1.0}];
    id<MTLTexture> tex = (__bridge id<MTLTexture> _Nullable)(get_texture_object(src_image));
    [renderEncoder setRenderPipelineState:theNormalPipelineState];
    [renderEncoder setVertexBytes:pos length:sizeof(pos) atIndex:0];
    [renderEncoder setFragmentTexture:tex atIndex:GameTextureIndexBaseColor];
    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [renderEncoder endEncoding];
}

//
// 画面にイメージをルール付きでレンダリングする
//
void render_image_rule(struct image * RESTRICT src_img,
                       struct image * RESTRICT rule_img,
                       int threshold)
{
    float pos[20];

    // If the textures are not created.
    if (get_texture_object(src_img) == NULL) {
        pixel_t *locked_pixels = get_image_pixels(src_img);
        void *texobj = NULL;
        unlock_texture(get_image_width(src_img), get_image_height(src_img), get_image_pixels(src_img), &locked_pixels, &texobj);
        set_texture_object(src_img, texobj);
    }
    if (get_texture_object(rule_img) == NULL) {
        pixel_t *locked_pixels = get_image_pixels(rule_img);
        void *texobj = NULL;
        unlock_texture(get_image_width(rule_img), get_image_height(rule_img), get_image_pixels(rule_img), &locked_pixels, &texobj);
        set_texture_object(rule_img, texobj);
    }

    // Get the image size.
    float width = get_image_width(src_img);
    float height = get_image_height(src_img);

    // Get the viewport size.
    float hw = (float)conf_window_width / 2.0f;
    float hh = (float)conf_window_height / 2.0f;

    // Get the texture size.
    float tw = (float)get_image_width(src_img);
    float th = (float)get_image_height(src_img);

    // Set the left top vertex.
    pos[0] = -1.0f;
    pos[1] = 1.0f;
    pos[2] = 0;
    pos[3] = 0;
    pos[4] = (float)threshold / 255.0f;

    // Set the right top vertex.
    pos[5] = (width - hw) / hw;
    pos[6] = 1.0f;
    pos[7] = width / tw;
    pos[8] = 0;
    pos[9] = (float)threshold / 255.0f;

    // Set the left bottom vertex.
    pos[10] = -1.0f;
    pos[11] = -(height - hh) / hh;
    pos[12] = 0;
    pos[13] = height / th;
    pos[14] = (float)threshold / 255.0f;

    // Set the right bottom vertex.
    pos[15] = (width - hw) / hw;
    pos[16] = -(height - hh) / hh;
    pos[17] = width / tw;
    pos[18] = height / th;
    pos[19] = (float)threshold / 255.0f;

    // Draw.
    id<MTLRenderCommandEncoder> renderEncoder = [theCommandBuffer renderCommandEncoderWithDescriptor:theMTKView.currentRenderPassDescriptor];
    renderEncoder.label = @"MyRenderEncoder";
    [renderEncoder setViewport:(MTLViewport){0.0, 0.0, theViewportSize.width, theViewportSize.height, -1.0, 1.0}];
    id<MTLTexture> tex1 = (__bridge id<MTLTexture> _Nullable)(get_texture_object(src_img));
    id<MTLTexture> tex2 = (__bridge id<MTLTexture> _Nullable)(get_texture_object(rule_img));
    [renderEncoder setRenderPipelineState:theRulePipelineState];
    [renderEncoder setVertexBytes:pos length:sizeof(pos) atIndex:GameVertexInputIndexVertices];
    [renderEncoder setFragmentTexture:tex1 atIndex:GameTextureIndexBaseColor];
    [renderEncoder setFragmentTexture:tex2 atIndex:GameTextureIndexRuleLevel];
    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [renderEncoder endEncoding];
}

//
// 画面にイメージをルール付き(メルト)でレンダリングする
//
void render_image_melt(struct image * RESTRICT src_img,
                       struct image * RESTRICT rule_img,
                       int threshold)
{
    float pos[20];

    // If the textures are not created.
    if (get_texture_object(src_img) == NULL) {
        pixel_t *locked_pixels = get_image_pixels(src_img);
        void *texobj = NULL;
        unlock_texture(get_image_width(src_img), get_image_height(src_img), get_image_pixels(src_img), &locked_pixels, &texobj);
        set_texture_object(src_img, texobj);
    }
    if (get_texture_object(rule_img) == NULL) {
        pixel_t *locked_pixels = get_image_pixels(rule_img);
        void *texobj = NULL;
        unlock_texture(get_image_width(rule_img), get_image_height(rule_img), get_image_pixels(rule_img), &locked_pixels, &texobj);
        set_texture_object(rule_img, texobj);
    }

    // Get the image size.
    float width = get_image_width(src_img);
    float height = get_image_height(src_img);

    // Get the viewport size.
    float hw = (float)conf_window_width / 2.0f;
    float hh = (float)conf_window_height / 2.0f;

    // Get the texture size.
    float tw = (float)get_image_width(src_img);
    float th = (float)get_image_height(src_img);

    // Set the left top vertex.
    pos[0] = -1.0f;
    pos[1] = 1.0f;
    pos[2] = 0;
    pos[3] = 0;
    pos[4] = (float)threshold / 255.0f;

    // Set the right top vertex.
    pos[5] = (width - hw) / hw;
    pos[6] = 1.0f;
    pos[7] = width / tw;
    pos[8] = 0;
    pos[9] = (float)threshold / 255.0f;

    // Set the left bottom vertex.
    pos[10] = -1.0f;
    pos[11] = -(height - hh) / hh;
    pos[12] = 0;
    pos[13] = height / th;
    pos[14] = (float)threshold / 255.0f;

    // Set the right bottom vertex.
    pos[15] = (width - hw) / hw;
    pos[16] = -(height - hh) / hh;
    pos[17] = width / tw;
    pos[18] = height / th;
    pos[19] = (float)threshold / 255.0f;

    // Draw.
    // Draw.
    id<MTLRenderCommandEncoder> renderEncoder = [theCommandBuffer renderCommandEncoderWithDescriptor:theMTKView.currentRenderPassDescriptor];
    renderEncoder.label = @"MyRenderEncoder";
    [renderEncoder setViewport:(MTLViewport){0.0, 0.0, theViewportSize.width, theViewportSize.height, -1.0, 1.0}];
    id<MTLTexture> tex1 = (__bridge id<MTLTexture> _Nullable)(get_texture_object(src_img));
    id<MTLTexture> tex2 = (__bridge id<MTLTexture> _Nullable)(get_texture_object(rule_img));
    [renderEncoder setRenderPipelineState:theNormalPipelineState];
    [renderEncoder setVertexBytes:pos length:sizeof(pos) atIndex:GameVertexInputIndexVertices];
    [renderEncoder setFragmentTexture:tex1 atIndex:GameTextureIndexBaseColor];
    [renderEncoder setFragmentTexture:tex2 atIndex:GameTextureIndexRuleLevel];
    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [renderEncoder endEncoding];
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
    //[theView playVideo:path skippable:is_skippable ? YES : NO];

    return true;
}

//
// ビデオを停止する
//
void stop_video(void)
{
    //[theView stopVideo];
}

//
// ビデオが再生中か調べる
//
bool is_video_playing(void)
{
    return false;
//    return [theView isMoviePlaying] ? true : false;
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
    return false;
//    return [theView isFullScreen];
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
