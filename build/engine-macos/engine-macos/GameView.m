#import "GameView.h"
#import <AVFoundation/AVFoundation.h>

@implementation GameView
+ (Class)layerClass {
    return AVPlayerLayer.class;
}

// マウス押下イベント
- (void)mouseDown:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= theViewController._screenScale;
    pos.y *= theViewController._screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

    on_event_mouse_press(MOUSE_LEFT, (int)pos.x, conf_window_height - (int)pos.y);
}

// マウス解放イベント
- (void)mouseUp:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= _screenScale;
    pos.y *= _screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

    on_event_mouse_release(MOUSE_LEFT, (int)pos.x, conf_window_height - (int)pos.y);
}

// マウス右ボタン押下イベント
- (void)rightMouseDown:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= _screenScale;
    pos.y *= _screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

    on_event_mouse_press(MOUSE_RIGHT, (int)pos.x, conf_window_height - (int)pos.y);
}

// マウス右ボタン解放イベント
- (void)rightMouseUp:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= _screenScale;
    pos.y *= _screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

    on_event_mouse_release(MOUSE_RIGHT, (int)pos.x, conf_window_height - (int)pos.y);
}

// マウス移動イベント
- (void)mouseMoved:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= _screenScale;
    pos.y *= _screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

    on_event_mouse_move((int)pos.x, conf_window_height - (int)pos.y);
}

// マウスドラッグイベント
- (void)mouseDragged:(NSEvent *)theEvent {
    NSPoint pos = [theEvent locationInWindow];
    pos.x *= _screenScale;
    pos.y *= _screenScale;
    if (pos.x < 0 && pos.x >= conf_window_width)
        return;
    if (pos.y < 0 && pos.y >= conf_window_height)
        return;

    on_event_mouse_move((int)pos.x, conf_window_height - (int)pos.y);
}

// マウスホイールイベント
- (void)scrollWheel:(NSEvent *)theEvent {
    if ([theEvent deltaY] > 0) {
        on_event_key_press(KEY_UP);
        on_event_key_release(KEY_UP);
    } else if ([theEvent deltaY] < 0) {
        on_event_key_press(KEY_DOWN);
        on_event_key_release(KEY_DOWN);
    }
}

@end
