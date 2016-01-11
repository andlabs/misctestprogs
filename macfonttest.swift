// 11 january 2016
import Cocoa

print("widths")
let collection = NSFontCollection.fontCollectionWithAllAvailableDescriptors()
var results: [String: String] = [:]
var maxlen: Int = 0
for desc: NSFontDescriptor in (collection.matchingDescriptors ?? []) {
	var name = desc.objectForKey(NSFontNameAttribute) as! String
	name += " " + (desc.objectForKey(NSFontFamilyAttribute) as! String)
	let traits = Int(desc.symbolicTraits)
	let traitsDict = desc.objectForKey(NSFontTraitsAttribute) as! NSDictionary
	let width = traitsDict.objectForKey(NSFontWidthTrait)
	if (traits & (NSFontCondensedTrait | NSFontExpandedTrait)) == 0 {
		if width == nil {
			continue
		} else if width as! NSObject == 0 {
			continue
		}
	}
	var props = ""
	if (traits & NSFontExpandedTrait) != 0 {
		props += " expanded "
	} else if (traits & NSFontCondensedTrait) != 0 {
		props += " condensed"
	} else {
		props += " neither  "
	}
	if width == nil {
		props += " no-width"
	} else {
		props += " width=\(width!)"
	}
	results[name] = props.substringFromIndex(props.startIndex.advancedBy(1))
	if maxlen < name.characters.count {
		maxlen = name.characters.count
	}
}
for (name, props) in results {
	let paddedName = name.stringByPaddingToLength(maxlen, withString: " ", startingAtIndex: 0)
	print(paddedName + " | " + props)
}

print("")

print("weights")
results = [:]
maxlen = 0
for desc: NSFontDescriptor in (collection.matchingDescriptors ?? []) {
	var name = desc.objectForKey(NSFontNameAttribute) as! String
	name += " " + (desc.objectForKey(NSFontFamilyAttribute) as! String)
	let traits = Int(desc.symbolicTraits)
	let traitsDict = desc.objectForKey(NSFontTraitsAttribute) as! NSDictionary
	let weight = traitsDict.objectForKey(NSFontWeightTrait)
	if (traits & NSFontBoldTrait) == 0 {
		if weight == nil {
			continue
		} else if weight as! NSObject == 0 {
			continue
		}
	}
	var props = ""
	if (traits & NSFontBoldTrait) != 0 {
		props += " bold"
	} else {
		props += " not "
	}
	if weight == nil {
		props += " no-weight"
	} else {
		props += " weight=\(weight!)"
	}
	results[name] = props.substringFromIndex(props.startIndex.advancedBy(1))
	if maxlen < name.characters.count {
		maxlen = name.characters.count
	}
}
for (name, props) in results {
	let paddedName = name.stringByPaddingToLength(maxlen, withString: " ", startingAtIndex: 0)
	print(paddedName + " | " + props)
}

print("")

print("small caps")
func has(selectors: [NSDictionary], _ what: Int) -> Bool {
	for s in selectors {
		let k = s.objectForKey(kCTFontFeatureSelectorIdentifierKey) as! Int
		if k == what {
			return true
		}
	}
	return false
}
results = [:]
maxlen = 0
for desc: NSFontDescriptor in (collection.matchingDescriptors ?? []) {
	var name = desc.objectForKey(NSFontNameAttribute) as! String
	name += " " + (desc.objectForKey(NSFontFamilyAttribute) as! String)
	let list = desc.objectForKey(kCTFontFeaturesAttribute as String)
	var props = ""
	if list != nil {
		for entry in (list as! [NSDictionary]) {
			let typeID = entry.objectForKey(kCTFontFeatureTypeIdentifierKey) as! Int
			let selectors = entry.objectForKey(kCTFontFeatureTypeSelectorsKey) as! [NSDictionary]
			switch typeID {
			case kLetterCaseType:
				if has(selectors, kSmallCapsSelector) {
					props += " kLetterCaseType"
				}
			case kLowerCaseType:
				if has(selectors, kLowerCaseSmallCapsSelector) {
					props += " kLowerCaseType"
				}
			case kUpperCaseType:
				if has(selectors, kUpperCaseSmallCapsSelector) {
					props += " kUpperCaseType"
				}
			// because programmers and language designers are stupid in general
			// it's just that swift ran with it
			default:
				break
			}
		}
	}
	if props == "" {
		continue
	}
	results[name] = props.substringFromIndex(props.startIndex.advancedBy(1))
	if maxlen < name.characters.count {
		maxlen = name.characters.count
	}
}
for (name, props) in results {
	let paddedName = name.stringByPaddingToLength(maxlen, withString: " ", startingAtIndex: 0)
	print(paddedName + " | " + props)
}
