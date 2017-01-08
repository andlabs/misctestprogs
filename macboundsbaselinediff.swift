// 8 january 2017
import Foundation
import CoreFoundation
import CoreGraphics
import CoreText

let our_CGFLOAT_MAX: CGFloat = CGFloat.greatestFiniteMagnitude

func iterative() {
	var s = "This is a test string."
	for _ in 0..<10 {
		s += " This is a test string."
	}
	let cs = s as! NSString as! CFString
	func runForFont(_ d: CTFontDescriptor, _ s: CGFloat) {
		let f = CTFontCreateWithFontDescriptor(d, s, nil)
		let attrs = CFDictionaryCreateMutable(nil, 0,
			[kCFCopyStringDictionaryKeyCallBacks],
			[kCFTypeDictionaryValueCallBacks])
		CFDictionaryAddValue(attrs,
			Unmanaged.passUnretained(kCTFontAttributeName).toOpaque(),
			Unmanaged.passUnretained(f).toOpaque())
		let cfas = CFAttributedStringCreate(nil, cs, attrs)
		let fs = CTFramesetterCreateWithAttributedString(cfas!)
		var range = CFRangeMake(0, CFAttributedStringGetLength(cfas))
		var fitRange = CFRange()
		let size = CTFramesetterSuggestFrameSizeWithConstraints(fs, range, nil,
			CGSize.init(width: 200, height: our_CGFLOAT_MAX),
			&fitRange)
		let rect = CGRect(origin: CGPoint.init(x: 0 /*5*/, y: 0 /*2.5*/), size: size)
		let path = CGPath(rect: rect, transform: nil)
		let frame = CTFramesetterCreateFrame(fs, range, path, nil)
		let lines = CTFrameGetLines(frame)
		let n = CFArrayGetCount(lines)
		if n < 2 {
			return
		}
		range.length = 2
		let pts = UnsafeMutablePointer<CGPoint>.allocate(capacity: 2)
		CTFrameGetLineOrigins(frame, range, pts)
		let ptheight = pts[0].y - pts[1].y		// unflipped
		let pl = CFArrayGetValueAtIndex(lines, 0)
		let line = Unmanaged<CTLine>.fromOpaque(pl!).takeUnretainedValue()
		let bounds = CTLineGetBoundsWithOptions(line, [])
		let nm = CTFontDescriptorCopyAttribute(d, kCTFontDisplayNameAttribute) as! CFString
		print("baseline diff \(ptheight) bounds \(bounds.size.height) difference \(ptheight - bounds.size.height) - \(s) \(nm)")
	}
	func runForDesc(_ d: CTFontDescriptor) {
		for ps in [CGFloat](arrayLiteral: 0, 8, 10, 12, 14, 16, 18, 20, 22, 24, 36, 48, 72) {
			runForFont(d, ps)
		}
	}
	let fc = CTFontCollectionCreateFromAvailableFonts(nil)
	let descs = CTFontCollectionCreateMatchingFontDescriptors(fc)
	let n = CFArrayGetCount(descs)
	for i in 0..<n {
		let pd = CFArrayGetValueAtIndex(descs, i)
		let d = Unmanaged<CTFontDescriptor>.fromOpaque(pd!).takeUnretainedValue()
		runForDesc(d)
	}
}
iterative()
