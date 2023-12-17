@import UIKit;
@import AVFoundation;

#import "GameViewControllerProtocol.h"

@interface ViewController : UIViewController <UITextViewDelegate, GameViewControllerProtocol>

@property BOOL isControlPressed;
@property BOOL isShiftPressed;
@property BOOL isCommandPressed;

- (void)onScriptShiftEnter;
- (void)onScriptRangedChange;
- (void)onScriptChange;

@end
