#import "GameView.h"
#import <AVFoundation/AVFoundation.h>

@implementation GameView
+ (Class)layerClass {
    return AVPlayerLayer.class;
}
@end
