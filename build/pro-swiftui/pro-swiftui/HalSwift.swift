/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

/*
 * This is the SwiftUI implementation of the Suika2 HAL.
 */

import SwiftUI

//
// Swift part of HAL implementation
//
class HalSwift {
    static var callbacks = HalSwiftExports(
        logInfo: { (msg) in
            print(String(cString: msg))
        },
        logWarn: { (msg) in
            print(String(cString: msg))
        },
        logError: { (msg) in
            print(String(cString: msg))
        },
        makeSavDir: { () in
            var path = FileManager.default.currentDirectoryPath
            path.append("/")
            path.append(SAVE_DIR)
            do {
                try FileManager.default.createDirectory(atPath: path, withIntermediateDirectories: false)
            } catch let error {
                print(error)
            }
            return true
        },
        makeValidPath: { (dir, file, dst, len) in
            var path = FileManager.default.currentDirectoryPath
            if dir != nil {
                if file != nil {
                    path.append("/")
                    path.append(String(cString: dir!))
                    path.append("/")
                    path.append(String(cString: file!))
                } else {
                    path.append("/")
                    path.append(String(cString: dir!))
                }
            } else {
                if file != nil {
                    path.append("/")
                    path.append(String(cString: file!))
                } else {
                    // Use the base path.
                }
            }
            let cstr = path.utf8CString
            let addr = cstr.withUnsafeBytes { $0.baseAddress! }
            dst.copyMemory(from: addr, byteCount: min(len, cstr.count))
        },
        notifyImageUpdate: { (image) in
            Renderer.instance.notifyImageUpdate(image: UnsafeRawPointer(image))
        },
        notifyImageFree: { (image) in
            Renderer.instance.notifyImageUpdate(image: UnsafeRawPointer(image))
        },
        renderImageNormal: { (dstLeft, dstTop, dstWidth, dstHeight, image, srcLeft, srcTop, srcWidth, srcHeight, alpha) in
            Renderer.instance.renderImageNormal(dstLeft: Int(dstLeft),
                                                dstTop: Int(dstTop),
                                                dstWidth: Int(dstWidth),
                                                dstHeight: Int(dstHeight),
                                                image: UnsafeRawPointer(image),
                                                srcLeft: Int(srcLeft),
                                                srcTop: Int(srcTop),
                                                srcWidth: Int(srcWidth),
                                                srcHeight: Int(srcHeight),
                                                alpha: Int(alpha))
        },
        renderImageAdd: { (dstLeft, dstTop, dstWidth, dstHeight, image, srcLeft, srcTop, srcWidth, srcHeight, alpha) in
            Renderer.instance.renderImageAdd(dstLeft: Int(dstLeft),
                                             dstTop: Int(dstTop),
                                             dstWidth: Int(dstWidth),
                                             dstHeight: Int(dstHeight),
                                             image: UnsafeRawPointer(image),
                                             srcLeft: Int(srcLeft),
                                             srcTop: Int(srcTop),
                                             srcWidth: Int(srcWidth),
                                             srcHeight: Int(srcHeight),
                                             alpha: Int(alpha))
        },
        renderImageDim: { (dstLeft, dstTop, dstWidth, dstHeight, image, srcLeft, srcTop, srcWidth, srcHeight, alpha) in
            Renderer.instance.renderImageDim(dstLeft: Int(dstLeft),
                                             dstTop: Int(dstTop),
                                             dstWidth: Int(dstWidth),
                                             dstHeight: Int(dstHeight),
                                             image: UnsafeRawPointer(image),
                                             srcLeft: Int(srcLeft),
                                             srcTop: Int(srcTop),
                                             srcWidth: Int(srcWidth),
                                             srcHeight: Int(srcHeight),
                                             alpha: Int(alpha))
        },
        renderImageRule: { (srcImage, ruleImage, threshold) in
            Renderer.instance.renderImageRule(image: UnsafeRawPointer(srcImage),
                                              ruleImage: UnsafeRawPointer(ruleImage),
                                              threshold: Int(threshold))
        },
        renderImageMelt: { (srcImage, ruleImage, threshold) in
            Renderer.instance.renderImageRule(image: UnsafeRawPointer(srcImage),
                                              ruleImage: UnsafeRawPointer(ruleImage),
                                              threshold: Int(threshold))
        },
        renderImage3DNormal: { (x1, y1, x2, y2, x3, y3, x4, y4, image, srcLeft, srcTop, srcWidth, srcHeight, alpha)  in
            Renderer.instance.renderImage3DNormal(x1: x1,
                                                  y1: y1,
                                                  x2: x2,
                                                  y2: y2,
                                                  x3: x3,
                                                  y3: y3,
                                                  x4: x4,
                                                  y4: y4,
                                                  image: UnsafeRawPointer(image),
                                                  srcLeft: Int(srcLeft),
                                                  srcTop: Int(srcTop),
                                                  srcWidth: Int(srcWidth),
                                                  srcHeight: Int(srcHeight),
                                                  alpha: Int(alpha))
        },
        renderImage3DAdd: { (x1, y1, x2, y2, x3, y3, x4, y4, image, srcLeft, srcTop, srcWidth, srcHeight, alpha) in
            Renderer.instance.renderImage3DAdd(x1: x1,
                                               y1: y1,
                                               x2: x2,
                                               y2: y2,
                                               x3: x3,
                                               y3: y3,
                                               x4: x4,
                                               y4: y4,
                                               image: UnsafeRawPointer(image),
                                               srcLeft: Int(srcLeft),
                                               srcTop: Int(srcTop),
                                               srcWidth: Int(srcWidth),
                                               srcHeight: Int(srcHeight),
                                               alpha: Int(alpha))
        },
        playVideo: { (file, skippable) in
            //Controller.instance.playVideo(file: String(cString: file), skippable: skippable)
            return false
        },
        stopVideo: { () in
            //Controller.instance.stopVideo()
        },
        isVideoPlaying: { () in
            //Controller.instance.isVideoPlaying()
            return false
        },
        updateWindowTitle: { () in
            //Controller.instance.updateWindowTitle()
        },
        isFullScreenSupported: { () in
            //Controller.instance.isFullScreenSupported()
            return false
        },
        isFullScreenMode: { () in
            //Controller.instance.isFullScreenMode()
            return false
        },
        enterFullScreenMode: { () in
            //Controller.instance.enterFullScreenMode()
        },
        leaveFullScreenMode: { () in
            //Controller.instance.leaveFullScreenMode()
        },
        getSystemLocale: { (dst, len) in
            let code = "ja"
            let locale = code.utf8CString
            let addr = locale.withUnsafeBytes { $0.baseAddress! }
            dst.copyMemory(from: addr, byteCount: min(len, locale.count))
        },
        speakText: { (text) in
            if let t = text {
                //Controller.instance.speakText(text: String(cString: UnsafePointer<CChar>(t)))
            }
        },
        isContinuePushed: { () in
            //Controller.instance.isContinuePushed()
            return false
        },
        isNextPushed: { () in
            //Controller.instance.isNextPushed()
            return false
        },
        isStopPushed: { () in
            //Controller.instance.isStopPushed()
            return false
        },
        isScriptOpened: { () in
            //Controller.instance.isScriptOpened()
            return false
        },
        getOpenedScript: { (dst, len) in
            //let script = Controller.instance.getOpenedScript()
            let script = "".utf8CString
            let addr = script.withUnsafeBytes { $0.baseAddress! }
            dst.copyMemory(from: addr, byteCount: min(len, script.count))
        },
        isExecLineChanged: { () in
            return false
        },
        getChangedExecLine: { () in
            return 0
        },
        onChangeRunningState: { (running, requestStop) in
        },
        onLoadScript: { () in
        },
        onChangePosition: { () in
        },
        onUpdateVariable: { () in
        }
    )
}
