// 16 december 2015
// scratch program 17 august 2015
import Cocoa

var keepAliveMainwin: NSWindow? = nil

class areaView : NSView {
	private var label: NSButton
	private var trackingArea: NSTrackingArea?

	init(_ label: NSButton) {
		self.label = label
		// another catch-22 because of swift's optionals misfeature, hooray
		// the tracking area needs to know that this is the view
		// I can't pass self because I didn't call super.init() yet
		// but I can't move the initialization until after super.init() because all ivars must be initiailized before super.init()
		self.trackingArea = nil
		super.init(frame: NSZeroRect)
		self.updateTrackingAreas()
	}

	required init?(coder: NSCoder) {
		fatalError("can't use this constructor, sorry")
	}

	override func updateTrackingAreas() {
		if self.trackingArea != nil {
			self.removeTrackingArea(self.trackingArea!)
		}
		// TODO NSTrackingAreaOptions.AssumeInside?
		self.trackingArea = NSTrackingArea(rect: self.bounds,
			options: NSTrackingAreaOptions.MouseEnteredAndExited | NSTrackingAreaOptions.MouseMoved | NSTrackingAreaOptions.ActiveAlways | NSTrackingAreaOptions.InVisibleRect | NSTrackingAreaOptions.EnabledDuringMouseDrag,
			owner: self,
			userInfo: nil)
		self.addTrackingArea(self.trackingArea!)
	}

	override var flipped: Bool {
		get {
			return true
		}
	}

	private func updateLabel(e: NSEvent) {
		var pt = self.convertPoint(e.locationInWindow, fromView: nil)

		self.label.title = "X \(pt.x) Y \(pt.y)"
	}

	override func mouseMoved(e: NSEvent) {
		self.updateLabel(e)
	}

	override func mouseDragged(e: NSEvent) {
		self.updateLabel(e)
	}

	override func rightMouseDragged(e: NSEvent) {
		self.updateLabel(e)
	}

	override func otherMouseDragged(e: NSEvent) {
		self.updateLabel(e)
	}

	override func mouseEntered(e: NSEvent) {
		self.label.state = NSOnState
	}

	override func mouseExited(e: NSEvent) {
		self.label.state = NSOffState
	}
}

func appLaunched() {
	var mainwin = NSWindow(
		contentRect: NSMakeRect(0, 0, 320, 240),
		styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask),
		backing: NSBackingStoreType.Buffered,
		defer: true)
	var contentView = mainwin.contentView as! NSView

	var sv = NSScrollView(frame: NSZeroRect)
	var label = NSButton(frame: NSZeroRect)
	var area = areaView(label)

	sv.hasHorizontalScroller = true
	sv.hasVerticalScroller = true

	label.setButtonType(NSButtonType.SwitchButton)
	label.bordered = false

	contentView.addSubview(sv)
	contentView.addSubview(label)
	sv.documentView = area
	area.frame = NSMakeRect(area.frame.origin.x, area.frame.origin.y,
		5000, 5000)

	autolayoutOnly(sv)
	autolayoutOnly(label)
	var views = [
		"sv":		sv,
		"label":	label
	]
	addConstraints(contentView, "H:|[sv]|", views)
	addConstraints(contentView, "H:|[label]|", views)
	addConstraints(contentView, "V:|[sv][label]|", views)

	mainwin.cascadeTopLeftFromPoint(NSMakePoint(20, 20))
	mainwin.makeKeyAndOrderFront(mainwin)
	keepAliveMainwin = mainwin
}

func autolayoutOnly(view: NSView) {
	view.translatesAutoresizingMaskIntoConstraints = false
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
