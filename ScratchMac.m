// 28 may 2014
// borrows some things from the blog GTK+ scratch program (see that + its history)
#import <stdio.h>
#import <stdlib.h>
#import <getopt.h>
#import <Cocoa/Cocoa.h>

// string helpers
#define toNSString(x) ([NSString stringWithUTF8String:(x)])
#define fromNSString(x) ([(x) UTF8String])

// command line flags go here
// the array format is @[NSString *name, NSString *help, NSValue *arg]
// name should not have a leading - or --
// arg should be @YES or @NO to determine if an argument is taken
NSArray *flagspec(void)
{
	return @[
		// args here
		// no need for -help
	];
}

// after init(), this will contain all the above flag names as keys with NSValue values
// - if the flag takes no argument, the value is either @YES or @NO
// - if the flag takes an argument, the value will be a pointer or NULL if not set
NSMutableDictionary *flags;

// after init(), this will contain the rest of the arguments
NSMutableArray *otherArgs;

// this is for the usage text
// name other command-line arguments here, if any
// for example, "[files...]"
char *args = "";

BOOL parseArgs()
{
	// parse arguments here, if any, from otherArgs
	// return YES if parsed successfully; NO otherwise
	// (returning NO will show usage and quit)
	return YES;
}

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

void init(int, char *[]);

int main(int argc, char *argv[])
{
	init(argc, argv);
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

void init(int argc, char *argv[])
{
	struct option *sopts;
	size_t soptssize;
	NSUInteger i;
	NSArray *fspec;
	int c;
	int soptsindex;
	int usageret = 1;

	fspec = flagspec();
	soptssize = ([fspec count] + 2) * sizeof (struct option);
	sopts = (struct option *) malloc(soptssize);
	if (sopts == NULL) {
		fprintf(stderr, "memory exhausted processing command-line arguments\n");
		exit(1);
	}
	memset(sopts, 0, soptssize);
	flags = [NSMutableDictionary new];
	for (i = 0; i < [fspec count]; i++) {
		NSArray *flag;

		flag = fspec[i];
		sopts[i].name = fromNSString(flag[0]);
		sopts[i].has_arg = no_argument;
		flags[flag[0]] = @NO;
		if ([flag[2] boolValue] == YES) {
			sopts[i].has_arg = required_argument;
			flags[flag[0]] =
				[NSValue valueWithPointer:NULL];
		}
		// leave sopts[i].flag as NULL and sopts[i].val as 0
	}
	sopts[i].name = "help";
	sopts[i].has_arg = no_argument;
	sopts[i].val = 'h';		// leave sopts[i].flag as NULL

	for (;;) {
		c = getopt_long_only(argc, argv, "", sopts, &soptsindex);
		if (c == -1)		// done
			break;
		switch (c) {
		case 0:			// one of our options
			if (sopts[soptsindex].has_arg == no_argument)
				flags[fspec[soptsindex][0]] = @YES;
			else
				flags[fspec[soptsindex][0]] =
					[NSValue valueWithPointer:optarg];
			continue;
		case 'h':			// -help
			usageret = 0;
			goto usage;
		case '?':			// unknown
			// getopt_long_only() will print the error
			goto usage;
		}
		fprintf(stderr, "internal error: getopt_long_only() returned %d\n", c);
		exit(1);
	}

	otherArgs = [NSMutableArray new];
	for (i = (NSUInteger) optind; i < (NSUInteger) argc; i++)
		[otherArgs addObject:toNSString(argv[i])];
	if (!parseArgs())
		goto usage;

	return;

usage:
	fprintf(stderr, "usage: %s [options]", argv[0]);
	if (args != NULL && *args != '\0')
		fprintf(stderr, " %s", args);
	fprintf(stderr, "\n");
	for (i = 0; i < [fspec count]; i++) {
		NSArray *flag;
		char *opt;

		flag = fspec[i];
		opt = "";
		if ([flag[2] boolValue] == YES)
			opt = " arg";
		fprintf(stderr, "\t-%s%s: %s\n",
			fromNSString(flag[0]), opt,
			fromNSString(flag[1]));
	}
	exit(usageret);
}
