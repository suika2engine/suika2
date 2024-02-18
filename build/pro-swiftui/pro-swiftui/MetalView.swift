//
//  MetalView.swift
//  pro-swiftui
//
//  Created by Keiichi Tabata on 2024/02/13.
//

import SwiftUI
import MetalKit

#if os(macOS)
struct MetalView: NSViewRepresentable {
    typealias NSViewType = MTKView
    typealias Coordinator = Renderer

    func makeCoordinator() -> Renderer {
        let view = MTKView()
        view.device = view.preferredDevice
        return Renderer(view)
    }

    func makeNSView(context: Context) -> MTKView {
        let view = context.coordinator.mtkView!
        view.delegate = context.coordinator
        view.enableSetNeedsDisplay = true
        return view
    }

    func updateNSView(_ uiView: MTKView, context: Context) {
        // FIXME
    }
}
#else
struct MetalView: UIViewRepresentable {
    typealias UIViewType = MTKView
    typealias Coordinator = MetalViewCoordinator
    
    func makeCoordinator() -> MetalViewCoordinator {
        let view = MTKView()
        view.device = view.preferredDevice
        return MetalViewCoordinator(view)
    }
    
    func makeUIView(context: Context) -> MTKView {
        let view = context.coordinator.view
        view.delegate = context.coordinator
        view.enableSetNeedsDisplay = true
        return view
    }
    
    func updateUIView(_ uiView: MTKView, context: Context) {
        // FIXME
    }
}
#endif
