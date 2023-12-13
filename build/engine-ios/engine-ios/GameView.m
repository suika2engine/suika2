#import "GameView.h"
#import "GameViewControllerProtocol.h"

#import <AVFoundation/AVFoundation.h>

#import "suika.h"

@implementation GameView
{
    BOOL _isTouch;
    int _touchStartX;
    int _touchStartY;
    int _touchLastY;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch *touch = [[event allTouches] anyObject];
    CGPoint touchLocation = [touch locationInView:self];

    _isTouch = YES;
    _touchStartX = (int)((touchLocation.x - self.left) * self.scale);
    _touchStartY = (int)((touchLocation.y - self.top) * self.scale);
    _touchLastY = _touchStartY;

    on_event_mouse_press(MOUSE_LEFT, _touchStartX, _touchStartY);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch *touch = [[event allTouches] anyObject];
    CGPoint touchLocation = [touch locationInView:self];

    int touchX = (int)((touchLocation.x - self.left) * self.scale);
    int touchY = (int)((touchLocation.y - self.top) * self.scale);

    const float LINE_HEIGHT = 10;
    float delta = touchY - _touchLastY;
    if(delta > LINE_HEIGHT) {
        on_event_key_press(KEY_DOWN);
        on_event_key_release(KEY_DOWN);
    } else if(delta < -LINE_HEIGHT) {
        on_event_key_press(KEY_UP);
        on_event_key_release(KEY_UP);
    }

    _touchLastY = touchY;

    on_event_mouse_move((int)touchX, (int)touchY);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch *touch = [[event allTouches] anyObject];
    CGPoint touchLocation = [touch locationInView:self];

    int touchEndX = (int)((touchLocation.x - self.left) * self.scale);
    int touchEndY = (int)((touchLocation.y - self.top) * self.scale);

    if([[event allTouches] count] == 1) {
        on_event_mouse_release(MOUSE_LEFT, touchEndX, touchEndY);
    } else {
        on_event_mouse_press(MOUSE_RIGHT, touchEndX, touchEndY);
        on_event_mouse_release(MOUSE_RIGHT, touchEndX, touchEndY);
    }

    _isTouch = NO;
}

@end
