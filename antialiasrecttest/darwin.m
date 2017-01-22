// 22 january 2017
#define CHARTYPE char
#define TOCHAR(s) s
#import <Cocoa/Cocoa.h>
#import "shared.h"

@interface rectView : NSView {
	int current;
	BOOL antialias;
	BOOL sameColor;
}
- (IBAction)currentChanged:(id)sender;
- (IBAction)antialiasChanged:(id)sender;
- (IBAction)sameColorChanged:(id)sender;
@end

@implementation rectView

- (id)initWithFrame:(NSRect)r
{
	self = [super initWithFrame:r];
	if (self) {
		self->current = 0;
		self->antialias = YES;
		self->sameColor = NO;
	}
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

	CGContextSaveGState(c);
	CGContextSetShouldAntialias(c, self->antialias != NO);

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
		if (!self->sameColor)
			cc = (cc + 1) % 4;
	}

	CGContextRestoreGState(c);
}

- (IBAction)currentChanged:(id)sender
{
	NSPopUpButton *pb = (NSPopUpButton *) sender;

	self->current = [pb indexOfSelectedItem];
	[self setNeedsDisplay:YES];
}

- (IBAction)antialiasChanged:(id)sender
{
	NSButton *cb = (NSButton *) sender;

	self->antialias = [cb state] != NSOffState;
	[self setNeedsDisplay:YES];
}

- (IBAction)sameColorChanged:(id)sender
{
	NSButton *cb = (NSButton *) sender;

	self->sameColor = [cb state] != NSOffState;
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
	NSButton *antialias;
	NSButton *sameColor;
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

	antialias = [[NSButton alloc] initWithFrame:NSZeroRect];
	[antialias setTitle:@"Antialias"];
	[antialias setButtonType:NSSwitchButton];
	// doesn't seem to have an associated bezel style
	[antialias setBordered:NO];
	[antialias setTransparent:NO];
	[antialias setState:NSOnState];
	[antialias setTarget:rv];
	[antialias setAction:@selector(antialiasChanged:)];
	[antialias setTranslatesAutoresizingMaskIntoConstraints:NO];
	[contentView addSubview:antialias];

	sameColor = [[NSButton alloc] initWithFrame:NSZeroRect];
	[sameColor setTitle:@"Same Color"];
	[sameColor setButtonType:NSSwitchButton];
	// doesn't seem to have an associated bezel style
	[sameColor setBordered:NO];
	[sameColor setTransparent:NO];
	[sameColor setTarget:rv];
	[sameColor setAction:@selector(sameColorChanged:)];
	[sameColor setTranslatesAutoresizingMaskIntoConstraints:NO];
	[contentView addSubview:sameColor];

	views = NSDictionaryOfVariableBindings(chooser, antialias, sameColor, rv);
	addConstraints = ^(NSString *s) {
		NSArray<NSLayoutConstraint *> *a;

		a = [NSLayoutConstraint constraintsWithVisualFormat:s
			options:0
			metrics:nil
			views:views];
		[contentView addConstraints:a];
	};
	addConstraints(@"H:|-[chooser]-|");
	addConstraints(@"H:|-[antialias]-[sameColor(==antialias)]-|");
	addConstraints(@"H:|-[rv]-|");
	addConstraints(@"V:|-[chooser][antialias]-[rv]-|");
	addConstraints(@"V:[chooser][sameColor]");

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
