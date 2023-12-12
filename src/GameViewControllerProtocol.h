#import <Foundation/Foundation.h>

@protocol GameViewControllerProtocol <NSObject>

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
