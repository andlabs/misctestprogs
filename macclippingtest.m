// 17 august 2015
#import <Cocoa/Cocoa.h>

BOOL useInsets = NO;

@interface ContainerView : NSView
@end

@implementation ContainerView

- (NSEdgeInsets)alignmentRectInsets
{
	if (useInsets)
		return NSEdgeInsetsMake(50, 50, 50, 50);
	return [super alignmentRectInsets];
}

@end

NSWindow *mainwin;
NSView *containerView;
NSProgressIndicator *progressbar;
NSPopUpButton *popupbutton;
NSButton *checkbox;

void addConstraints(NSView *view, NSString *constraint, NSDictionary *views)
{
	NSArray *constraints = [NSLayoutConstraint constraintsWithVisualFormat:constraint
		options:0
		metrics:nil
		views:views];
	[view addConstraints:constraints];
}

void relayout(BOOL spaced)
{
	[containerView removeConstraints:[containerView constraints]];
	NSDictionary *views = @{
		@"pbar":		progressbar,
		@"pbutton":	popupbutton,
		@"checkbox":	checkbox,
	};
	NSString *vconstraint = @"V:|[pbar][pbutton][checkbox]|";
	if (spaced)
		vconstraint = @"V:|[pbar]-[pbutton]-[checkbox]|";
	addConstraints(containerView, vconstraint, views);
	addConstraints(containerView, @"H:|[pbar]|", views);
	addConstraints(containerView, @"H:|[pbutton]|", views);
	addConstraints(containerView, @"H:|[checkbox]|", views);

	NSView *contentView = [mainwin contentView];
	[contentView removeConstraints:[contentView constraints]];
	NSString *base = @":|[view]|";
	if (spaced)
		base = @":|-[view]-|";
	views = @{
		@"view":		containerView,
	};
	addConstraints(contentView, [@"H" stringByAppendingString:base], views);
	addConstraints(contentView, [@"V" stringByAppendingString:base], views);
}

@interface appDelegate : NSObject<NSApplicationDelegate>
@end

@implementation appDelegate

- (IBAction)onChecked:(id)sender
{
	relayout([checkbox state] == NSOnState);
}

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
	mainwin = [[NSWindow alloc]
		initWithContentRect:NSMakeRect(0, 0, 320, 240)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	NSView *contentView = [mainwin contentView];

	containerView = [[ContainerView alloc] initWithFrame:NSZeroRect];
	[containerView setTranslatesAutoresizingMaskIntoConstraints:NO];

	progressbar = [[NSProgressIndicator alloc] initWithFrame:NSZeroRect];
	[progressbar setControlSize:NSRegularControlSize];
	[progressbar setBezeled:YES];
	[progressbar setStyle:NSProgressIndicatorBarStyle];
	[progressbar setIndeterminate:NO];
	[progressbar setTranslatesAutoresizingMaskIntoConstraints:NO];
	[containerView addSubview:progressbar];

	popupbutton = [[NSPopUpButton alloc] initWithFrame:NSZeroRect];
	[popupbutton setPreferredEdge:NSMinYEdge];
	NSPopUpButtonCell *pbcell = (NSPopUpButtonCell *) [popupbutton cell];
	[pbcell setArrowPosition:NSPopUpArrowAtBottom];
	[popupbutton addItemWithTitle:@"Item 1"];
	[popupbutton addItemWithTitle:@"Item 2"];
	[popupbutton setTranslatesAutoresizingMaskIntoConstraints:NO];
	[containerView addSubview:popupbutton];

	checkbox = [[NSButton alloc] initWithFrame:NSZeroRect];
	[checkbox setTitle:@"Spaced"];
	[checkbox setButtonType:NSSwitchButton];
	[checkbox setBordered:NO];
	[checkbox setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
	[checkbox setTarget:self];
	[checkbox setAction:@selector(onChecked:)];
	[checkbox setTranslatesAutoresizingMaskIntoConstraints:NO];
	[containerView addSubview:checkbox];

	[contentView addSubview:containerView];
	relayout(NO);

	[mainwin cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
	[mainwin makeKeyAndOrderFront:mainwin];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
	return YES;
}

@end

int main(int argc, char *argv[])
{
	useInsets = (argc > 1);

	NSApplication *app = [NSApplication sharedApplication];
	[app setActivationPolicy:NSApplicationActivationPolicyRegular];
	[app setDelegate:[appDelegate new]];
	[app run];
	return 0;
}
