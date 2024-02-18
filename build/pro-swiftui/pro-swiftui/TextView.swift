import SwiftUI

struct TextView: NSViewRepresentable {
    typealias NSViewType = NSTextView
    typealias Coordinator = TextViewCoordinator

    func makeCoordinator() -> Coordinator {
        Coordinator(self)
    }

    func makeNSView(context: Context) -> NSTextView {
        let textView = NSTextView()
        textView.delegate = context.coordinator
        textView.isEditable = true
        return textView
    }

    func updateNSView(_ textView: NSTextView, context: Context) {
    }
}

class TextViewCoordinator: NSObject, NSTextViewDelegate {
    private var textView: TextView

    init(_ textView: TextView) {
        self.textView = textView
    }

    private func textView(_ textView: NSTextView, shouldChangeTextIn range: NSRange, replacementText text: String) -> Bool {
        return true
    }

    func textViewDidChange(_ textView: NSTextView) {
        //get textView.string
    }
}
