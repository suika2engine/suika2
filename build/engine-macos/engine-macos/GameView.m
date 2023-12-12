#import "GameView.h"
#import "GameViewControllerProtocol.h"

#import <AVFoundation/AVFoundation.h>

#import "suika.h"

@implementation GameView
+ (Class)layerClass {
    return AVPlayerLayer.class;
}

- (id<GameViewControllerProtocol>) viewControllerFrom:(NSEvent *)event {
    NSObject *viewController = event.window.contentViewController;
    if ([viewController conformsToProtocol:@protocol(GameViewControllerProtocol)])
        return (NSObject<GameViewControllerProtocol> *)viewController;
    return nil;
}

- (void)mouseDown:(NSEvent *)theEvent {
    id<GameViewControllerProtocol> viewController = [self viewControllerFrom:theEvent];
    NSPoint point = [theEvent locationInWindow];
    float scale = [viewController screenScale];
    NSPoint offset = [viewController screenOffset];
    int x = (int)((point.x - offset.x) / scale);
    int y = (int)((point.y - offset.y) / scale);
    on_event_mouse_press(MOUSE_LEFT, x, conf_window_height - y);
}

- (void)mouseUp:(NSEvent *)theEvent {
    id<GameViewControllerProtocol> viewController = [self viewControllerFrom:theEvent];
    NSPoint point = [theEvent locationInWindow];
    float scale = [viewController screenScale];
    NSPoint offset = [viewController screenOffset];
    int x = (int)((point.x - offset.x) * scale);
    int y = (int)((point.y - offset.y) * scale);
    on_event_mouse_release(MOUSE_LEFT, x, conf_window_height - y);
}

- (void)rightMouseDown:(NSEvent *)theEvent {
    id<GameViewControllerProtocol> viewController = [self viewControllerFrom:theEvent];
    NSPoint point = [theEvent locationInWindow];
    float scale = [viewController screenScale];
    NSPoint offset = [viewController screenOffset];
    int x = (int)((point.x - offset.x) * scale);
    int y = (int)((point.y - offset.y) * scale);
    on_event_mouse_press(MOUSE_RIGHT, x, conf_window_height - y);
}

- (void)rightMouseUp:(NSEvent *)theEvent {
    id<GameViewControllerProtocol> viewController = [self viewControllerFrom:theEvent];
    NSPoint point = [theEvent locationInWindow];
    float scale = [viewController screenScale];
    NSPoint offset = [viewController screenOffset];
    int x = (int)((point.x - offset.x) * scale);
    int y = (int)((point.y - offset.y) * scale);
    on_event_mouse_release(MOUSE_RIGHT, x, conf_window_height - y);
}

- (void)mouseDragged:(NSEvent *)theEvent {
    id<GameViewControllerProtocol> viewController = [self viewControllerFrom:theEvent];
    NSPoint point = [theEvent locationInWindow];
    float scale = [viewController screenScale];
    NSPoint offset = [viewController screenOffset];
    int x = (int)((point.x - offset.x) * scale);
    int y = (int)((point.x - offset.y) * scale);
    on_event_mouse_move(x, conf_window_height - y);
}

- (void)scrollWheel:(NSEvent *)theEvent {
    int delta = [theEvent deltaY];
    if (delta > 0) {
        on_event_key_press(KEY_UP);
        on_event_key_release(KEY_UP);
    } else if (delta < 0) {
        on_event_key_press(KEY_DOWN);
        on_event_key_release(KEY_DOWN);
    }
}

@end
