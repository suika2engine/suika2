@import AppKit;
@import AVFoundation;

@interface ViewController : NSViewController <NSWindowDelegate>
@property AVPlayer *player;
@property AVPlayerLayer *playerLayer;
@property BOOL isFullscreen;
@end

void doFirstFrameInitialization(void);
