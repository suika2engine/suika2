// -*- tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changed]
 *  - 2021/08/25 Created.
 */

#import "OpenGLView.h"

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

    // レイヤをセットする
    _eaglLayer = (CAEAGLLayer*) self.layer;
    _eaglLayer.opaque = YES;
    _eaglLayer.drawableProperties =
        [NSDictionary dictionaryWithObjectsAndKeys:
                          [NSNumber numberWithBool:NO],
                      kEAGLDrawablePropertyRetainedBacking,
                      kEAGLColorFormatRGBA8,
                      kEAGLDrawablePropertyColorFormat,
                      nil];

    // コンテキストをセットする
    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:_context];

    // バッファを作成する
    [self setupBuffers];

    // 描画タイマをセットする
    _timer = [NSTimer scheduledTimerWithTimeInterval:1.0f/30.0f
                      target:self
                      selector:@selector(setNeedsDisplay)
                      userInfo:nil
                      repeats:YES];
}

- (void)layoutSubviews
{
    [EAGLContext setCurrentContext:_context];
    [self destroyBuffers];
    [self setupBuffers];
    [self render];
}

- (void)drawRect:(CGRect)rect
{
    [EAGLContext setCurrentContext:_context];
    [self render];
}

- (void)render
{
    static float f = 0.0f;

    glClearColor(0, 1.0, f, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    [_context presentRenderbuffer:GL_RENDERBUFFER];

    f += 0.01f;
    if(f > 1.0f)
        f = 0.0f;
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
    glDeleteFramebuffers(1, &_frameBuffer);
    glDeleteRenderbuffers(1, &_renderBuffer);
    _frameBuffer = 0;
    _renderBuffer = 0;
}

@end
