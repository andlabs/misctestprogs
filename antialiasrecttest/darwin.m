// 22 january 2017
#define CHARTYPE char
#define TOCHAR(s) s
#import <Cocoa/Cocoa.h>
#import "shared.h"

@interface rectView : NSView {
	int current;
}
- (IBAction)currentChanged:(id)sender;
@end

@implementation rectView

- (id)initWithFrame:(NSRect)r
{
	self = [super initWithFrame:r];
	if (self)
		self->current = 0;
	return self;
}

- (BOOL)isFlipped
{
	return YES;
}

- (void)drawRect:(NSRect)r
{
	const struct rect *cur = rectList[self->current].rects;
	CGContextRef c;
	int cc = 0;

	c = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];

	while (cur->x < 50) {
		CGMutablePathRef path;
		CGRect rr;

		path = CGPathCreateMutable();
		rr.origin.x = cur->x;
		rr.origin.y = cur->y;
		rr.size.width = cur->width;
		rr.size.height = cur->height;
		CGPathAddRect(path, NULL, rr);
		CGContextSetRGBFillColor(c,
			fillcolors[cc].r,
			fillcolors[cc].g,
			fillcolors[cc].b,
			fillcolors[cc].a);
		CGContextAddPath(c, path);
		CGContextFillPath(c);
		CFRelease(path);
		cur++;
		cc = (cc + 1) % 4;
	}
}

- (IBAction)currentChanged:(id)sender
{
	NSPopUpButton *pb = (NSPopUpButton *) sender;

	self->current = [pb indexOfSelectedItem];
	[self setNeedsDisplay:YES];
}

@end

@interface appDelegate : NSObject<NSApplicationDelegate>
@end

@implementation appDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
	NSWindow *mainwin;
	__block NSView *contentView;
	NSPopUpButton *chooser;
	NSPopUpButtonCell *pbcell;
	int i;
	rectView *rv;
	__block NSDictionary *views;
	void (^addConstraints)(NSString *s);

	mainwin = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 640, 480)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	contentView = [mainwin contentView];

	rv = [[rectView alloc] initWithFrame:NSZeroRect];
	[rv setTranslatesAutoresizingMaskIntoConstraints:NO];
	[contentView addSubview:rv];

	chooser = [[NSPopUpButton alloc] initWithFrame:NSZeroRect pullsDown:NO];
	[chooser setPreferredEdge:NSMinYEdge];
	pbcell = (NSPopUpButtonCell *) [chooser cell];
	[pbcell setArrowPosition:NSPopUpArrowAtBottom];
	[chooser setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
	[chooser setTranslatesAutoresizingMaskIntoConstraints:NO];
	[contentView addSubview:chooser];

	for (i = 0; rectList[i].name != NULL; i++)
		[chooser addItemWithTitle:[NSString stringWithUTF8String:rectList[i].name]];
	[chooser selectItemAtIndex:0];
	[chooser setTarget:rv];
	[chooser setAction:@selector(currentChanged:)];

	views = NSDictionaryOfVariableBindings(chooser, rv);
	addConstraints = ^(NSString *s) {
		NSArray<NSLayoutConstraint *> *a;

		a = [NSLayoutConstraint constraintsWithVisualFormat:s
			options:0
			metrics:nil
			views:views];
		[contentView addConstraints:a];
	};
	addConstraints(@"H:|-[chooser]-|");
	addConstraints(@"H:|-[rv]-|");
	addConstraints(@"V:|-[chooser]-[rv]-|");

	[mainwin center];
	[mainwin makeKeyAndOrderFront:self];
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
