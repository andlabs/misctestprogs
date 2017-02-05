// 5 february 2017
// macdashstroketest.m 15 october 2015
#import <Cocoa/Cocoa.h>

@interface dashStrokeView : NSView
@end

CTFontRef font = NULL, font2 = NULL;

void putstr(CGContextRef c, const char *str, double x, double y, int which)
{
	CFStringRef string;
	CFStringRef keys[1];
	CFTypeRef values[1];
	CFDictionaryRef attrs;
	CFAttributedStringRef attrstr;
	CGPathRef path;
	CTFramesetterRef fs;
	CTFrameRef frame;
	CTLineRef line;

	if (font == NULL) {
		CGDataProviderRef dp;
		CGFontRef cgfont;

		dp = CGDataProviderCreateWithURL((CFURLRef) [NSURL fileURLWithPath:@"arial_no_gpos.ttf"]);
		cgfont = CGFontCreateWithDataProvider(dp);
		font = CTFontCreateWithGraphicsFont(cgfont, 14, NULL, NULL);
	}
	if (font2 == NULL)
		font2 = CTFontCreateWithNameAndOptions(CFSTR("ArialMT"), 14, NULL, 0);

	string = CFStringCreateWithCString(kCFAllocatorDefault,
		str, kCFStringEncodingUTF8);
	keys[0] = kCTFontAttributeName;
	values[0] = font;
	if (which == 2) values[0] = font2;
	attrs = CFDictionaryCreate(kCFAllocatorDefault,
		keys, values,
		1,
		&kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	attrstr = CFAttributedStringCreate(kCFAllocatorDefault, string, attrs);

	fs = CTFramesetterCreateWithAttributedString(attrstr);
	path = CGPathCreateWithRect(CGRectMake(x, y, 200, 200), NULL);
	frame = CTFramesetterCreateFrame(fs,
		CFRangeMake(0, CFAttributedStringGetLength(attrstr)),
		path,
		NULL);
	line = (CTLineRef) CFArrayGetValueAtIndex(CTFrameGetLines(frame), 0);
	CFRelease(path);

	CGContextSaveGState(c);
	CGContextSetRGBFillColor(c, 0.0, 1.0, 0.0, 0.5);
	{CGRect r;
	r=CTLineGetBoundsWithOptions(line, 0);
	r.size.height = CGRectGetMaxY(r);
	NSLog(@"%g %g -> %g", x, y, r.size.height);
	r.origin.x = x;
	r.origin.y = y;
	CGContextFillRect(c, r);}
	CGContextRestoreGState(c);

//	CGContextSetTextPosition(c, x, y);
	CTFrameDraw(frame, c);

	CFRelease(frame);
	CFRelease(fs);
	CFRelease(attrstr);
	CFRelease(attrs);
	CFRelease(string);
}

@implementation dashStrokeView

- (void)drawRect:(NSRect)r
{
	CGContextRef c;

	c = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];

	putstr(c, "A\xCC\xAA\xEF\xB8\xA0", 100, 100, 1);
	putstr(c, "A\xCC\xAA\xEF\xB8\xA0", 130, 100, 2);

	CGContextSaveGState(c);
	CGContextSetRGBFillColor(c, 1.0, 0.0, 0.0, 0.5);
	CGContextFillRect(c, CGRectMake(100, 100, 20,
		floor(CTFontGetAscent(font2) + 0.5)
		));
	CGContextRestoreGState(c);
}

- (BOOL)isFlipped
{
	return NO;
}

@end

@interface appDelegate : NSObject<NSApplicationDelegate>
@end

@implementation appDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
	NSWindow *mainwin;
	NSView *contentView;
	dashStrokeView *view;
	NSDictionary *views;
	NSArray *constraints;

	mainwin = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, 320, 360)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	[mainwin setTitle:@"Dash/Stroke Example"];
	contentView = [mainwin contentView];

	view = [[dashStrokeView alloc] initWithFrame:NSZeroRect];
	[view setTranslatesAutoresizingMaskIntoConstraints:NO];
	[contentView addSubview:view];

	views = NSDictionaryOfVariableBindings(view);
	constraints = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-[view]-|"
		options:0
		metrics:nil
		views:views];
	[contentView addConstraints:constraints];
	constraints = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|-[view]-|"
		options:0
		metrics:nil
		views:views];
	[contentView addConstraints:constraints];

	[mainwin cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
	[mainwin makeKeyAndOrderFront:nil];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
	return YES;
}

@end

int main(void)
{
	NSApplication *app;

	app = [NSApplication sharedApplication];
	[app setActivationPolicy:NSApplicationActivationPolicyRegular];
	[app setDelegate:[appDelegate new]];
	[app run];
	return 0;
}
