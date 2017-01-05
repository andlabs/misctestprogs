// 5 january 2017
#import <Cocoa/Cocoa.h>

int main(void)
{
	CFDictionaryRef attrs;
	NSString *s;
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
//	CFDictionaryAddValue(attrs, kCTFontAttributeName, font);
	s = @"This is a test string.";
	for (i = 0; i < 10; i++)
		s = [s stringByAppendingString:@" This is a test string."];
	cfas = CFAttributedStringCreate(NULL, (CFStringRef) s, attrs);
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
