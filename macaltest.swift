// 9 august 2015
import Cocoa

var makeUnambiguous: Bool = false

var keepAliveMainwin: NSWindow? = nil

// the swift bridge isn't perfect; it won't recognize these properly
// thanks to Eridius in freenode/#swift-lang
let myNSLayoutPriorityRequired: NSLayoutPriority = 1000
let myNSLayoutPriorityDefaultHigh: NSLayoutPriority = 750
let myNSLayoutPriorityDragThatCanResizeWindow: NSLayoutPriority = 510
let myNSLayoutPriorityWindowSizeStayPut: NSLayoutPriority = 500
let myNSLayoutPriorityDragThatCannotResizeWindow: NSLayoutPriority = 490
let myNSLayoutPriorityDefaultLow: NSLayoutPriority = 250
let myNSLayoutPriorityFittingSizeCompression: NSLayoutPriority = 50

var nButton = 1

func appLaunched() {
	var mainwin = NSWindow(
		contentRect: NSMakeRect(0, 0, 320, 240),
		styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask),
		backing: NSBackingStoreType.Buffered,
		defer: true)
	var contentView = mainwin.contentView as! NSView

	var spinbox1 = mkSpinbox()
	makeVerticallyNotStretchy(spinbox1)
	contentView.addSubview(spinbox1)
	var spinbox2 = mkSpinbox()
	makeVerticallyStretchy(spinbox2)
	contentView.addSubview(spinbox2)

	var views = [
		"spinbox1":	spinbox1,
		"spinbox2":	spinbox2,
	]
	addConstraints(contentView, "V:|-[spinbox1]-[spinbox2]-|", views)
	addConstraints(contentView, "H:|-[spinbox1]-|", views)
	addConstraints(contentView, "H:|-[spinbox2]-|", views)

	mainwin.cascadeTopLeftFromPoint(NSMakePoint(20, 20))
	mainwin.makeKeyAndOrderFront(mainwin)
	isAmbiguous(contentView, 0)
	keepAliveMainwin = mainwin
}

class IntrinsicSizeView : NSView {
	override var intrinsicContentSize: NSSize {
		get {
			return NSMakeSize(96, 30)
		}
	}
}

func mkSpinbox() -> NSView {
	var view = NSView(frame: NSZeroRect)
	if makeUnambiguous {
		view = IntrinsicSizeView(frame: NSZeroRect)
	}
	view.translatesAutoresizingMaskIntoConstraints = false
	var t = mkButton()//mkTextField()
	var s = mkStepper()
	view.addSubview(t)
	view.addSubview(s)
	makeVerticallyStretchy(t)
	makeVerticallyStretchy(s)
	var views = [
		"t":	t,
		"s":	s,
	]
	addConstraints(view, "H:|[t]-[s]|", views)
	addConstraints(view, "V:|[t]|", views)
	addConstraints(view, "V:|[s]|", views)
	return view
}

func isAmbiguous(view: NSView, indent: Int) {
	var s = String(count: indent, repeatedValue: " " as Character)
	if view.hasAmbiguousLayout {
		view.window?.visualizeConstraints(view.superview!.constraints)
	}
	for subview in view.subviews {
		isAmbiguous(subview as! NSView, indent + 1)
	}
}

// TODO why can't the lets above be here?

func makeVerticallyNotStretchy(view: NSView) {
	view.setContentHuggingPriority(myNSLayoutPriorityDefaultHigh, forOrientation: NSLayoutConstraintOrientation.Vertical)
}

func makeVerticallyStretchy(view: NSView) {
	view.setContentHuggingPriority(myNSLayoutPriorityDefaultLow, forOrientation: NSLayoutConstraintOrientation.Vertical)
}

func addConstraints(view: NSView, constraint: String, views: [String: NSView]) {
	var constraints = NSLayoutConstraint.constraintsWithVisualFormat(
		constraint,
		options: NSLayoutFormatOptions(0),
		metrics: nil,
		views: views)
	view.addConstraints(constraints)
}

func mkButton() -> NSButton {
	var b = NSButton(frame: NSZeroRect)
	b.title = "Button \(nButton)"
	nButton++
	b.setButtonType(NSButtonType.MomentaryPushInButton)
	b.bordered = true
	b.bezelStyle = NSBezelStyle.RoundedBezelStyle
	b.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.RegularControlSize))
	b.translatesAutoresizingMaskIntoConstraints = false
	return b
}

func mkTextField() -> NSTextField {
	var cell: NSTextFieldCell

	var t = NSTextField(frame: NSZeroRect)
	t.selectable = true
	t.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.RegularControlSize))
	t.bordered = false
	t.bezelStyle = NSTextFieldBezelStyle.SquareBezel
	t.bezeled = true
	cell = t.cell() as! NSTextFieldCell
	cell.lineBreakMode = NSLineBreakMode.ByClipping
	cell.scrollable = true
	t.translatesAutoresizingMaskIntoConstraints = false
	return t
}

func mkStepper() -> NSStepper {
	var s = NSStepper(frame: NSZeroRect)
	s.increment = 1
	s.valueWraps = false
	s.autorepeat = true
	s.translatesAutoresizingMaskIntoConstraints = false
	return s
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
	makeUnambiguous = Process.arguments.count > 1 && Process.arguments[1] == "unambiguous"

	var app = NSApplication.sharedApplication()
	app.setActivationPolicy(NSApplicationActivationPolicy.Regular)
	// NSApplication.delegate is weak; if we don't use the temporary variable, the delegate will die before it's used
	var delegate = appDelegate()
	app.delegate = delegate
	app.run()
}

main()
