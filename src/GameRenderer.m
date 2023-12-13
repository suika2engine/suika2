// -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*-

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

//
// GameRenderer: The Suika2 HAL for Apple macOS/iOS/iPadOS with Metal
//

@import simd;
@import MetalKit;

#import "GameRenderer.h"
#import "GameShaderTypes.h"
#import "GameViewControllerProtocol.h"

// Suika2 Base
#import "suika.h"

// POSIX
#import <sys/time.h>

// Suika2 HAL implementation
#import "aunit.h"

//
// GameView Objects (accessed by the HAL functions)
//
static id<GameViewControllerProtocol> theViewController;
static MTKView *theMTKView;
static id<MTLDevice> theDevice;
static id<MTLRenderPipelineState> theNormalPipelineState;
static id<MTLRenderPipelineState> theCopyPipelineState;
static id<MTLRenderPipelineState> theAddPipelineState;
static id<MTLRenderPipelineState> theDimPipelineState;
static id<MTLRenderPipelineState> theRulePipelineState;
static id<MTLRenderPipelineState> theMeltPipelineState;
static id<MTLCommandBuffer> theCommandBuffer;
static id<MTLBlitCommandEncoder> theBlitEncoder;
static id<MTLRenderCommandEncoder> theRenderEncoder;
static struct image *theInitialUploadArray[128];
static int theInitialUploadArrayCount;
static id<MTLTexture> thePurgeArray[128];
static int thePurgeArrayCount;
static dispatch_semaphore_t in_flight_semaphore;

//
// Forward declarations
//
static BOOL runSuika2Frame(void);
static FILE *openLog(void);
static void drawPrimitives(int dst_left, int dst_top, struct image *src_image,
                           int width, int height, int src_left, int src_top, int alpha,
                           struct image *rule_image, id<MTLRenderPipelineState> pipeline);

//
// GameRender
//

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

    // Construct the normal shader pipeline.
    MTLRenderPipelineDescriptor *normalPipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    normalPipelineStateDescriptor.label = @"Normal Texturing Pipeline";
    normalPipelineStateDescriptor.vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
    normalPipelineStateDescriptor.fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentNormalShader"];
    normalPipelineStateDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat;
    normalPipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
    normalPipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    normalPipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    normalPipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    normalPipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    normalPipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    normalPipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor =  MTLBlendFactorOne;
    theNormalPipelineState = [theDevice newRenderPipelineStateWithDescriptor:normalPipelineStateDescriptor error:&error];
    NSAssert(theNormalPipelineState, @"Failed to create pipeline state: %@", error);

    // Construct the copy shader pipeline.
    MTLRenderPipelineDescriptor *copyPipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    copyPipelineStateDescriptor.label = @"Copy Texturing Pipeline";
    copyPipelineStateDescriptor.vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
    copyPipelineStateDescriptor.fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentCopyShader"];
    copyPipelineStateDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat;
    copyPipelineStateDescriptor.colorAttachments[0].blendingEnabled = FALSE;
    theCopyPipelineState = [theDevice newRenderPipelineStateWithDescriptor:copyPipelineStateDescriptor error:&error];
    NSAssert(theCopyPipelineState, @"Failed to create pipeline state: %@", error);

    // Construct the add shader pipeline.
    MTLRenderPipelineDescriptor *addPipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    addPipelineStateDescriptor.label = @"Normal Texturing Pipeline";
    addPipelineStateDescriptor.vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
    addPipelineStateDescriptor.fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentNormalShader"];
    addPipelineStateDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat;
    addPipelineStateDescriptor.colorAttachments[0].blendingEnabled = TRUE;
    addPipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    addPipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    addPipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    addPipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    addPipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    addPipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor =  MTLBlendFactorOne;
    theAddPipelineState = [theDevice newRenderPipelineStateWithDescriptor:addPipelineStateDescriptor error:&error];
    NSAssert(theAddPipelineState, @"Failed to create pipeline state: %@", error);

    // Construct a dim shader pipeline.
    MTLRenderPipelineDescriptor *dimPipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    dimPipelineStateDescriptor.label = @"Dim Texturing Pipeline";
    dimPipelineStateDescriptor.vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
    dimPipelineStateDescriptor.fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentDimShader"];
    dimPipelineStateDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat;
    dimPipelineStateDescriptor.colorAttachments[0].blendingEnabled = TRUE;
    dimPipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    dimPipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    dimPipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
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
    rulePipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    rulePipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    rulePipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    rulePipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;
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
    meltPipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    meltPipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    meltPipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    meltPipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;
    theMeltPipelineState = [theDevice newRenderPipelineStateWithDescriptor:meltPipelineStateDescriptor error:&error];
    NSAssert(theMeltPipelineState, @"Failed to create pipeline state: %@", error);

    // Create a command queue.
    _commandQueue = [theDevice newCommandQueue];

    // TODO:
    in_flight_semaphore = dispatch_semaphore_create(1);

    return self;
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
    for (int i = 0; i < theInitialUploadArrayCount; i++)
        if (theInitialUploadArray[i] != NULL)
            notify_image_update(theInitialUploadArray[i]);

    dispatch_semaphore_wait(in_flight_semaphore, DISPATCH_TIME_FOREVER);
    __block dispatch_semaphore_t block_sema = in_flight_semaphore;
    [theCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
         dispatch_semaphore_signal(block_sema);
    }];

    // Create an array for textures to be destroyed.
    thePurgeArrayCount = 0;
    
    // Run a Suika2 frame event and do rendering.
    if(!runSuika2Frame())
        exit(0);

    // End encodings.
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
    for (int i = 0; i < thePurgeArrayCount; i++) {
        if (thePurgeArray[i] == NULL)
            continue;
        [thePurgeArray[i] setPurgeableState:MTLPurgeableStateEmpty];
        thePurgeArray[i] = NULL;
    }
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
}

@end

//
// HAL
//

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
    [alert setMessageText:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_INFO)]];
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
    [alert setMessageText:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_WARN)]];
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
    [alert setMessageText:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_ERROR)]];
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

//
// テクスチャの更新を通知する
//
void notify_image_update(struct image *img)
{
    if (theCommandBuffer == nil) {
        assert(theInitialUploadArrayCount < 128);
        assert(img->width > 0 && img->width < 4096);
        assert(img->height > 0 && img->height < 4096);
        theInitialUploadArray[theInitialUploadArrayCount++] = img;
        return;
    }

    MTLRegion region = {{ 0, 0, 0 }, {img->width, img->height, 1}};
    id<MTLTexture> texture = nil;
    
    if (img->texture == NULL) {
        // For the first time, create a texture.
        MTLTextureDescriptor *textureDescriptor = [[MTLTextureDescriptor alloc] init];
        textureDescriptor.pixelFormat = MTLPixelFormatBGRA8Unorm;
        textureDescriptor.width = img->width;
        textureDescriptor.height = img->height;
        texture = [theDevice newTextureWithDescriptor:textureDescriptor];
        img->texture = (__bridge void * _Nonnull)(texture);
        CFBridgingRetain(texture);
    } else {
        // Get the existing texture.
        texture = (__bridge id<MTLTexture>)(img->texture);
    }
    
    // Upload the pixels.
    assert(theRenderEncoder == nil);
    if (theBlitEncoder == nil) {
        theBlitEncoder = [theCommandBuffer blitCommandEncoder];
        theBlitEncoder.label = @"Texture Encoder";
    }
    [texture replaceRegion:region mipmapLevel:0 withBytes:img->pixels bytesPerRow:img->width * 4];
    [theBlitEncoder synchronizeResource:texture];
}

//
// テクスチャの破棄を通知する
//
void notify_image_free(struct image *img)
{
    // Metal初期化前に作成され、Metal初期化前に削除されるイメージを、theInitialUploadArrayから削除する
    for (int i = 0; i < theInitialUploadArrayCount; i++) {
        if (theInitialUploadArray[i] == img) {
            theInitialUploadArray[i] = NULL;
            return;
        }
    }
    
    thePurgeArray[thePurgeArrayCount++] = (__bridge id<MTLTexture>)(img->texture);
    CFBridgingRelease(img->texture);
}

//
// Render an image to the screen with the "copy" pipeline.
//
void render_image_copy(int dst_left, int dst_top, struct image *src_image, int width, int height, int src_left, int src_top)
{
    drawPrimitives(dst_left, dst_top, src_image, width, height, src_left, src_top, 255, NULL, theCopyPipelineState);
}

//
// Render an image to the screen with the "normal" pipeline.
//
void render_image_normal(int dst_left, int dst_top, struct image *src_image, int width, int height, int src_left, int src_top, int alpha)
{
    drawPrimitives(dst_left, dst_top, src_image, width, height, src_left, src_top, alpha, NULL, theNormalPipelineState);
}

//
// Render an image to the screen with the "add" pipeline.
//
void render_image_add(int dst_left, int dst_top, struct image *src_image, int width, int height, int src_left, int src_top, int alpha)
{
    // TODO: add
    drawPrimitives(dst_left, dst_top, src_image, width, height, src_left, src_top, alpha, NULL, theAddPipelineState);
}

//
// Render an image to the screen with the "dim" pipeline.
//
void render_image_dim(int dst_left, int dst_top,
                      struct image *src_image,
                      int width, int height, int src_left, int src_top)
{
    drawPrimitives(dst_left, dst_top, src_image, width, height, src_left, src_top, 0, NULL, theDimPipelineState);
}

//
// Render an image to the screen with the "rule" pipeline.
//
void render_image_rule(struct image *src_img, struct image *rule_img, int threshold)
{
    drawPrimitives(0, 0, src_img, src_img->width, src_img->height, 0, 0, threshold, rule_img, theRulePipelineState);
}

//
// Render an image to the screen with the "melt" pipeline.
//
void render_image_melt(struct image *src_img, struct image *rule_img, int threshold)
{
    drawPrimitives(0, 0, src_img, src_img->width, src_img->height, 0, 0, threshold, rule_img, theMeltPipelineState);
}

//
// Draw a rectangle with a specified pipeline.
//
static void drawPrimitives(int dst_left, int dst_top, struct image *src_image,
                           int width, int height, int src_left, int src_top, int alpha,
                           struct image *rule_image, id<MTLRenderPipelineState> pipeline)
{
    // Calc the half size of the window.
    float hw = (float)conf_window_width / 2.0f;
    float hh = (float)conf_window_height / 2.0f;

    // Get the texture size.
    float tw = (float)src_image->width;
    float th = (float)src_image->height;
    
    // The vertex shader input
    float vsIn[24];

    // Set the left top vertex.
    vsIn[0] = ((float)dst_left - hw) / hw;   // X (-1.0 to 1.0, left to right)
    vsIn[1] = -((float)dst_top - hh) / hh;   // Y (-1.0 to 1.0, bottom to top)
    vsIn[2] = (float)src_left / tw;          // U (0.0 to 1.0, left to right)
    vsIn[3] = (float)src_top / th;           // V (0.0 to 1.0, top to bottom)
    vsIn[4] = (float)alpha / 255.0f;         // Alpha (0.0 to 1.0)
    vsIn[5] = 0;                             // Padding for a 64-bit boundary

    // Set the right top vertex.
    vsIn[6] = ((float)dst_left + (float)width - hw) / hw;    // X (-1.0 to 1.0, left to right)
    vsIn[7] = -((float)dst_top - hh) / hh;                   // Y (-1.0 to 1.0, bottom to top)
    vsIn[8] = (float)(src_left + width) / tw;                // U (0.0 to 1.0, left to right)
    vsIn[9] = (float)(src_top) / th;                         // V (0.0 to 1.0, top to bottom)
    vsIn[10] = (float)alpha / 255.0f;                        // Alpha (0.0 to 1.0)
    vsIn[11] = 0;                                            // Padding for a 64-bit boundary
    
    // Set the left bottom vertex.
    vsIn[12] = ((float)dst_left - hw) / hw;                  // X (-1.0 to 1.0, left to right)
    vsIn[13] = -((float)dst_top + (float)height - hh) / hh;  // Y (-1.0 to 1.0, bottom to top)
    vsIn[14] = (float)src_left / tw;                         // U (0.0 to 1.0, left to right)
    vsIn[15] = (float)(src_top + height) / th;               // V (0.0 to 1.0, top to bottom)
    vsIn[16] = (float)alpha / 255.0f;                        // Alpha (0.0 to 1.0)
    vsIn[17] = 0;                                            // Padding for a 64-bit boundary

    // Set the right bottom vertex.
    vsIn[18] = ((float)dst_left + (float)width - hw) / hw;   // X (-1.0 to 1.0, left to right)
    vsIn[19] = -((float)dst_top + (float)height - hh) / hh;  // Y (-1.0 to 1.0, bottom to top)
    vsIn[20] = (float)(src_left + width) / tw;               // U (0.0 to 1.0, left to right)
    vsIn[21] = (float)(src_top + height) / th;               // V (0.0 to 1.0, top to bottom)
    vsIn[22] = (float)alpha / 255.0f;                        // Alpha (0.0 to 1.0)
    vsIn[23] = 0;                                            // Padding for a 64-bit boundary

    // Upload textures if they are pending.
    if (theBlitEncoder != nil) {
        [theBlitEncoder endEncoding];
        theBlitEncoder = nil;
    }

    // Draw two triangles.
    if (theRenderEncoder == nil) {
        theRenderEncoder = [theCommandBuffer renderCommandEncoderWithDescriptor:theMTKView.currentRenderPassDescriptor];
        theRenderEncoder.label = @"MyRenderEncoder";

        MTLViewport viewport;
        viewport.originX = [theViewController screenOffset].x;
        viewport.originY = [theViewController screenOffset].y;
        viewport.width = [theViewController screenSize].width;
        viewport.height = [theViewController screenSize].height;
        viewport.zfar = 0;
        viewport.znear = 0;
        [theRenderEncoder setViewport:viewport];
    }
    [theRenderEncoder setRenderPipelineState:pipeline];
    id<MTLTexture> tex1 = (__bridge id<MTLTexture> _Nullable)(src_image->texture);
    id<MTLTexture> tex2 = rule_image != NULL ? (__bridge id<MTLTexture> _Nullable)(rule_image->texture) : nil;
    [theRenderEncoder setVertexBytes:vsIn length:sizeof(vsIn) atIndex:GameVertexInputIndexVertices];
    [theRenderEncoder setFragmentTexture:tex1 atIndex:GameTextureIndexColor];
    if (tex2 != nil)
        [theRenderEncoder setFragmentTexture:tex2 atIndex:GameTextureIndexRule];
    [theRenderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
}

//
// タイマをリセットする
//
void reset_lap_timer(uint64_t *origin)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    *origin = (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

//
// タイマのラップをミリ秒単位で取得する
//
uint64_t get_lap_timer_millisec(uint64_t *origin)
{
    struct timeval tv;
    uint64_t now;

    gettimeofday(&tv, NULL);
    now = (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
    if (now < *origin) {
        reset_lap_timer(origin);
        return 0;
    }
    return (int)(now - *origin);
}

//
// 終了ダイアログを表示する
//
bool exit_dialog(void)
{
    @autoreleasepool {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_YES)]];
        [alert addButtonWithTitle:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_NO)]];
        [alert setMessageText:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_EXIT)]];
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
        [alert addButtonWithTitle:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_YES)]];
        [alert addButtonWithTitle:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_NO)]];
        [alert setMessageText:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_TITLE)]];
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
        [alert addButtonWithTitle:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_YES)]];
        [alert addButtonWithTitle:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_NO)]];
        [alert setMessageText:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_DELETE)]];
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
        [alert addButtonWithTitle:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_YES)]];
        [alert addButtonWithTitle:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_NO)]];
        [alert setMessageText:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_OVERWRITE)]];
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
        [alert addButtonWithTitle:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_YES)]];
        [alert addButtonWithTitle:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_NO)]];
        [alert setMessageText:[[NSString alloc] initWithUTF8String:get_ui_message(UIMSG_DEFAULT)]];
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
        NSString *windowTitle = [[NSString alloc] initWithUTF8String:conf_window_title];

        // 章タイトルを取得する
        NSString *chapterTitle = [[NSString alloc] initWithUTF8String:get_chapter_name()];

        // セパレータを取得する
        NSString *sep;
        if (conf_window_title_separator == NULL) {
            sep = @" ";
        } else {
            sep = [[NSString alloc] initWithUTF8String:conf_window_title_separator];
        }

        // タイトルを連結する
        NSString *s = [windowTitle stringByAppendingString:sep];
        s = [s stringByAppendingString:chapterTitle];

        // ウィンドウのタイトルを設定する
        [theViewController setTitle:s];
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

void speak_text(const char *text)
{
    // TODO: Use NSSpeechSynthesizer
}
