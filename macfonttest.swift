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
