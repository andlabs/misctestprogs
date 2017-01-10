// 10 january 2017
import CoreFoundation
import CoreGraphics
import CoreText

import Foundation

let our_CGFLOAT_MAX: CGFloat = CGFloat.greatestFiniteMagnitude

func iterative3() {
	var s = "This is a test string."
	for _ in 0..<10 {
		s += " This is a test string."
	}
	let cs = s as! NSString as! CFString
	var pass: CFIndex = 0
	var total: CFIndex = 0
	var tbinfo = mach_timebase_info()
	mach_timebase_info(&tbinfo)
	var now = mach_absolute_time()
	func bumpTotal() {
		total += 1
/*
		if total % 1000 == 0 {
			var elapsed = mach_absolute_time()
			elapsed -= now
			elapsed = elapsed * UInt64(tbinfo.numer) / UInt64(tbinfo.denom)
			let sec = Double(elapsed) / Double(NSEC_PER_SEC)
			print("#progress \(total) - \(sec) s")
			now = mach_absolute_time()
		}
*/
	}
	func runForFontWidth(_ d: CTFontDescriptor, _ s: CGFloat, _ wid: CGFloat, _ nm: CFString) {
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
		                                                        CGSize.init(width: wid, height: our_CGFLOAT_MAX),
		                                                        &fitRange)
		let rect = CGRect(origin: CGPoint.init(x: 0 /*5*/, y: 0 /*2.5*/), size: size)
		let path = CGPath(rect: rect, transform: nil)
		let frame = CTFramesetterCreateFrame(fs, range, path, nil)
		let lines = CTFrameGetLines(frame)
		let n = CFArrayGetCount(lines)
		if n < 2 {
			return
		}
		bumpTotal()
		range.length = n
		let pts = UnsafeMutablePointer<CGPoint>.allocate(capacity: n)
		CTFrameGetLineOrigins(frame, range, pts)
		var ptheight: CGFloat = pts[0].y - pts[1].y
		var heightRemaining = size.height - ptheight
		for i in 1..<(n - 1) {
			let next = pts[i].y - pts[i + 1].y
			if next != ptheight {
				print("height mismatch at line index \(i) of \(n): expected \(ptheight) got \(next) - \(s) \(wid) \(nm)")
				return
			}
			heightRemaining -= next
		}
		let plast = CFArrayGetValueAtIndex(lines, n - 1)
		let last = Unmanaged<CTLine>.fromOpaque(plast!).takeUnretainedValue()
		var lastlead: CGFloat = 0
		CTLineGetTypographicBounds(last, nil, nil, &lastlead)
		ptheight -= floor(lastlead + 0.5)
		if heightRemaining != ptheight {
			print("last line difference: ptheight-lastlead \(ptheight) remaining \(heightRemaining) - \(s) \(wid) \(nm)")
			return
		}
		pass += 1
	}
	func runForDesc(_ d: CTFontDescriptor) {
		let nm = CTFontDescriptorCopyAttribute(d, kCTFontDisplayNameAttribute) as! CFString
		if CFStringFind(nm, "Klee" as! NSString as! CFString, []).location == kCFNotFound {
			return
		}
		for ps in [CGFloat](arrayLiteral: 0, 8, 10, 12, 14, 16, 18, 20, 22, 24, 36, 48, 72) {
			var wid: CGFloat = 0
			while wid < 5001 {
				runForFontWidth(d, ps, wid, nm)
				wid += 0.1
			}
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
	print("done - \(pass)/\(total) pass")
}
iterative3()
