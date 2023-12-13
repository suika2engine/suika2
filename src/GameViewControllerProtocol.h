#import <Foundation/Foundation.h>

@protocol GameViewControllerProtocol <NSObject>

// Set title.
- (void)setTitle:(NSString *)name;

// Point conversion.
- (NSPoint)windowPointToScreenPoint:(NSPoint)windowPoint;

// Screen scaling
- (float)screenScale;
- (NSPoint)screenOffset;
- (NSSize)screenSize;

// Full screen implementation
- (BOOL)isFullScreen;
- (void)enterFullScreen;
- (void)leaveFullScreen;

// Video playback implementation
- (BOOL)isVideoPlaying;
- (void)playVideoWithPath:(NSString *)path skippable:(BOOL)isSkippable;
- (void)stopVideo;

@end
