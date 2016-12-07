// 6 december 2016
import Cocoa

protocol DecodedType {
	var String: String { get }
	static func Decodes(_ encoded: String) -> Bool
	init(_ encoded: String)
}

class InvalidType : DecodedType {
	var String: String
	static func Decodes(_ encoded: String) -> Bool { return true }
	required init(_ encoded: String) { self.String = "(invalid type " + encoded + ")" }
}

func firstChar(_ s: String) -> String {
	return s.substring(to: s.index(after: s.startIndex))
}

func restOfString(_ s: String) -> String {
	return s.substring(from: s.index(after: s.startIndex))
}

let digits: [String: UIntMax] = [
	"0":	0,
	"1":	1,
	"2":	2,
	"3":	3,
	"4":	4,
	"5":	5,
	"6":	6,
	"7":	7,
	"8":	8,
	"9":	9,
]

func swallowNumber(_ s: String, _ count: inout UIntMax) -> String {
	var t = s
	count = 0
	while true {
		let c = firstChar(t)
		if let d = digits[c] {
			count *= 10
			count += d
			t = restOfString(t)
			continue
		}
		return t
	}
}

class BasicType : DecodedType {
	private var char: String
	
	private static let names: [String: String] = [
		"c":	"char",
		"i":	"int",
		"s":	"short",
		"l":	"long",
		"q":	"long long",
		"C":	"unsigned char",
		"I":	"unsigned int",
		"S":	"unsigned short",
		"L":	"unsigned long",
		"Q":	"unsigned long long",
		"f":	"float",
		"d":	"double",
		"b":	"_Bool",
		"v":	"void",
		"*":	"char *",
	]
	
	public var String: String {
		get {
			return BasicType.names[self.char]!
		}
	}
	
	public static func Decodes(_ encoded: String) -> Bool {
		return BasicType.names.index(forKey: firstChar(encoded)) != nil
	}
	
	public required init(_ encoded: String) {
		self.char = firstChar(encoded)
	}
}

class ObjCObjectType : DecodedType {
	public var String: String {
		get {
			return "id"
		}
	}
	
	public static func Decodes(_ encoded: String) -> Bool {
		return firstChar(encoded) == "@"
	}
	
	public required init(_ encoded: String) {
		// do nothing
	}
}

class ObjCClassType : DecodedType {
	public var String: String {
		get {
			return "Class"
		}
	}
	
	public static func Decodes(_ encoded: String) -> Bool {
		return firstChar(encoded) == "#"
	}
	
	public required init(_ encoded: String) {
		// do nothing
	}
}

class ObjCSelectorType : DecodedType {
	public var String: String {
		get {
			return "SEL"
		}
	}
	
	public static func Decodes(_ encoded: String) -> Bool {
		return firstChar(encoded) == ":"
	}
	
	public required init(_ encoded: String) {
		// do nothing
	}
}

class ArrayType : DecodedType {
	private var elem: DecodedType
	private var count: UIntMax = 0
	
	public var String: String {
		get {
			return self.elem.String + "[\(self.count)]"
		}
	}
	
	public static func Decodes(_ encoded: String) -> Bool {
		return firstChar(encoded) == "[" &&
			encoded.characters.count >= 4
		// TODO also look for the ]? and the number?
	}
	
	public required init(_ encoded: String) {
		var e2 = restOfString(encoded)
		e2 = swallowNumber(e2, &self.count)
		self.elem = decodeEncodedType(e2)
	}
}

var decodedTypes: [DecodedType.Type] = [
	BasicType.self,
	ObjCObjectType.self,
	ObjCClassType.self,
	ObjCSelectorType.self,
	ArrayType.self,
]

func decodeEncodedType(_ encoded: String) -> DecodedType {
	for i in 0..<decodedTypes.count {
		let t = decodedTypes[i]
		if t.Decodes(encoded) {
			return t.init(encoded)
		}
	}
	return InvalidType(encoded)
}
