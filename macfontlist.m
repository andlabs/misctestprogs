// 22 december 2015
#include <Cocoa/Cocoa.h>

void printstr(NSString *str)
{
	printf("%s\n", [str UTF8String]);
}

void enumCoreText(BOOL noDups)
{
	const void *keys[1];
	const void *values[1];
	SInt32 one = 1;
	CFNumberRef cfone;
	CFDictionaryRef options;
	CTFontCollectionRef collection;
	CFArrayRef fonts;
	CFIndex n, i;

	options = NULL;
	if (noDups) {
		cfone = CFNumberCreate(NULL, kCFNumberSInt32Type, &one);
		keys[0] = kCTFontCollectionRemoveDuplicatesOption;
		values[0] = cfone;
		options = CFDictionaryCreate(NULL,
			keys, values, 1,
			NULL, NULL);
	}
	collection = CTFontCollectionCreateFromAvailableFonts(options);
	fonts = CTFontCollectionCreateMatchingFontDescriptors(collection);

	n = CFArrayGetCount(fonts);
	for (i = 0; i < n; i++) {
		CTFontDescriptorRef font;
		CFStringRef familystr;

		font = (CTFontDescriptorRef) CFArrayGetValueAtIndex(fonts, i);
		familystr = (CFStringRef) CTFontDescriptorCopyAttribute(font, kCTFontFamilyNameAttribute);
		printstr((NSString *) familystr);
		CFRelease(familystr);
		// Get Rule means we do not free font, apparently
	}

	CFRelease(fonts);
	CFRelease(collection);
	if (noDups) {
		CFRelease(options);
		CFRelease(cfone);
	}
}

void enumCocoa(BOOL noDups)
{
	NSDictionary *options;
	NSFontCollection *collection;
	NSArray *fonts;
	NSUInteger i, n;

	options = nil;
	if (noDups)
		options = [NSDictionary dictionaryWithObject:@YES
			forKey:NSFontCollectionRemoveDuplicatesOption];
	collection = [NSFontCollection fontCollectionWithAllAvailableDescriptors];
	fonts = [collection matchingDescriptorsWithOptions:options];

	n = [fonts count];
	for (i = 0; i < n; i++) {
		NSFontDescriptor *desc;
		NSString *familystr;

		desc = (NSFontDescriptor *) [fonts objectAtIndex:i];
		familystr = (NSString *) [desc objectForKey:NSFontFamilyAttribute];
		printstr(familystr);
		// TODO what of the above do we release?
	}

	// TODO what of the above do we release?
}

int main(int argc, char *argv[])
{
	BOOL cocoa = NO;
	BOOL noDups = NO;
	int i;

	for (i = 1; i < argc; i++)
		if (strcmp(argv[i], "-cocoa") == 0)
			cocoa = YES;
		else if (strcmp(argv[i], "-nodups") == 0)
			noDups = YES;
		else {
			fprintf(stderr, "usage: %s [-cocoa] [-nodups]\n", argv[0]);
			return 1;
		}

	if (cocoa)
		enumCocoa(noDups);
	else
		enumCoreText(noDups);

	return 0;
}
