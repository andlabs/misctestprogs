// 31 july 2015
#import <Cocoa/Cocoa.h>

@interface appDelegate : NSObject<NSApplicationDelegate>
@end

NSButton *makeButton(NSString *);

@implementation appDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
	NSWindow *w;
	NSButton *a, *b, *c, *d;
	NSDictionary *views;
	NSArray *constraints;

	w = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 320, 240)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	[w setTitle:@"Auto Layout Test"];

	a = makeButton(@"A");
	[[w contentView] addSubview:a];
	b = makeButton(@"B");
	[[w contentView] addSubview:b];
	c = makeButton(@"C");
	[[w contentView] addSubview:c];
	d = makeButton(@"D");
	[[w contentView] addSubview:d];

	/* intended layout:
	   +------[Auto Layout Test]------+
	   |[      A      ]               |
	   |[      B      ][      C      ]|
	   |               [      D      ]|
	   |                              | */

	views = NSDictionaryOfVariableBindings(a, b, c, d);

#define CC(x) constraints = [NSLayoutConstraint constraintsWithVisualFormat:x options:0 metrics:nil views:views]
	CC(@"H:|[a(==b)]");
	[[w contentView] addConstraints:constraints];
	CC(@"H:|[b(>=0)][c(==b)]|");
	[[w contentView] addConstraints:constraints];
	CC(@"V:|[a][b]");
	[[w contentView] addConstraints:constraints];
	CC(@"V:|[a][c]");
	[[w contentView] addConstraints:constraints];
	CC(@"H:[d(==b)]|");
	[[w contentView] addConstraints:constraints];
	CC(@"V:[c][d]");
	[[w contentView] addConstraints:constraints];
#undef CC

	[w cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
	[w makeKeyAndOrderFront:self];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)a
{
	return YES;
}

@end

NSButton *makeButton(NSString *text)
{
	NSButton *b;

	b = [[NSButton alloc] initWithFrame:NSZeroRect];
	[b setTitle:text];
	[b setButtonType:NSMomentaryPushInButton];
	[b setBordered:YES];
	[b setBezelStyle:NSRoundedBezelStyle];
	[b setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
	[b setTranslatesAutoresizingMaskIntoConstraints:NO];
	return b;
}

int main(int argc, char *argv[])
{
	NSApplication *app;

	[NSApplication sharedApplication];
	app = (NSApplication *) NSApp;	// shut compiler up to expose real errors; NSApp is id
	[app setActivationPolicy:NSApplicationActivationPolicyRegular];
	[app setDelegate:[appDelegate new]];
	[app run];
	return 0;
}
