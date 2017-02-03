// 5 january 2017
#import <Cocoa/Cocoa.h>

int main(int argc, char *argv[])
{
	CFMutableDictionaryRef attrs;
	CFStringRef s;
	CTFontRef font;
	int i;
	CFAttributedStringRef cfas;
	CTFramesetterRef fs;
	CFRange range;
	CFRange unused;
	CGSize size;
	CGRect rect;
	CGPathRef path;
	CTFrameRef frame;

	attrs = CFDictionaryCreateMutable(NULL, 0,
		&kCFCopyStringDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	if (argc > 1 && strcmp(argv[1], "-times") == 0)
		font = CTFontCreateWithName(CFSTR("Times New Roman"), 14, NULL);
	else
		font = CTFontCreateWithName(CFSTR("Helvetica"), 14, NULL);
	CFDictionaryAddValue(attrs, kCTFontAttributeName, font);
	if (argc > 1 && strcmp(argv[1], "longer") == 0)
		s = CFStringCreateWithCString(NULL, "This is a test A\xCC\xAA\xEF\xB8\xA0 and this is a test", kCFStringEncodingUTF8);
	else
		s = CFStringCreateWithCString(NULL, "A\xCC\xAA\xEF\xB8\xA0", kCFStringEncodingUTF8);
	cfas = CFAttributedStringCreate(NULL, s, attrs);
	fs = CTFramesetterCreateWithAttributedString(cfas);
	range = CFRangeMake(0, CFAttributedStringGetLength(cfas));
	size = CTFramesetterSuggestFrameSizeWithConstraints(fs,
		range,
		NULL,
		CGSizeMake(200, CGFLOAT_MAX),
		&unused);
	rect.origin = CGPointMake(5, 2.5);
	rect.size = size;
	path = CGPathCreateWithRect(rect, NULL);
	frame = CTFramesetterCreateFrame(fs, range, path, NULL);
	return 0;
}
