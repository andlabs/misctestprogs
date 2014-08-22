// <codedate
// scratch mac program 28 may 2014
// borrows some things from the blog GTK+ scratch program (see that + its history)
#import <stdio.h>
#import <stdlib.h>
#import <getopt.h>
#import <Cocoa/Cocoa.h>

// string helpers
#define toNSString(x) ([NSString stringWithUTF8String:(x)])
#define fromNSString(x) ([(x) UTF8String])

NSThread *mainthread;

@implementation NSThread (FuckYou)

+ (NSThread *)mainThread
{
	return mainthread;
}

+ (BOOL)isMainThread
{
	return [[NSThread currentThread] isEqual:mainthread];
}

- (BOOL)isMainThread
{
	return [self isEqual:mainthread];
}

@end

@interface AppDelegate : NSObject
@end

AppDelegate *appDelegate;
NSWindow *mainwin;

@implementation AppDelegate

// implementation methods here

@end

#define sysFontSize(x) [NSFont systemFontSizeForControlSize:(NS ## x ## ControlSize)]

void buildUI(void)
{
#define SETFONT(c, size) [(c) setFont:[NSFont systemFontOfSize:sysFontSize(size)]]
#define RESIZE(c, mask) [(c) setAutoresizingMask:(mask)]
#define ADD(c) [[mainwin contentView] addSubview:(c)]
	// use SETFONT(Regular/Small/Mini), RESIZE() and ADD()
	// the mask argument to RESIZE() is one of the NSView resizing masks

	// add widgets here
}

int main(int argc, char *argv[])
{
	appDelegate = [AppDelegate new];
	mainthread = [[NSThread alloc] initWithTarget:appDelegate
		selector:@selector(threadMain:)
		object:nil];
	[mainthread start];
	while ([mainthread isExecuting]) {
		[NSApp updateWindows];
		[NSThread sleepForTimeInterval:1];
	}
	return 0;
}

// the big magic
// technically an undocumented function, but here's an Apple engineer publicly suggesting use of the function:
// - http://lists.apple.com/archives/darwin-development/2002/Sep/msg00250.html
// and here's a core OS module that uses it:
// - http://www.opensource.apple.com/source/kext_tools/kext_tools-19.2/kextd_main.c
extern void _CFRunLoopSetCurrent(CFRunLoopRef);

@implementation AppDelegate (usuallyTheSame)

- (void)threadMain:(id)unused
{
	NSLog(@"in");
	_CFRunLoopSetCurrent(CFRunLoopGetMain());
	[NSApplication sharedApplication];
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	[NSApp activateIgnoringOtherApps:TRUE];
	[NSApp setDelegate:appDelegate];
	NSLog(@"running");
	[NSApp run];
}

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
	NSLog(@"here");
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
