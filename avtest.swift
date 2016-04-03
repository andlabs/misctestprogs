// 2 april 2016
import Cocoa
import AVFoundation
import AVKit

class appDelegate : NSObject, NSApplicationDelegate {
	func applicationShouldTerminateAfterLastWindowClosed(app: NSApplication) -> Bool {
		return true
	}
}

var app = NSApplication.sharedApplication()
app.setActivationPolicy(.Regular)
var ad = appDelegate()
app.delegate = ad

var mainwin = NSWindow(contentRect: NSMakeRect(0, 0, 800, 600),
	styleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask,
	backing: .Buffered,
	defer: true)

var session = AVCaptureSession()
session.sessionPreset = AVCaptureSessionPresetPhoto
var device = AVCaptureDevice.defaultDeviceWithMediaType(AVMediaTypeVideo)
var input = try! AVCaptureDeviceInput(device: device)
session.addInput(input)
session.startRunning()

var avv = NSView(frame: NSMakeRect(20, 20, 760, 560))
mainwin.contentView?.addSubview(avv)
var avl = AVCaptureVideoPreviewLayer(session: session)
avv.layer = avl
avv.wantsLayer = true

mainwin.center()
mainwin.makeKeyAndOrderFront(nil)

app.run()
