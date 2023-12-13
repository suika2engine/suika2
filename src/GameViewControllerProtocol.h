#import <Foundation/Foundation.h>

@protocol GameViewControllerProtocol <NSObject>

// Screen scaling
- (float)screenScale;
- (CGPoint)screenOffset;
- (CGSize)screenSize;

// Video playback implementation
- (BOOL)isVideoPlaying;
- (void)playVideoWithPath:(NSString *)path skippable:(BOOL)isSkippable;
- (void)stopVideo;

#ifdef SUIKA_TARGET_MACOS
// Set title.
- (void)setTitle:(NSString *)name;

// Point conversion.
- (NSPoint)windowPointToScreenPoint:(NSPoint)windowPoint;

// Full screen implementation
- (BOOL)isFullScreen;
- (void)enterFullScreen;
- (void)leaveFullScreen;
#endif

@end
