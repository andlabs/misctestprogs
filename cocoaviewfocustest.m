// 7 june 2014
// scratch program 28 may 2014
// borrows some things from the blog GTK+ scratch program (see that + its history)
#import <stdio.h>
#import <stdlib.h>
#import <getopt.h>
#import <Cocoa/Cocoa.h>

// string helpers
#define toNSString(x) ([NSString stringWithUTF8String:(x)])
#define fromNSString(x) ([(x) UTF8String])

@interface AppDelegate : NSObject
@end

AppDelegate *appDelegate;
NSWindow *mainwin;

@implementation AppDelegate

// implementation methods here

@end

#define sysFontSize(x) [NSFont systemFontSizeForControlSize:(NS ## x ## ControlSize)]

@interface myView : NSView {
	NSColor *curColor;
}
@end

@implementation myView

- (id)initWithFrame:(NSRect)r
{
	self = [super initWithFrame:r];
	self->curColor = [NSColor blackColor];
	return self;
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (BOOL)becomeFirstResponder
{
	self->curColor = [NSColor blueColor];
	[self display];
	return YES;
}

- (BOOL)resignFirstResponder
{
	self->curColor = [NSColor blackColor];
	[self display];
	return YES;
}

- (void)drawRect:(NSRect)r
{
	[self->curColor set];
	[NSBezierPath fillRect:r];
}

@end

void buildUI(void)
{
#define SETFONT(c, size) [(c) setFont:[NSFont systemFontOfSize:sysFontSize(size)]]
#define RESIZE(c, mask) [(c) setAutoresizingMask:(mask)]
#define ADD(c) [[mainwin contentView] addSubview:(c)]
	// use SETFONT(Regular/Small/Mini), RESIZE() and ADD()
	// the mask argument to RESIZE() is one of the NSView resizing masks

	myView *v;
	NSScrollView *sv;
	NSRect frame;

	v = [[myView alloc] initWithFrame:NSMakeRect(0, 0, 250, 150)];
	frame = [[mainwin contentView] frame];
	frame.origin.y += 20;
	frame.size.height -= 20;
	sv = [[NSScrollView alloc] initWithFrame:frame];
	[sv setDocumentView:v];
	[sv setHasHorizontalScroller:YES];
	[sv setHasVerticalScroller:YES];
	RESIZE(sv, NSViewWidthSizable | NSViewHeightSizable);
	ADD(sv);

	// a simple NSTextField so we can focus on something else
	NSTextField *f;

	f = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 250, 20)];
	ADD(f);
}

void init(int, char *[]);

int main(int argc, char *argv[])
{
	[NSApplication sharedApplication];
	if ([NSApp setActivationPolicy:NSApplicationActivationPolicyRegular] != YES) {
		fprintf(stderr, "error activating Cocoa application\n");
		return 1;
	}
	[NSApp activateIgnoringOtherApps:TRUE];
	appDelegate = [AppDelegate new];
	[NSApp setDelegate:appDelegate];
	[NSApp run];
	return 0;
}

@implementation AppDelegate (usuallyTheSame)

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
	mainwin = [[NSWindow alloc]
		initWithContentRect:NSMakeRect(300, 300, 320, 240)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask |
			NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	[mainwin setTitle:@"Main Window"];
	[[mainwin contentView] setAutoresizesSubviews:YES];
	buildUI();
	[mainwin makeKeyAndOrderFront:self];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
	return YES;
}

@end
