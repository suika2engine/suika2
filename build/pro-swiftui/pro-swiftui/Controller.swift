import SwiftUI
import CoreGraphics

class Controller {
    static var instance: Controller!

    init() {
        Controller.instance = self
    }

    func screenScale() -> Float {
        return 1.0
    }

    func screenOffset() -> CGPoint {
        return CGPointMake(0, 0)
    }

    func screenSize() -> CGSize {
        return CGSizeMake(100, 100)
    }

    func windowPointToScreenPoint(windowPoint: CGPoint) -> CGPoint {
        return windowPoint
    }

    func isVideoPlaying() -> Bool {
        return false
    }

    func playVideoWithPath(path: String, skippable: Bool) -> Void {
    }

    func stopVideo() -> Void {
    }

    func setTitle(name: String) -> Void {
    }

    func isFullScreen() -> Bool {
        return false
    }

    func enterFullScreen() -> Void {
    }

    func leaveFullScreen() -> Void {
    }
}
