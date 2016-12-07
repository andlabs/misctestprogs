// 6 december 2016
import Cocoa

func dumpIvar(_ s: NSObject) {
	var rty: AnyClass? = type(of: s)
	while let ty = rty {
		let count = UnsafeMutablePointer<UInt32>.allocate(capacity: 1)
		let rlistptr = class_copyIvarList(ty, count)
		let rlist = UnsafeMutableBufferPointer<Ivar?>(start: rlistptr, count: Int(count.pointee))
		rlist.forEach { (iopt) in
			let ivar = iopt!
			let rname = ivar_getName(ivar)
			let name = String(utf8String: UnsafePointer<CChar>(rname!))!
			let rtyp = ivar_getTypeEncoding(ivar)
			let typ = String(cString: rtyp!)
			var decoded = decodeEncodedType(typ)
			print("\(decoded.String)")
			if !(decoded is ObjCObjectType) {		// object_getIvar() returns objects
				print("\(name): \(typ)")
				return
			}
			let val = object_getIvar(s, ivar)
			print("\(name): \(typ) -> \(val)")
			if let realval = val {
				print("[")
				dumpIvar(realval as! NSObject)
				print("]")
			}
		}
		free(rlistptr)
		rty = ty.superclass()
	}
}

func main() {
var s = NSMutableAttributedString(string: "abc")
s.setAttributes([NSForegroundColorAttributeName: NSColor.red], range: NSMakeRange(0, 1))
s.setAttributes([NSForegroundColorAttributeName: NSColor.green], range: NSMakeRange(1, 1))
s.setAttributes([NSForegroundColorAttributeName: NSColor.blue], range: NSMakeRange(2, 1))
print(s.description); print("==")
s.replaceCharacters(in: NSMakeRange(1, 0), with: String("d"))
print(s.description); print("--")
var md = NSMutableData()
var ka = NSKeyedArchiver(forWritingWith: md)
ka.outputFormat = .xml
ka.encode(s, forKey: "root")
ka.finishEncoding()
print("\(String.init(data: md as Data, encoding: .utf8)!)"); print("--")
dumpIvar(s); print("==")
s.addAttributes([NSForegroundColorAttributeName: NSColor.controlHighlightColor], range: NSMakeRange(0, 1))
print(s.description); print("--")
md = NSMutableData()
ka = NSKeyedArchiver(forWritingWith: md)
ka.outputFormat = .xml
ka.encode(s, forKey: "root")
ka.finishEncoding()
print("\(String.init(data: md as Data, encoding: .utf8)!)"); print("--")
dumpIvar(s)
}
main()
