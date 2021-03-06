// 3 january 2016
// scratch program 17 august 2015
import Cocoa
import WebKit

var keepAliveMainwin: NSWindow? = nil

func appLaunched() {
	let mainwin = NSWindow(
		contentRect: NSMakeRect(0, 0, 320, 240),
		styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask),
		backing: NSBackingStoreType.Buffered,
		`defer`: true)
	let contentView = mainwin.contentView!

	let splitView = NSSplitView(frame: NSZeroRect)
	splitView.dividerStyle = NSSplitViewDividerStyle.Thin
	splitView.vertical = true
	splitView.translatesAutoresizingMaskIntoConstraints = false
	contentView.addSubview(splitView)

	let box1 = NSScrollView(frame: NSZeroRect)
	let ov = NSOutlineView(frame: NSZeroRect)
	ov.headerView = nil
	ov.selectionHighlightStyle = NSTableViewSelectionHighlightStyle.SourceList
	box1.documentView = ov
	box1.translatesAutoresizingMaskIntoConstraints = false
	splitView.addSubview(box1)

	let box2 = WebView(frame: NSZeroRect)
	box2.translatesAutoresizingMaskIntoConstraints = false
	splitView.addSubview(box2)

	let views: [String: NSView] = [
		"splitView":	splitView,
	]
	addConstraint(contentView, "H:|-[splitView]-|", views)
	addConstraint(contentView, "V:|-[splitView]-|", views)

	splitView.setPosition(50, ofDividerAtIndex: 0)

	mainwin.cascadeTopLeftFromPoint(NSMakePoint(20, 20))
	mainwin.makeKeyAndOrderFront(mainwin)
	keepAliveMainwin = mainwin
}

func addConstraint(view: NSView, _ constraint: String, _ views: [String: NSView]) {
	let constraints = NSLayoutConstraint.constraintsWithVisualFormat(
		constraint,
		options: [],
		metrics: nil,
		views: views)
	view.addConstraints(constraints)
}

class appDelegate : NSObject, NSApplicationDelegate {
	func applicationDidFinishLaunching(note: NSNotification) {
		appLaunched()
	}

	func applicationShouldTerminateAfterLastWindowClosed(app: NSApplication) -> Bool {
		return true
	}
}

func main() {
	let app = NSApplication.sharedApplication()
	app.setActivationPolicy(NSApplicationActivationPolicy.Regular)
	// NSApplication.delegate is weak; if we don't use the temporary variable, the delegate will die before it's used
	let delegate = appDelegate()
	app.delegate = delegate
	app.run()
}

main()
