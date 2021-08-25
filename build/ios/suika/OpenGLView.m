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
#import "suika.h"
#import "glesrender.h"

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
//    self.opaque = NO;
//    self.backgroundColor = [UIColor clearColor];

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
//    const CGFloat black[] = {0.0, 0.0, 0.0, 0.0};
//    _eaglLayer.backgroundColor = CGColorCreate(CGColorSpaceCreateDeviceRGB(),
//                                               black);

    // コンテキストの設定を行う
    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:_context];

    // Suika2のOpenGL ESレンダラを初期化する
    init_opengl();
}

- (void)layoutSubviews
{
    [EAGLContext setCurrentContext:_context];
    [self destroyBuffers];
    [self setupBuffers];

    float aspect = (float)conf_window_height / (float)conf_window_width;
    float w = self.bounds.size.width * self.contentScaleFactor;
    float h = w * aspect;
    float y = (self.bounds.size.height * self.contentScaleFactor - h) / 2.0f;

    glViewport(0, y, w, h);

    _top = (int)y;
    _scale = (float)conf_window_width / w;
}

- (void)render
{
    opengl_start_rendering();

    int x, y, w, h;
    on_event_frame(&x, &y, &w, &h);

    opengl_end_rendering();

    [_context presentRenderbuffer:GL_RENDERBUFFER];
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
    _touchStartX = touchLocation.x * _scale;
    _touchStartY = touchLocation.y * _scale - _top;
    _touchLastY = _touchStartY;

    on_event_mouse_move((int)_touchStartX, (int)_touchStartY);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = [[event allTouches] anyObject];
    CGPoint touchLocation = [touch locationInView:self];

    float touchX = touchLocation.x * _scale;
    float touchY = touchLocation.y * _scale - _top;

    const float LINE_HEIGHT = 10;
    float delta = touchY - _touchLastY;
    if (delta > LINE_HEIGHT) {
        on_event_key_press(KEY_DOWN);
        on_event_key_release(KEY_DOWN);
    } else if (delta < -LINE_HEIGHT) {
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

    float touchEndX = touchLocation.x * _scale;
    float touchEndY = touchLocation.y * _scale - _top;

    if([touches count] == 1)
        on_event_mouse_press(MOUSE_LEFT, (int)touchEndX, (int)touchEndY);
    else
        on_event_mouse_press(MOUSE_RIGHT, (int)touchEndX, (int)touchEndY);

    _isTouch = NO;
}

@end
