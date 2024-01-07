// -*- coding: utf-8; indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

#import "ScriptTextView.h"
#import "ViewController.h"

#import <objc/runtime.h>

@implementation ScriptTextView

- (void)keyDown:(NSEvent *)event {
    const int RETURN = 36;
    const int BACKSPACE = 51;
    const int DELETE = 117;
    const int V = 9;
    const int X = 7;
    const int D = 2;
    const int Z = 6;

    ViewController *viewController = (ViewController *)self.superview.window.contentViewController;

    if (viewController.isShiftPressed && event.keyCode == RETURN) {
        [viewController onScriptShiftEnter];
        return;
    }

    if (self.selectedRange.length >= 1 ||
        event.keyCode == BACKSPACE ||
        event.keyCode == DELETE ||
        event.keyCode == RETURN ||
        (viewController.isCommandPressed && event.keyCode == V) ||
        (viewController.isCommandPressed && event.keyCode == X) ||
        (viewController.isCommandPressed && event.keyCode == D) ||
        (viewController.isCommandPressed && event.keyCode == Z))
    {
        [viewController onScriptRangedChange];
    }

    [super keyDown:event];
}

- (void)textDidChange:(NSNotification *)notification {
    ViewController *viewController = (ViewController *)self.superview.window.contentViewController;
    [viewController onScriptChange];
}

@end
