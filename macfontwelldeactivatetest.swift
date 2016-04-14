// 13 april 2016
// maccolorwelldeactivatetest.swift 13 april 2016
// scratch program 17 august 2015
import Cocoa

var keepAliveMainwin: NSWindow? = nil

class fontButton : NSButton {
	static var activatedOne: fontButton? = nil

	var which: Int = 0

	var chosenFont: NSFont {
		didSet {
			self.fontWasSet()
		}
	}

	override init(frame: NSRect) {
		// default font values according to the CTFontDescriptor reference
		self.chosenFont = NSFont(name: "Helvetica", size: 12.0)!

		super.init(frame: frame)

		self.fontWasSet()

		self.setButtonType(NSButtonType.PushOnPushOffButton)
		// imitate a well
		self.bezelStyle = NSBezelStyle.ShadowlessSquareBezelStyle
		self.bordered = true

		self.target = self
		self.action = "toggle:"
	}

	required init(coder: NSCoder) {
		fatalError("not implemented")
	}

	private func fontWasSet() {
		self.title = "\(self.chosenFont.displayName!) \(self.chosenFont.pointSize)"
	}

	func activate() {
		if fontButton.activatedOne != nil {
			fontButton.activatedOne!.deactivate()
		}
		debugPrint("activating \(which)")
		let fm = NSFontManager.sharedFontManager()
		fm.target = self
		fm.setSelectedFont(self.chosenFont, isMultiple: false)
		fm.orderFrontFontPanel(self)
		fontButton.activatedOne = self
		self.state = NSOnState
	}

	func deactivate() {
		debugPrint("deactivating \(which)")
		NSFontManager.sharedFontManager().target = nil
		fontButton.activatedOne = nil
		self.state = NSOffState
	}

	func toggle(sender: AnyObject?) {
		if self.state == NSOnState {
			self.activate()
		} else {
			self.deactivate()
		}
	}

	override func changeFont(sender: AnyObject?) {
		debugPrint("changing font")
		self.chosenFont = (sender as! NSFontManager).convertFont(self.chosenFont)
	}

	override func validModesForFontPanel(_ fontPanel: NSFontPanel) -> Int {
		return Int(NSFontPanelFaceModeMask | NSFontPanelSizeModeMask | NSFontPanelCollectionModeMask)
	}
}

func appLaunched() {
	let mainwin = NSWindow(
		contentRect: NSMakeRect(0, 0, 320, 240),
		styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask),
		backing: NSBackingStoreType.Buffered,
		defer: true)
	let contentView = mainwin.contentView!

	let well1 = fontButton(frame: NSZeroRect)
	well1.which = 1
	well1.translatesAutoresizingMaskIntoConstraints = false
	contentView.addSubview(well1)
	let well2 = fontButton(frame: NSZeroRect)
//	well2.color = NSColor.blackColor()
	well2.which = 2
	well2.translatesAutoresizingMaskIntoConstraints = false
	contentView.addSubview(well2)

	let tv = NSTextView(frame: NSZeroRect)
	tv.translatesAutoresizingMaskIntoConstraints = false
	contentView.addSubview(tv)

	let views = [
		"well1":	well1,
		"well2":	well2,
		"tv":		tv,
	]
	addConstraint(contentView, "H:|-[well1]-|", views)
	addConstraint(contentView, "H:|-[well2]-|", views)
	addConstraint(contentView, "H:|-[tv]-|", views)
	addConstraint(contentView, "V:|-[well1]-[tv(==well1)]-[well2(==well1)]-|", views)

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

var inhibit: Bool = false

class ourApp : NSApplication {
	override func sendAction(_ theAction: Selector, to theTarget: AnyObject?, from sender: AnyObject?) -> Bool {
		debugPrint("sendAction \(theAction) (\(noOpt(sender)) -> \(noOpt(theTarget)))")
		if inhibit && theAction == "changeFont:" {
			if !(theTarget is fontButton) {
				debugPrint("overriding changeFont: (\(noOpt(sender)) -> \(noOpt(theTarget)))")
				return false
			}
		}
		return super.sendAction(theAction, to: theTarget, from: sender)
	}

	override func targetForAction(_ theAction: Selector, to theTarget: AnyObject?, from sender: AnyObject?) -> AnyObject? {
		if fontButton.activatedOne != nil && theAction == "validModesForFontPanel:" {
			return fontButton.activatedOne
		}
		// defer to super even if there is no active button and the selector matches, just to be safe
		if theAction != "validModesForFontPanel:" {		// reduce spew
			debugPrint("targetForAction \(theAction) (\(noOpt(sender)) -> \(noOpt(theTarget)))")
		}
		return super.targetForAction(theAction, to: theTarget, from: sender)
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

inhibit = (Process.arguments.count > 1 && Process.arguments[1] == "inhibit")
main()
