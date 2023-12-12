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
        (viewController.isCommandPressed && event.keyCode == X))
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
