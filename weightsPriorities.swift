// 13-14 october 2017, possibly also 12 october too but I forget now
import Foundation
import CoreText

// change to true to show family names
let optShowFamilyNames: Bool = false

// we need this because enums marked @objc (like these) don't preserve reflection info (including value names)
// thanks to various stackoverflow pages and mikeash in irc.freenode.net/#swift-lang
func priorityName(_ pri: Int) -> String {
	let priorityNames: [Int: String] = [
		kCTFontPriorityUser: "user",
		kCTFontPrioritySystem: "system",
		kCTFontPriorityDynamic: "dynamic",
		kCTFontPriorityNetwork: "network",
		kCTFontPriorityProcess: "process",
		kCTFontPriorityComputer: "computer",
	]
	
	return priorityNames[pri] ?? "unknown"
}

func priorityOf(_ font: CTFont) -> Int {
	let pri = CTFontCopyAttribute(font, kCTFontPriorityAttribute) as! NSNumber
	return pri.intValue
}

func priorityOf(desc: CTFontDescriptor) -> Int {
	let pri = CTFontDescriptorCopyAttribute(desc, kCTFontPriorityAttribute) as! NSNumber
	return pri.intValue
}

func scopeName(_ scope: UInt32) -> String {
	let scopeNames: [UInt32: String] = [
		CTFontManagerScope.none.rawValue: "none",
		CTFontManagerScope.process.rawValue: "process",
		CTFontManagerScope.session.rawValue: "session",
		CTFontManagerScope.user.rawValue: "user",
	]
	
	return scopeNames[scope] ?? "unknown"
}

func scopeOf(_ font: CTFont) -> UInt32 {
	let scope = CTFontCopyAttribute(font, kCTFontRegistrationScopeAttribute) as! NSNumber
	return scope.uint32Value
}

func scopeOf(desc: CTFontDescriptor) -> UInt32 {
	let scope = CTFontDescriptorCopyAttribute(desc, kCTFontRegistrationScopeAttribute) as! NSNumber
	return scope.uint32Value
}

func os2WeightOf(_ font: CTFont) -> String {
	let cftable = CTFontCopyTable(font, CTFontTableTag(kCTFontTableOS2), [])
	if cftable == nil {
		return "n/a"
	}
	let table = cftable! as Data
	var usWeightClass: UInt16 = 0
	usWeightClass |= UInt16(table[4]) << 8
	usWeightClass |= UInt16(table[5])
	return "\(usWeightClass)"
}

func os2WeightOf(desc: CTFontDescriptor) -> String {
	return os2WeightOf(CTFontCreateWithFontDescriptor(desc, 0.0, nil))
}

func each<T>(_ item: T,
	_ getAttrFunc: (T, CFString) -> CFTypeRef?,
	_ getPriorityFunc: (T) -> Int,
	_ getScopeFunc: (T) -> UInt32,
	_ getOS2WeightFunc: (T) -> String) -> String {
	let dname = getAttrFunc(item, kCTFontNameAttribute) as! CFString
	let dcffam = getAttrFunc(item, kCTFontFamilyNameAttribute) as! CFString
	var dfam = ""
	if optShowFamilyNames {
		dfam = "\(dcffam)\t"
	}
	let dstyle = getAttrFunc(item, kCTFontStyleNameAttribute) as! CFString
	let dtraits = getAttrFunc(item, kCTFontTraitsAttribute) as! [String: Any]
	let os2 = getOS2WeightFunc(item)
	let dweight = dtraits[kCTFontWeightTrait as String] as! NSNumber
	let dweightval = dweight.floatValue
	let dpri = getPriorityFunc(item)
	let dpristr = priorityName(dpri)
	let dscope = getScopeFunc(item)
	let dscopestr = scopeName(dscope)
	return "\(dname)\t\(dfam)\(dstyle)\t\(os2)\t\(dweightval)\t\(dpri)\t\(dpristr)\t\(dscope)\t\(dscopestr)"
}

func each(font: CTFont) -> String {
	return each(font,
		CTFontCopyAttribute,
		priorityOf,
		scopeOf,
		os2WeightOf)
}

func each(desc: CTFontDescriptor) -> String {
	return each(desc,
		CTFontDescriptorCopyAttribute,
		priorityOf(desc:),
		scopeOf(desc:),
		os2WeightOf(desc:))
}

let descs = CTFontCollectionCreateMatchingFontDescriptors(CTFontCollectionCreateFromAvailableFonts(nil))
for (i, desc) in (descs as! [CTFontDescriptor]).enumerated() {
	let s = each(desc: desc)
	print("\(i)\t\(s)")
}

// thanks to Eridius in irc.freenode.net/#swift-lang for info about DictionaryLiteral
let uiFonts: DictionaryLiteral<CTFontUIFontType, String> = [
	CTFontUIFontType.none:						"none",
	CTFontUIFontType.user:						"user",
	CTFontUIFontType.userFixedPitch:			"userFixedPitch",
	CTFontUIFontType.system:					"system",
	CTFontUIFontType.emphasizedSystem:			"emphasizedSystem",
	CTFontUIFontType.smallSystem:				"smallSystem",
	CTFontUIFontType.smallEmphasizedSystem:		"smallEmphasizedSystem",
	CTFontUIFontType.miniSystem:				"miniSystem",
	CTFontUIFontType.miniEmphasizedSystem:		"miniEmphasizedSystem",
	CTFontUIFontType.views:						"views",
	CTFontUIFontType.application:				"application",
	CTFontUIFontType.label:						"label",
	CTFontUIFontType.menuTitle:					"menuTitle",
	CTFontUIFontType.menuItem:					"menuItem",
	CTFontUIFontType.menuItemMark:				"menuItemMark",
	CTFontUIFontType.menuItemCmdKey:			"menuItemCmdKey",
	CTFontUIFontType.windowTitle:				"windowTitle",
	CTFontUIFontType.pushButton:				"pushButton",
	CTFontUIFontType.utilityWindowTitle:		"utilityWindowTitle",
	CTFontUIFontType.alertHeader:				"alertHeader",
	CTFontUIFontType.systemDetail:				"systemDetail",
	CTFontUIFontType.emphasizedSystemDetail:	"emphasizedSystemDetail",
	CTFontUIFontType.toolbar:					"toolbar",
	CTFontUIFontType.smallToolbar:				"smallToolbar",
	CTFontUIFontType.message:					"message",
	CTFontUIFontType.palette:					"palette",
	CTFontUIFontType.toolTip:					"toolTip",
	CTFontUIFontType.controlContent:			"controlContent",
]
for (type, name) in uiFonts {
	let f = CTFontCreateUIFontForLanguage(type, 0, nil)
	if f == nil {
		print("** \(name) is NULL")
		continue
	}
	let s = each(font: f!)
	print("\(name)\t\(s)")
}

let filenames: [String] = [
	"/Users/pietro/site/static/static/fonts/lucida/original.ignore/hlsr8r.pfa",
	"/Users/pietro/site/static/static/fonts/lucida/original.ignore/hlsb8r.pfa",
	"/Users/pietro/site/static/static/fonts/lucida/original.ignore/hlsu8r.pfa",
	"/Users/pietro/extsrc/adobe/Adobe Font Folio 11/Western Fonts/Kabel LT Std/KabelLTStd-Light.otf",
	"/Users/pietro/extsrc/adobe/Adobe Font Folio 11/Western Fonts/Kabel LT Std/KabelLTStd-Book.otf",
	"/Users/pietro/extsrc/adobe/Adobe Font Folio 11/Western Fonts/Kabel LT Std/KabelLTStd-Heavy.otf",
	"/Users/pietro/extsrc/adobe/Adobe Font Folio 11/Western Fonts/Kabel LT Std/KabelLTStd-Black.otf",
	"/Users/pietro/extsrc/winfonts/win10fonts/impact.ttf",
	"/Users/pietro/extsrc/fonts/ofl/lato/Lato-Thin.ttf",
	"/Users/pietro/extsrc/fonts/ofl/lato/Lato-Regular.ttf",
	"/Users/pietro/extsrc/fonts/ofl/lato/Lato-Medium.ttf",
	"/Users/pietro/extsrc/fonts/ofl/lato/Lato-SemiBold.ttf",
	"/Users/pietro/extsrc/fonts/ofl/lato/Lato-Bold.ttf",
	"/Users/pietro/extsrc/fonts/ofl/lato/Lato-ExtraBold.ttf",
]
for filename in filenames {
	let basename = (filename as NSString).lastPathComponent
	let url = NSURL(fileURLWithPath: filename) as CFURL
	let cfdescs = CTFontManagerCreateFontDescriptorsFromURL(url)
	var descs: [CTFontDescriptor] = []
	if cfdescs != nil {
		descs = cfdescs as! [CTFontDescriptor]
	}
	if descs.count == 0 {
		print("** \(basename) has no fonts")
		continue
	}
	for (i, desc) in descs.enumerated() {
		let s = each(desc: desc)
		print("\(basename) \(i)\t\(s)")
	}
}

/*

func realFamilyName(_ font: CTFont) -> String {
	return CTFontCopyAttribute(font, kCTFontFamilyNameAttribute) as! String
}

func fontWithFamily(_ familyName: String) -> CTFont {
	let desc = CTFontDescriptorCreateWithAttributes([
		kCTFontFamilyNameAttribute as String:		familyName as CFString,
	] as CFDictionary)
	return CTFontCreateWithFontDescriptor(desc, 10, nil)
}

func report(_ font: CTFont) -> String {
	let pri = priorityOf(font)
	return "real family \(realFamilyName(font)) " +
		"priority \(priorityName(pri)) (\(pri))"
}

func reportFamily(_ familyName: String) -> String {
	return "\(familyName): \(report(fontWithFamily(familyName)))"
}

print(reportFamily("Helvetica"))
print(reportFamily("Impact"))
print(reportFamily("Kabel LT Std"))

*/
