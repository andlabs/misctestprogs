// 13 april 2016
// scratch program 17 august 2015
import Cocoa

var keepAliveMainwin: NSWindow? = nil

class well : NSColorWell {
	var which: Int = 0
	override func activate(_ exclusive: Bool) {
		debugPrint("activating \(which) \(exclusive)")
		if !exclusive {
			debugPrint("== overriding multi-activation")
		}
		super.activate(true)
	}
	override func deactivate() {
		debugPrint("deactivating \(which)")
		super.deactivate()
	}
}

func appLaunched() {
	let mainwin = NSWindow(
		contentRect: NSMakeRect(0, 0, 320, 240),
		styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask),
		backing: NSBackingStoreType.Buffered,
		defer: true)
	let contentView = mainwin.contentView!

	let well1 = well(frame: NSZeroRect)
	well1.which = 1
	well1.translatesAutoresizingMaskIntoConstraints = false
	contentView.addSubview(well1)
	let well2 = well(frame: NSZeroRect)
	well2.color = NSColor.blackColor()
	well2.which = 2
	well2.translatesAutoresizingMaskIntoConstraints = false
	contentView.addSubview(well2)

	let views = [
		"well1":	well1,
		"well2":	well2,
	]
	addConstraint(contentView, "H:|-[well1]-|", views)
	addConstraint(contentView, "H:|-[well2]-|", views)
	addConstraint(contentView, "V:|-[well1]-[well2(==well1)]-|", views)

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

func noOpt(_ opt: AnyObject?) -> String {
	if opt == nil {
		return "nil"
	}
	return "\(opt!)"
}

class ourApp : NSApplication {
	override func sendAction(_ theAction: Selector, to theTarget: AnyObject?, from sender: AnyObject?) -> Bool {
		if theAction == "changeColor:" {
			debugPrint("overriding changeColor: (\(noOpt(sender)) -> \(noOpt(theTarget)))")
			return false
		}
		return super.sendAction(theAction, to: theTarget, from: sender)
	}
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
	let app = ourApp.sharedApplication()
	app.setActivationPolicy(NSApplicationActivationPolicy.Regular)
	// NSApplication.delegate is weak; if we don't use the temporary variable, the delegate will die before it's used
	let delegate = appDelegate()
	app.delegate = delegate
	app.run()
}

main()
