// 17 august 2015
import Cocoa

var keepAliveMainwin: NSWindow? = nil

func appLaunched() {
	var mainwin = NSWindow(
		contentRect: NSMakeRect(0, 0, 320, 240),
		styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask),
		backing: NSBackingStoreType.Buffered,
		defer: true)
	var contentView = mainwin.contentView as! NSView

	mainwin.cascadeTopLeftFromPoint(NSMakePoint(20, 20))
	mainwin.makeKeyAndOrderFront(mainwin)
	keepAliveMainwin = mainwin
}

func addConstraints(view: NSView, constraint: String, views: [String: NSView]) {
	var constraints = NSLayoutConstraint.constraintsWithVisualFormat(
		constraint,
		options: NSLayoutFormatOptions(0),
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
	var app = NSApplication.sharedApplication()
	app.setActivationPolicy(NSApplicationActivationPolicy.Regular)
	// NSApplication.delegate is weak; if we don't use the temporary variable, the delegate will die before it's used
	var delegate = appDelegate()
	app.delegate = delegate
	app.run()
}

main()
