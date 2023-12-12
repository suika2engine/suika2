@import AppKit;
@import AVFoundation;

#import "GameViewControllerProtocol.h"

@interface ViewController : NSViewController <NSWindowDelegate, NSTextViewDelegate, GameViewControllerProtocol>

@property BOOL isControlPressed;
@property BOOL isShiftPressed;
@property BOOL isCommandPressed;

- (void)onScriptShiftEnter;
- (void)onScriptRangedChange;
- (void)onScriptChange;

@end
