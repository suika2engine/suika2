/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

//
// MetalViewCoordinator is the renderer for MetalView and it implements Suika2 HAL.
//

import Foundation
import MetalKit

class Renderer : NSObject, MTKViewDelegate {
    // The sole instance of this class.
    static var instance: Renderer!
    
    // Mutual exclusion for rendering.
    static var in_flight_semaphore = DispatchSemaphore(value: 1)
    
    // Metal objects.
    var mtkView: MTKView!
    var mtlDevice: MTLDevice!
    var normalPipelineState: MTLRenderPipelineState!
    var addPipelineState: MTLRenderPipelineState!
    var dimPipelineState: MTLRenderPipelineState!
    var rulePipelineState: MTLRenderPipelineState!
    var meltPipelineState: MTLRenderPipelineState!
    var commandQueue: MTLCommandQueue!
    var commandBuffer: MTLCommandBuffer!
    var blitEncoder: MTLBlitCommandEncoder!
    var renderEncoder: MTLRenderCommandEncoder!
    var textureArray = [MTLTexture]()
    var imageToTextureMap = Dictionary<UnsafeRawPointer, MTLTexture>()

    // For delayed blit.
    var initialUploadArray = [UnsafeRawPointer]()
    var purgeArray = [MTLTexture]()
    
    init(_ view: MTKView) {
        // Setup a Swift function callback table for C code.
        HalCSetup(&Renderer.callbacks)

        mtkView = view
        mtlDevice = view.device
        
        // Load shaders.
        let defaultLibrary = view.device?.makeDefaultLibrary()
        
        // Construct the normal shader pipeline.
        let normalPipelineStateDescriptor = MTLRenderPipelineDescriptor()
        normalPipelineStateDescriptor.label = "Normal Texturing Pipeline"
        normalPipelineStateDescriptor.vertexFunction = defaultLibrary?.makeFunction(name: "vertexShader")
        normalPipelineStateDescriptor.fragmentFunction = defaultLibrary?.makeFunction(name: "fragmentNormalShader");
        normalPipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat
        normalPipelineStateDescriptor.colorAttachments[0].isBlendingEnabled = true
        normalPipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperation.add
        normalPipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperation.add
        normalPipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactor.sourceAlpha
        normalPipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactor.one
        normalPipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactor.oneMinusSourceAlpha
        normalPipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor =  MTLBlendFactor.one
        normalPipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat
        do {
            normalPipelineState = try view.device?.makeRenderPipelineState(descriptor: normalPipelineStateDescriptor)
        } catch let error {
            print(error)
        }
        
        // Construct the add shader pipeline.
        let addPipelineStateDescriptor = MTLRenderPipelineDescriptor()
        addPipelineStateDescriptor.label = "Add Texturing Pipeline"
        addPipelineStateDescriptor.vertexFunction = defaultLibrary?.makeFunction(name: "vertexShader")
        addPipelineStateDescriptor.fragmentFunction = defaultLibrary?.makeFunction(name: "fragmentAddShader")
        addPipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat
        addPipelineStateDescriptor.colorAttachments[0].isBlendingEnabled = true
        addPipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperation.add
        addPipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperation.add
        addPipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactor.sourceAlpha
        addPipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactor.one
        addPipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactor.oneMinusSourceAlpha
        addPipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor =  MTLBlendFactor.one
        addPipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat
        do {
            addPipelineState = try view.device?.makeRenderPipelineState(descriptor: addPipelineStateDescriptor)
        } catch let error {
            print(error)
        }
        
        // Construct a dim shader pipeline.
        let dimPipelineStateDescriptor = MTLRenderPipelineDescriptor()
        dimPipelineStateDescriptor.label = "Dim Texturing Pipeline"
        dimPipelineStateDescriptor.vertexFunction = defaultLibrary?.makeFunction(name: "vertexShader")
        dimPipelineStateDescriptor.fragmentFunction = defaultLibrary?.makeFunction(name: "fragmentDimShader")
        dimPipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat
        dimPipelineStateDescriptor.colorAttachments[0].isBlendingEnabled = true
        dimPipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperation.add
        dimPipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperation.add
        dimPipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactor.sourceAlpha
        dimPipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactor.one
        dimPipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactor.oneMinusSourceAlpha
        dimPipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor =  MTLBlendFactor.one
        dimPipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat
        do {
            dimPipelineState = try view.device?.makeRenderPipelineState(descriptor: dimPipelineStateDescriptor)
        } catch let error {
            print(error)
        }
        
        // Construct a rule shader pipeline.
        let rulePipelineStateDescriptor = MTLRenderPipelineDescriptor()
        rulePipelineStateDescriptor.label = "Rule Texturing Pipeline"
        rulePipelineStateDescriptor.vertexFunction = defaultLibrary?.makeFunction(name: "vertexShader")
        rulePipelineStateDescriptor.fragmentFunction = defaultLibrary?.makeFunction(name:"fragmentRuleShader")
        rulePipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat
        rulePipelineStateDescriptor.colorAttachments[0].isBlendingEnabled = true
        rulePipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperation.add
        rulePipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperation.add
        rulePipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactor.sourceAlpha
        rulePipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactor.one
        rulePipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactor.oneMinusSourceAlpha
        rulePipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactor.one
        rulePipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat
        do {
            rulePipelineState = try view.device?.makeRenderPipelineState(descriptor:rulePipelineStateDescriptor)
        } catch let error {
            print(error)
        }
        
        // Construct a melt shader pipeline.
        let meltPipelineStateDescriptor = MTLRenderPipelineDescriptor()
        meltPipelineStateDescriptor.label = "Melt Texturing Pipeline"
        meltPipelineStateDescriptor.vertexFunction = defaultLibrary?.makeFunction(name:"vertexShader")
        meltPipelineStateDescriptor.fragmentFunction = defaultLibrary?.makeFunction(name: "fragmentMeltShader")
        meltPipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat
        meltPipelineStateDescriptor.colorAttachments[0].isBlendingEnabled = true
        meltPipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperation.add
        meltPipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperation.add
        meltPipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactor.sourceAlpha
        meltPipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactor.one
        meltPipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactor.oneMinusSourceAlpha
        meltPipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactor.one
        meltPipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat
        do {
            meltPipelineState = try view.device?.makeRenderPipelineState(descriptor: meltPipelineStateDescriptor)
        } catch let error {
            print(error)
        }
        
        // Create a command queue.
        commandQueue = view.device?.makeCommandQueue()
    }
    
    func draw(in view: MTKView) {
        // Guard if not drawable.
        if view.currentRenderPassDescriptor == nil {
            return
        }
        
        // Don't render if a video is playing back.
        if Controller.instance.isVideoPlaying() {
            runSuika2Frame()
            return;
        }
        
        // Create a command buffer.
        commandBuffer = commandQueue.makeCommandBuffer()
        commandBuffer.label = "MyCommand"
        
        // Nil-ify the encoders.
        blitEncoder = nil
        renderEncoder = nil
        
        // Create textures for images that are loaded before the first rendering.
        for image in initialUploadArray {
            notifyImageUpdate(image: image)
        }
        
        // Set a complete handler for semaphore.
        Renderer.in_flight_semaphore.wait()
        commandBuffer.addCompletedHandler { commandBuffer in
            Renderer.in_flight_semaphore.signal()
        }
        
        // Make sure to reset the array for textures to be destroyed.
        purgeArray.removeAll()
        
        // Run a Suika2 frame event and do rendering.
        runSuika2Frame()
        
        // End encodings.
        if blitEncoder != nil {
            blitEncoder.endEncoding()
        }
        if renderEncoder != nil {
            renderEncoder.endEncoding()
        }
        
        // Schedule a rendering to the current drawable.
        commandBuffer.present(view.currentDrawable!)
        
        // Push the command buffer to the GPU.
        commandBuffer.commit()
        
        // Synchronize.
        commandBuffer.waitUntilCompleted()
        
        // Set destroyed textures purgeable.
        for texture in purgeArray {
            texture.setPurgeableState(MTLPurgeableState.empty)
        }
        purgeArray.removeAll();
    }
    
    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
    }
    
    func runSuika2Frame() -> Void {
        on_event_frame()
    }
    
    func notifyImageUpdate(image: UnsafeRawPointer) -> Void {
        // Treat images that are created before the Metal initialization.
        if commandBuffer == nil {
            if !initialUploadArray.contains(image) {
                initialUploadArray.append(image)
            }
            return
        }
        
        let width = Int(get_image_width(image))
        let height = Int(get_image_height(image))
        
        if imageToTextureMap.index(forKey: image) == nil {
            // For the first time, create a texture.
            
            // Make a texture descriptor.
            let textureDescriptor = MTLTextureDescriptor()
            textureDescriptor.pixelFormat = MTLPixelFormat.bgra8Unorm
            textureDescriptor.width = width
            textureDescriptor.height = height
            
            // Make a texture.
            let texture = mtlDevice.makeTexture(descriptor: textureDescriptor)
            textureArray.append(texture!)
            imageToTextureMap[image] = texture
            
            // Upload the pixels.
            assert(renderEncoder == nil)
            if blitEncoder == nil {
                blitEncoder = commandBuffer.makeBlitCommandEncoder()
                blitEncoder.label = "Texture Encoder"
            }
            let pixels = get_image_pixels(image)
            texture?.replace(region: MTLRegionMake2D(0, 0, width, height), mipmapLevel: 0, withBytes: pixels, bytesPerRow: width * 4)
        } else {
            // Get an existing texture.
            let texture = imageToTextureMap[image]
            
            // Upload the pixels.
            assert(renderEncoder == nil)
            if blitEncoder == nil {
                blitEncoder = commandBuffer.makeBlitCommandEncoder()
                blitEncoder.label = "Texture Encoder"
            }
            let pixels = get_image_pixels(image)
            texture?.replace(region: MTLRegionMake2D(0, 0, width, height), mipmapLevel: 0, withBytes: pixels, bytesPerRow: width * 4)
        }
    }
    
    func notifyImageFree(image: UnsafeRawPointer) -> Void {
        // Treat images that are created and removed before the Metal initialization.
        initialUploadArray.removeAll(where: {$0 == image})
        
        // Remove a texture.
        let texture = imageToTextureMap[image]
        if texture != nil {
            purgeArray.append(texture!)
            imageToTextureMap.removeValue(forKey: image)
        }
    }
    
    func renderImageNormal(dstLeft: Int, dstTop: Int, dstWidth: Int, dstHeight: Int, image: UnsafeRawPointer, srcLeft: Int, srcTop: Int, srcWidth: Int, srcHeight: Int, alpha: Int) -> Void {
        let dw = (dstWidth == -1) ? Int(get_image_width(image)) : dstWidth
        let dh = (dstHeight == -1) ? Int(get_image_height(image)) : dstHeight
        let sw = (srcWidth == -1) ? Int(get_image_width(image)) : srcWidth
        let sh = (srcHeight == -1) ? Int(get_image_height(image)) : srcHeight
        drawPrimitives2D(dstLeft: dstLeft,
                         dstTop: dstTop,
                         dstWidth: dw,
                         dstHeight: dh,
                         image: image,
                         ruleImage: nil,
                         srcLeft: srcLeft,
                         srcTop: srcTop,
                         srcWidth: sw,
                         srcHeight: sh,
                         alpha: alpha,
                         pipeline: normalPipelineState)
    }
    
    func renderImageAdd(dstLeft: Int, dstTop: Int, dstWidth: Int, dstHeight: Int, image: UnsafeRawPointer, srcLeft: Int, srcTop: Int, srcWidth: Int, srcHeight: Int, alpha: Int) -> Void {
        let dw = (dstWidth == -1) ? Int(get_image_width(image)) : dstWidth
        let dh = (dstHeight == -1) ? Int(get_image_height(image)) : dstHeight
        let sw = (srcWidth == -1) ? Int(get_image_width(image)) : srcWidth
        let sh = (srcHeight == -1) ? Int(get_image_height(image)) : srcHeight
        drawPrimitives2D(dstLeft: dstLeft,
                         dstTop: dstTop,
                         dstWidth: dw,
                         dstHeight: dh,
                         image: image,
                         ruleImage: nil,
                         srcLeft: srcLeft,
                         srcTop: srcTop,
                         srcWidth: sw,
                         srcHeight: sh,
                         alpha: alpha,
                         pipeline: addPipelineState)
    }
    
    func renderImageDim(dstLeft: Int, dstTop: Int, dstWidth: Int, dstHeight: Int, image: UnsafeRawPointer, srcLeft: Int, srcTop: Int, srcWidth: Int, srcHeight: Int, alpha: Int) -> Void {
        let dw = (dstWidth == -1) ? Int(get_image_width(image)) : dstWidth
        let dh = (dstHeight == -1) ? Int(get_image_height(image)) : dstHeight
        let sw = (srcWidth == -1) ? Int(get_image_width(image)) : srcWidth
        let sh = (srcHeight == -1) ? Int(get_image_height(image)) : srcHeight
        drawPrimitives2D(dstLeft: 0,
                         dstTop: 0,
                         dstWidth: dw,
                         dstHeight: dh,
                         image: image,
                         ruleImage: nil,
                         srcLeft: 0,
                         srcTop: 0,
                         srcWidth: sw,
                         srcHeight: sh,
                         alpha: alpha,
                         pipeline: rulePipelineState)    }
    
    func renderImageRule(image: UnsafeRawPointer, ruleImage: UnsafeRawPointer, threshold: Int) -> Void {
        let width = Int(get_image_width(image))
        let height = Int(get_image_height(image))
        drawPrimitives2D(dstLeft: 0,
                         dstTop: 0,
                         dstWidth: width,
                         dstHeight: height,
                         image: image,
                         ruleImage: ruleImage,
                         srcLeft: 0,
                         srcTop: 0,
                         srcWidth: width,
                         srcHeight: height,
                         alpha: threshold,
                         pipeline: rulePipelineState)
    }
    
    func renderImageMelt(image: UnsafeRawPointer, ruleImage: UnsafeRawPointer, progress: Int) -> Void {
        let width = Int(get_image_width(image))
        let height = Int(get_image_height(image))
        drawPrimitives2D(dstLeft: 0,
                         dstTop: 0,
                         dstWidth: width,
                         dstHeight: height,
                         image: image,
                         ruleImage: ruleImage,
                         srcLeft: 0,
                         srcTop: 0,
                         srcWidth: width,
                         srcHeight: height,
                         alpha: progress,
                         pipeline: meltPipelineState)
    }

    func drawPrimitives2D(dstLeft: Int, dstTop: Int, dstWidth: Int, dstHeight: Int, image: UnsafeRawPointer, ruleImage: UnsafeRawPointer!, srcLeft: Int, srcTop: Int, srcWidth: Int, srcHeight: Int, alpha: Int, pipeline: MTLRenderPipelineState) {
        let x1 = Float(dstLeft)
        let y1 = Float(dstTop)
        let x2 = Float(dstLeft + dstWidth - 1)
        let y2 = Float(dstTop)
        let x3 = Float(dstLeft)
        let y3 = Float(dstTop + dstHeight - 1)
        let x4 = Float(dstLeft + dstWidth - 1)
        let y4 = Float(dstTop + dstHeight - 1)
        drawPrimitives3D(x1: x1,
                         y1: y1,
                         x2: x2,
                         y2: y2,
                         x3: x3,
                         y3: y3,
                         x4: x4,
                         y4: y4,
                         image: image,
                         ruleImage: ruleImage,
                         srcLeft: Float(srcLeft),
                         srcTop: Float(srcTop),
                         srcWidth: Float(srcWidth),
                         srcHeight: Float(srcHeight),
                         alpha: Float(alpha),
                         pipeline: pipeline)
    }

    func renderImage3DNormal(x1: Float, y1: Float, x2: Float, y2: Float, x3: Float, y3: Float, x4: Float, y4: Float, image: UnsafeRawPointer, srcLeft: Int, srcTop: Int, srcWidth: Int, srcHeight: Int, alpha: Int, pipeline: MTLRenderPipelineState) {
        drawPrimitives3D(x1: x1,
                         y1: y1,
                         x2: x2,
                         y2: y2,
                         x3: x3,
                         y3: y3,
                         x4: x4,
                         y4: y4,
                         image: image,
                         ruleImage: nil,
                         srcLeft: Float(srcLeft),
                         srcTop: Float(srcTop),
                         srcWidth: Float(srcWidth),
                         srcHeight: Float(srcHeight),
                         alpha: Float(alpha),
                         pipeline: normalPipelineState)
    }

    func renderImage3DAdd(x1: Float, y1: Float, x2: Float, y2: Float, x3: Float, y3: Float, x4: Float, y4: Float, image: UnsafeRawPointer, srcLeft: Int, srcTop: Int, srcWidth: Int, srcHeight: Int, alpha: Int, pipeline: MTLRenderPipelineState) {
        drawPrimitives3D(x1: x1,
                         y1: y1,
                         x2: x2,
                         y2: y2,
                         x3: x3,
                         y3: y3,
                         x4: x4,
                         y4: y4,
                         image: image,
                         ruleImage: nil,
                         srcLeft: Float(srcLeft),
                         srcTop: Float(srcTop),
                         srcWidth: Float(srcWidth),
                         srcHeight: Float(srcHeight),
                         alpha: Float(alpha),
                         pipeline: addPipelineState)
    }

    func drawPrimitives3D(x1: Float, y1: Float, x2: Float, y2: Float, x3: Float, y3: Float, x4: Float, y4: Float, image: UnsafeRawPointer, ruleImage: UnsafeRawPointer!, srcLeft: Float, srcTop: Float, srcWidth: Float, srcHeight: Float, alpha: Float, pipeline: MTLRenderPipelineState) {
        // Calc the half size of the window.
        let hw = Float(conf_window_width) / 2.0
        let hh = Float(conf_window_height) / 2.0
        
        // Get the texture size.
        let tw = Float(get_image_width(image))
        let th = Float(get_image_height(image))
        
        // The vertex shader input
        var vsIn = [Float]()
        
        // Set the left top vertex.
        vsIn[0] = (x1 - hw) / hw                 // X (-1.0 to 1.0, left to right)
        vsIn[1] = -(y1 - hh) / hh                // Y (-1.0 to 1.0, bottom to top)
        vsIn[2] = srcLeft / tw                   // U (0.0 to 1.0, left to right)
        vsIn[3] = srcTop / th                    // V (0.0 to 1.0, top to bottom)
        vsIn[4] = alpha / 255.0                  // Alpha (0.0 to 1.0)
        vsIn[5] = 0                              // Padding for a 64-bit boundary
        
        // Set the right top vertex.
        vsIn[6] = (x2 - hw) / hw                 // X (-1.0 to 1.0, left to right)
        vsIn[7] = -(y2 - hh) / hh                // Y (-1.0 to 1.0, bottom to top)
        vsIn[8] = (srcLeft + srcWidth) / tw      // U (0.0 to 1.0, left to right)
        vsIn[9] = srcTop / th                    // V (0.0 to 1.0, top to bottom)
        vsIn[10] = alpha / 255.0                 // Alpha (0.0 to 1.0)
        vsIn[11] = 0                             // Padding for a 64-bit boundary
        
        // Set the left bottom vertex.
        vsIn[12] = (x3 - hw) / hw                // X (-1.0 to 1.0, left to right)
        vsIn[13] = -(y3 - hh) / hh               // Y (-1.0 to 1.0, bottom to top)
        vsIn[14] = srcLeft / tw                  // U (0.0 to 1.0, left to right)
        vsIn[15] = srcTop + srcHeight / th       // V (0.0 to 1.0, top to bottom)
        vsIn[16] = alpha / 255.0                 // Alpha (0.0 to 1.0)
        vsIn[17] = 0                             // Padding for a 64-bit boundary
        
        // Set the right bottom vertex.
        vsIn[18] = (x4 - hw) / hw                // X (-1.0 to 1.0, left to right)
        vsIn[19] = -(y4 - hh) / hh               // Y (-1.0 to 1.0, bottom to top)
        vsIn[20] = (srcLeft + srcWidth) / tw     // U (0.0 to 1.0, left to right)
        vsIn[21] = (srcTop + srcHeight) / th     // V (0.0 to 1.0, top to bottom)
        vsIn[22] = alpha / 255.0                 // Alpha (0.0 to 1.0)
        vsIn[23] = 0                             // Padding for a 64-bit boundary
        
        // Upload textures if they are pending.
        if blitEncoder != nil {
            blitEncoder.endEncoding()
            blitEncoder = nil
        }
        
        // Draw two triangles.
        if renderEncoder == nil {
            mtkView.currentRenderPassDescriptor?.colorAttachments[0].loadAction = MTLLoadAction.clear
            mtkView.currentRenderPassDescriptor?.colorAttachments[0].storeAction = MTLStoreAction.store
            mtkView.currentRenderPassDescriptor?.colorAttachments[0].clearColor = (conf_window_white != 0) ?
            MTLClearColorMake(1.0, 1.0, 1.0, 1.0) :
            MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
            renderEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: mtkView.currentRenderPassDescriptor!)
            renderEncoder.label = "MyRenderEncoder"
            
            var viewport = MTLViewport()
            viewport.originX = Controller.instance.screenOffset().x
            viewport.originY = Controller.instance.screenOffset().y
            viewport.width = Controller.instance.screenSize().width
            viewport.height = Controller.instance.screenSize().height
            viewport.zfar = 0
            viewport.znear = 0
            renderEncoder.setViewport(viewport)
        }
        renderEncoder.setRenderPipelineState(pipeline)
        renderEncoder.setVertexBytes(vsIn, length: vsIn.count * MemoryLayout<Float>.stride, index: 0)
        if imageToTextureMap.index(forKey: image) != nil {
            renderEncoder.setFragmentTexture(imageToTextureMap[image], index: ShaderTextureIndexColor)
        }
        if imageToTextureMap.index(forKey: ruleImage) != nil {
            renderEncoder.setFragmentTexture(imageToTextureMap[ruleImage], index: ShaderTextureIndexRule)
        }
        renderEncoder.drawPrimitives(type: MTLPrimitiveType.triangleStrip, vertexStart: 0, vertexCount: 4)
    }

    //
    // Swift part of HAL implementation
    //

    private static func halPrint(msg: UnsafePointer<CChar>) -> Bool {
        print("\(String(cString: msg))")
        return true
    }

    private static func halMakeSavDir() -> Bool {
        var path = FileManager.default.currentDirectoryPath
        path.append("/")
        path.append(SAVE_DIR)
        
        do {
            try FileManager.default.createDirectory(atPath: path, withIntermediateDirectories: false)
        } catch let error {
            print(error)
        }
        return true
    }

    private static func halMakeValidPath(dir: UnsafePointer<CChar>?, file: UnsafePointer<CChar>?) -> String {
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
        return path
    }
    
    private static func halNotifyImageUpdate(image: UnsafeRawPointer) -> Void {
        Renderer.instance.notifyImageUpdate(image: image)
    }
    
    private static func halNotifyImageFree(image: UnsafeRawPointer) -> Void {
        Renderer.instance.notifyImageFree(image: image)
    }
    
    private static func halRenderImageNormal(dstLeft: Int, dstTop: Int, dstWidth: Int, dstHeight: Int, srcImage: UnsafeRawPointer, srcLeft: Int, srcTop: Int, srcWidth: Int, srcHeight: Int, alpha: Int) -> Void {
    }
    
    private static func halRenderImageAdd(dstLeft: Int, dstTop: Int, dstWidth: Int, dstHeight: Int, srcImage: UnsafeRawPointer, srcLeft: Int, srcTop: Int, srcWidth: Int, srcHeight: Int, alpha: Int) -> Void {
    }
    
    private static func halRenderImageDim(dstLeft: Int, dstTop: Int, dstWidth: Int, dstHeight: Int, srcImage: UnsafeRawPointer, srcLeft: Int, srcTop: Int, srcWidth: Int, srcHeight: Int, alpha: Int) -> Void {
    }
    
    private static func halRenderImageRule(srcImage: UnsafeRawPointer, ruleImage: UnsafeRawPointer, threshold: Int) -> Void {
    }
    
    private static func halRenderImageMelt(srcImage: UnsafeRawPointer, ruleImage: UnsafeRawPointer, progress: Int) -> Void {
    }
    
    private static func halPlayVideo(file: String, skippable: Bool) -> Bool {
        return false
    }
    
    private static func halStopVideo() -> Void {
    }
    
    private static func halIsVideoPlaying() -> Bool {
        return false
    }
    
    private static func halUpdateWindowTitle() -> Void {
    }
    
    private static func halIsFullScreenSupported() -> Bool {
        return false
    }
    
    private static func halIsFullScreenMode() -> Bool {
        return false
    }
    
    private static func halEnterFullScreenMode() -> Void {
    }
    
    private static func halLeaveFullScreenMode() -> Void {
    }
    
    private static func halGetSystemLocale() -> String {
        return "ja"
    }
    
    private static func halSpeakText(text: String) -> Void {
    }
    
    static var callbacks: HalSwiftExports = HalSwiftExports(
        logInfo: { (msg) in
            halPrint(msg: msg)
        },
        logWarn: { (msg) in
            halPrint(msg: msg)
        },
        logError: { (msg) in
            halPrint(msg: msg)
        },
        makeSavDir: { () in
            halMakeSavDir()
        },
        makeValidPath: { (dir, file, dst, len) in
            let s = halMakeValidPath(dir: dir, file: file).utf8CString
            let addr = s.withUnsafeBytes { $0.baseAddress! }
            dst.copyMemory(from: addr, byteCount: min(len, s.count))
        },
        notifyImageUpdate: { (img) in
            halNotifyImageUpdate(image: UnsafeRawPointer(img))
        },
        notifyImageFree: { (img) in
            halNotifyImageFree(image: UnsafeRawPointer(img))
        },
        renderImageNormal: { (dstLeft, dstTop, dstWidth, dstHeight, srcImage, srcLeft, srcTop, srcWidth, srcHeight, alpha) in
            halRenderImageNormal(dstLeft: Int(dstLeft), dstTop: Int(dstTop), dstWidth: Int(dstWidth), dstHeight: Int(dstHeight), srcImage: UnsafeRawPointer(srcImage), srcLeft: Int(srcLeft), srcTop: Int(srcTop), srcWidth: Int(srcWidth), srcHeight: Int(srcHeight), alpha: Int(alpha))
        },
        renderImageAdd: { (dstLeft, dstTop, dstWidth, dstHeight, srcImage, srcLeft, srcTop, srcWidth, srcHeight, alpha) in
            halRenderImageAdd(dstLeft: Int(dstLeft), dstTop: Int(dstTop), dstWidth: Int(dstWidth), dstHeight: Int(dstHeight), srcImage: UnsafeRawPointer(srcImage), srcLeft: Int(srcLeft), srcTop: Int(srcTop), srcWidth: Int(srcWidth), srcHeight: Int(srcHeight), alpha: Int(alpha))
        },
        renderImageDim: { (dstLeft, dstTop, dstWidth, dstHeight, srcImage, srcLeft, srcTop, srcWidth, srcHeight, alpha) in
            halRenderImageNormal(dstLeft: Int(dstLeft), dstTop: Int(dstTop), dstWidth: Int(dstWidth), dstHeight: Int(dstHeight), srcImage: UnsafeRawPointer(srcImage), srcLeft: Int(srcLeft), srcTop: Int(srcTop), srcWidth: Int(srcWidth), srcHeight: Int(srcHeight), alpha: Int(alpha))
        },
        renderImageRule: { (srcImage, ruleImage, threshold) in
            halRenderImageRule(srcImage: UnsafeRawPointer(srcImage), ruleImage: UnsafeRawPointer(ruleImage), threshold: Int(threshold))
        },
        renderImageMelt: { (srcImage, ruleImage, progress) in
            halRenderImageMelt(srcImage: UnsafeRawPointer(srcImage), ruleImage: UnsafeRawPointer(ruleImage), progress: Int(progress))
        },
        playVideo: { (file, skippable) in
            halPlayVideo(file: String(cString: file), skippable: skippable)
        },
        stopVideo: { () in
            halStopVideo()
        },
        isVideoPlaying: { () in
            halIsVideoPlaying()
        },
        updateWindowTitle: { () in
            halUpdateWindowTitle()
        },
        isFullScreenSupported: { () in
            halIsFullScreenSupported()
        },
        isFullScreenMode: { () in
            halIsFullScreenMode()
        },
        enterFullScreenMode: { () in
            halEnterFullScreenMode()
        },
        leaveFullScreenMode: { () in
            halLeaveFullScreenMode()
        },
        getSystemLocale: { () in
            return halGetSystemLocale().utf8CString.withUnsafeBytes { $0.baseAddress! }
        },
        speakText: { (text) in
            if let t = text {
                halSpeakText(text: String(cString: UnsafePointer<CChar>(t)))
            }
        }
    )
}
