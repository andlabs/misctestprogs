// 17 august 2015
import Cocoa

var keepAliveMainwin: NSWindow? = nil
var matrix: NSMatrix? = nil

class ButtonHandler : NSObject {
	@IBAction func onClicked(sender: AnyObject) {
		var lastRow = matrix!.numberOfRows
		matrix!.renewRows(lastRow + 1, columns: 1)
		var cell = matrix!.cellAtRow(lastRow, column: 0) as! NSButtonCell
		cell.title = "New Item"
		matrix!.sizeToCells()
	}
}

var buttonHandler: ButtonHandler = ButtonHandler()

func appLaunched() {
	var mainwin = NSWindow(
		contentRect: NSMakeRect(0, 0, 320, 240),
		styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask),
		backing: NSBackingStoreType.Buffered,
		defer: true)
	var contentView = mainwin.contentView as! NSView

	var prototype = NSButtonCell()
	prototype.setButtonType(NSButtonType.RadioButton)
	prototype.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.RegularControlSize))

	matrix = NSMatrix(frame: NSZeroRect,
		mode: NSMatrixMode.RadioModeMatrix,
		prototype: prototype,
		numberOfRows: 0,
		numberOfColumns: 0)
	matrix!.allowsEmptySelection = false
	matrix!.selectionByRect = true
	matrix!.intercellSpacing = NSMakeSize(4, 2)
	matrix!.autorecalculatesCellSize = true
	matrix!.drawsBackground = false
	matrix!.drawsCellBackground = false
	matrix!.autosizesCells = true
	matrix!.translatesAutoresizingMaskIntoConstraints = false
	contentView.addSubview(matrix!)

	var button = NSButton(frame: NSZeroRect)
	button.title = "Append Item"
	button.setButtonType(NSButtonType.MomentaryPushInButton)
	button.bordered = true
	button.bezelStyle = NSBezelStyle.RoundedBezelStyle
	button.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.RegularControlSize))
	button.translatesAutoresizingMaskIntoConstraints = false
	contentView.addSubview(button)

	button.target = buttonHandler
	button.action = "onClicked:"

	var views: [String: NSView]
	views = [
		"button":	button,
		"matrix":	matrix!,
	]
	addConstraints(contentView, "V:|-[matrix]-[button]-|", views)
	addConstraints(contentView, "H:|-[matrix]-|", views)
	addConstraints(contentView, "H:|-[button]-|", views)

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
