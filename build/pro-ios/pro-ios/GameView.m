/* -*- coding: utf-8; indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

#import "GameView.h"
#import "GameViewControllerProtocol.h"

#import <AVFoundation/AVFoundation.h>

#import "suika.h"

static bool isContinuousSwipeEnabled;

@implementation GameView
{
    BOOL _isTouch;
    int _touchStartX;
    int _touchStartY;
    int _touchLastY;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    _isTouch = YES;

    UITouch *touch = [[event allTouches] anyObject];
    CGPoint touchLocation = [touch locationInView:self];
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

	// Emulate a wheel down.
	const int FLICK_Y_DISTANCE = 30;
	int deltaY = touchY - _touchLastY;
	_touchLastY = touchY;
    if (isContinuousSwipeEnabled) {
        if (deltaY > 0 && deltaY < FLICK_Y_DISTANCE) {
            on_event_key_press(KEY_DOWN);
            on_event_key_release(KEY_DOWN);
            return;
        }
    }

	// Emulate a mouse move.
    on_event_mouse_move((int)touchX, (int)touchY);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    _isTouch = NO;

    UITouch *touch = [[event allTouches] anyObject];
    CGPoint touchLocation = [touch locationInView:self];
    int touchEndX = (int)((touchLocation.x - self.left) * self.scale);
    int touchEndY = (int)((touchLocation.y - self.top) * self.scale);

    // Detect a down/up swipe.
	const int FLICK_Y_DISTANCE = 50;
	int deltaY = touchEndY - _touchStartY;
	if (deltaY > FLICK_Y_DISTANCE) {
        on_event_touch_cancel();
        on_event_swipe_down();
        return;
	} else if (deltaY < -FLICK_Y_DISTANCE) {
        on_event_touch_cancel();
        on_event_swipe_up();
        return;
    }

	// Emulate a left click.
	const int FINGER_DISTANCE = 10;
    if ([[event allTouches] count] == 1 &&
        abs(touchEndX - _touchStartX) < FINGER_DISTANCE &&
	    abs(touchEndY - _touchStartY) < FINGER_DISTANCE) {
        on_event_touch_cancel();
        on_event_mouse_press(MOUSE_LEFT, touchEndX, touchEndY);
        on_event_mouse_release(MOUSE_LEFT, touchEndX, touchEndY);
        return;
    }

    // Emulate a right click.
    if ([[event allTouches] count] == 2 &&
        abs(touchEndX - _touchStartX) < FINGER_DISTANCE &&
        abs(touchEndY - _touchStartY) < FINGER_DISTANCE) {
        on_event_touch_cancel();
        on_event_mouse_press(MOUSE_RIGHT, touchEndX, touchEndY);
        on_event_mouse_release(MOUSE_RIGHT, touchEndX, touchEndY);
        return;
    }

    // Cancel the touch move.
    on_event_touch_cancel();
}

@end

void set_continuous_swipe_enabled(bool is_enabled)
{
	isContinuousSwipeEnabled = is_enabled;
}
