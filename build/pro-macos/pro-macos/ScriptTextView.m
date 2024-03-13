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

    ViewController *viewController = (ViewController *)self.superview.window.contentViewController;

    if (!viewController.isShiftPressed && event.keyCode == RETURN) {
        [viewController onScriptEnter];
        return;
    }

    [super keyDown:event];
}

- (void)textDidChange:(NSNotification *)notification {
    ViewController *viewController = (ViewController *)self.superview.window.contentViewController;
    [viewController onScriptChange];
}



@end
