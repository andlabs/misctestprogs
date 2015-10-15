// 15 october 2015
#import <Cocoa/Cocoa.h>

@interface dashStrokeView : NSView
@end

void putstr(CGContextRef c, const char *str, double x, double y)
{
	NSFont *sysfont;
	CFStringRef string;
	CTFontRef font;
	CFStringRef keys[1];
	CFTypeRef values[1];
	CFDictionaryRef attrs;
	CFAttributedStringRef attrstr;
	CTLineRef line;

	sysfont = [NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]];
	font = (CTFontRef) sysfont;		// toll-free bridge

	string = CFStringCreateWithCString(kCFAllocatorDefault,
		str, kCFStringEncodingUTF8);
	keys[0] = kCTFontAttributeName;
	values[0] = font;
	attrs = CFDictionaryCreate(kCFAllocatorDefault,
		keys, values,
		1,
		&kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	attrstr = CFAttributedStringCreate(kCFAllocatorDefault, string, attrs);

	line = CTLineCreateWithAttributedString(attrstr);
	CGContextSetTextPosition(c, x, y);
	CTLineDraw(line, c);

	CFRelease(line);
	CFRelease(attrstr);
	CFRelease(attrs);
	CFRelease(string);
}

@implementation dashStrokeView

- (void)drawRect:(NSRect)r
{
	CGContextRef c;
	CGFloat lengths[2] = { 5, 3 };
	CGMutablePathRef buildpath;
	CGPathRef copy, copy2;

	c = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];

	CGContextSaveGState(c);

	putstr(c, "Dash + Stroke With CGContext Functions", 10, 10);
	CGContextMoveToPoint(c, 50, 50);
	CGContextAddLineToPoint(c, 100, 30);
	CGContextAddLineToPoint(c, 150, 70);
	CGContextAddLineToPoint(c, 200, 50);
	CGContextSetLineWidth(c, 10);
	CGContextSetLineJoin(c, kCGLineJoinBevel);
	CGContextSetLineCap(c, kCGLineCapRound);
	CGContextSetLineDash(c, 0, lengths, 2);
	CGContextSetRGBStrokeColor(c, 0, 0, 0, 1);
	CGContextStrokePath(c);

	CGContextTranslateCTM(c, 0, 100);
	putstr(c, "Dash With CGPath Functions", 10, 10);

	CGContextTranslateCTM(c, 0, 100);
	putstr(c, "Dash + Stroke With CGPath Functions", 10, 10);

	CGContextRestoreGState(c);
}

- (BOOL)isFlipped
{
	return YES;
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

	mainwin = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, 320, 240)
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
