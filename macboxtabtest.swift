// 19 august 2015
import Cocoa

var keepAliveMainwin: NSWindow? = nil

func appLaunched() {
	var mainwin = NSWindow(
		contentRect: NSMakeRect(0, 0, 320, 240),
		styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask),
		backing: NSBackingStoreType.Buffered,
		defer: true)
	var contentView = mainwin.contentView as! NSView

	var tabview = NSTabView(frame: NSZeroRect)
	tabview.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.RegularControlSize))
	tabview.translatesAutoresizingMaskIntoConstraints = false
	contentView.addSubview(tabview)
	addConstraints(contentView, "H:|-[tabview]-|", [
		"tabview": tabview,
	])
	addConstraints(contentView, "V:|-[tabview]-|", [
		"tabview": tabview,
	])
	
	var tabpage1 = NSTabViewItem()
	tabpage1.label = "Page 1"
	tabpage1.view = NSView(frame: NSZeroRect)
	tabview.addTabViewItem(tabpage1)

	var tabpage2view = NSView(frame: NSZeroRect)
	var tabpage2 = NSTabViewItem()
	tabpage2.label = "Page 2"
	tabpage2.view = tabpage2view
	tabview.addTabViewItem(tabpage2)

	var box = NSBox(frame: NSZeroRect)
	box.title = "Box"
	box.boxType = NSBoxType.Primary
	box.borderType = NSBorderType.LineBorder
	box.transparent = false
	box.titlePosition = NSTitlePosition.AtTop
	box.titleFont = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.SmallControlSize))
	box.translatesAutoresizingMaskIntoConstraints = false
	tabpage2view.addSubview(box)

	var stretchyView = NSView(frame: NSZeroRect)
	stretchyView.translatesAutoresizingMaskIntoConstraints = false
	tabpage2view.addSubview(stretchyView)

	addConstraints(tabpage2view, "H:|-[box]-|", [
		"box":			box,
	])
	addConstraints(tabpage2view, "H:|-[stretchyView]-|", [
		"stretchyView":		stretchyView,
	])
	addConstraints(tabpage2view, "V:|-[box]-[stretchyView]-|", [
		"box":			box,
		"stretchyView":		stretchyView,
	])

	var button = NSButton(frame: NSZeroRect)
	button.title = "Button"
	button.setButtonType(NSButtonType.MomentaryPushInButton)
	button.bordered = true
	button.bezelStyle = NSBezelStyle.RoundedBezelStyle
	button.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.RegularControlSize))
	button.translatesAutoresizingMaskIntoConstraints = false
	var bcv = box.contentView as! NSView
	bcv.addSubview(button)
	addConstraints(bcv, "H:|-[button]-|", [
		"button": button,
	])
	addConstraints(bcv, "V:|-[button]-|", [
		"button": button,
	])

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
