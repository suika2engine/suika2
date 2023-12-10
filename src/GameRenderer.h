@import MetalKit;

#import "GameViewControllerProtocol.h"

@interface GameRenderer : NSObject<MTKViewDelegate>
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView andController:(nonnull id<GameViewControllerProtocol>)controller;
@end
