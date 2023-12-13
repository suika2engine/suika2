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

- (void)mouseDown:(NSEvent *)event {
    id<GameViewControllerProtocol> viewController = [self viewControllerFrom:event];
    NSPoint point = [viewController windowPointToScreenPoint:[event locationInWindow]];
    on_event_mouse_press(MOUSE_LEFT, point.x, point.y);
}

- (void)mouseUp:(NSEvent *)theEvent {
    id<GameViewControllerProtocol> viewController = [self viewControllerFrom:theEvent];
    NSPoint point = [viewController windowPointToScreenPoint:[theEvent locationInWindow]];
    on_event_mouse_release(MOUSE_LEFT, point.x, point.y);
}

- (void)rightMouseDown:(NSEvent *)theEvent {
    id<GameViewControllerProtocol> viewController = [self viewControllerFrom:theEvent];
    NSPoint point = [viewController windowPointToScreenPoint:[theEvent locationInWindow]];
    on_event_mouse_press(MOUSE_RIGHT, point.x, point.y);
}

- (void)rightMouseUp:(NSEvent *)event {
    id<GameViewControllerProtocol> viewController = [self viewControllerFrom:event];
    NSPoint point = [viewController windowPointToScreenPoint:[event locationInWindow]];
    on_event_mouse_release(MOUSE_RIGHT, point.x, point.y);
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
