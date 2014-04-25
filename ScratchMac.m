// scratch Mac OS X program by pietro gagliardi 24 april 2014
// borrows code from the other scratch programs might as well stop tracking those dates :/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>

// cheating: we store the help string in the flag argument, collect them, then overwrite them with NULL in init() so getopt_long_only() will return val and not overwrite a string (apparently I'm not the first to think of this: GerbilSoft says GNU tools do this too)
#define flagBool(name, help, short) { name, no_argument, (int *) help, short }
#define flagString(name, help, short) { name, required_argument, (int *) help, short }
static struct option flags[] = {
	// place other options here
	flagBool("help", "show help and quit", 'h'),
	{ 0, 0, 0, 0 },
};

#define toNSString(s) [NSString stringWithUTF8String:(s)]
#define fromNSString(s) [(s) UTF8String]

@interface _appDelegate : NSObject @end

@implementation _appDelegate
// app delegate methods here
@end

_appDelegate *appDelegate;

void init(int argc, char *argv[]);

char *args = "";		// other command-line arguments here, if any

BOOL parseArgs(int argc, char *argv[])
{
	// parse arguments here, if any; use optind
	// return YES if parsed successfully; NO otherwise
	// (returning NO will show usage and quit)
	return YES;
}

void buildUI(NSWindow *mainwin)
{
#define ADD(c) [[mainwin contentView] addSubview:(c)];
#define DELEGATE(c) [(c) setDelegate:appDelegate];
#define TARGACT(c, a) [(c) setTarget:appDelegate]; [(c) setAction:@selector(a)];
#define SETFONT(c) [(c) setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
	// build UI here; use ADD(), DELEGATE(), TARGACT(), and SETFONT()
}

@implementation _appDelegate (MainCategory)

- (void)applicationDidFinishLaunching:(NSNotification *)n
{
	NSWindow *mainwin;

	mainwin = [[NSWindow alloc]
		initWithContentRect:NSMakeRect(50, 50, 400, 400)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];		// defer creation until shown
	[mainwin setTitle:@"Main Window"];
	buildUI(mainwin);
	[mainwin makeKeyAndOrderFront:self];
}

@end

int main(int argc, char *argv[])
{
	init(argc, argv);

	appDelegate = [_appDelegate new];
	[NSApplication sharedApplication];
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	[NSApp activateIgnoringOtherApps:YES];		// TODO YES (Russ Cox's devdraw from plan9ports) or NO (Finder)?
	[NSApp setDelegate:appDelegate];
	[NSApp run];
	return 0;
}

void init(int argc, char *argv[])
{
	int usageExit = 1;
	char *opthelp[512];		// more than enough
	int i;

	for (i = 0; flags[i].name != 0; i++) {
		opthelp[i] = (char *) flags[i].flag;
		flags[i].flag = NULL;
	}

	for (;;) {
		int c;

		c = getopt_long_only(argc, argv, "", flags, NULL);
		if (c == -1)
			break;
		switch (c) {
		case 'h':		// -help
			usageExit = 0;
			goto usage;
		case '?':
			// getopt_long_only() should have printed something since we did not set opterr to 0
			goto usage;
		default:
			fprintf(stderr, "internal error: getopt_long_only() returned %d\n", c);
			exit(1);
		}
	}

	if (parseArgs(argc, argv) == YES)
		return;
	// otherwise fall through

usage:
	fprintf(stderr, "usage: %s [options]", argv[0]);
	if (args != NULL && *args != '\0')
		fprintf(stderr, " %s", args);
	fprintf(stderr, "\n");
	for (i = 0; flags[i].name != 0; i++)
		fprintf(stderr, "\t-%s%s - %s\n",
			flags[i].name,
			(flags[i].has_arg == required_argument) ? " string" : "",
			opthelp[i]);
	exit(usageExit);
}

@implementation _appDelegate (WindowShouldCloseCategory)

// this ensures that when we close all the windows, the program closes
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)a
{
	return YES;
}

@end
