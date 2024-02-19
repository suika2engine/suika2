//
//  ContentView.swift
//  pro-swiftui
//
//  Created by Keiichi Tabata on 2024/02/13.
//

import SwiftUI

struct ContentView: View {
    @State private var text = "script"

    var body: some View {
        HStack {
            MetalView()
            TextView()
        }
        .padding()
    }
}

#Preview {
    ContentView()
}
