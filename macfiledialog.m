/* 31 march-1 april 2014 */
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>

static BOOL mkdir = NO;
static BOOL showhidden = NO;
static char *initpath = NULL;
static char *defname = NULL;

/* cheating: we store the help string in the flag argument, collect them, then overwrite them with NULL in init() so getopt_long_only() will return val and not overwrite a string (apparently I'm not the first to think of this: GerbilSoft says GNU tools do this too) */
#define flagBool(name, help, short) { name, no_argument, (int *) help, short }
#define flagString(name, help, short) { name, required_argument, (int *) help, short }
static struct option flags[] = {
	flagBool("mkdir", "provide New Folder button", 'M'),
	flagBool("help", "show help and quit", 'h'),
	flagBool("showhidden", "show hidden files", 'H'),
	flagString("initpath", "specify an initial filename", 'p'),
	flagString("defname", "specify a default filename", 'd'),
	{ 0, 0, 0, 0 },
};

static id panel;

#define STR(s) [NSString stringWithUTF8String:(s)]

void init(int argc, char *argv[])
{
	int usageExit = 1;
	char *mode;
	char *opthelp[512];		/* more than enough */
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
		case 'M':
			mkdir = YES;
			break;
		case 'H':
			showhidden = YES;
			break;
		case 'p':
			initpath = optarg;
			break;
		case 'd':
			defname = optarg;
			break;
		case 'h':		/* -help */
			usageExit = 0;
			goto usage;
		case '?':
			/* getopt_long_only() should have printed something since we did not set opterr to 0 */
			goto usage;
		default:
			fprintf(stderr, "internal error: getopt_long_only() returned %d\n", c);
			exit(1);
		}
	}

	if (optind != argc - 1)		/* equivalent to argc != 2 */
		goto usage;
	mode = argv[optind];
	if (strcmp(mode, "open") == 0)
		panel = [NSOpenPanel openPanel];
	else if (strcmp(mode, "save") == 0)
		panel = [NSSavePanel savePanel];
	else if (strcmp(mode, "opendir") == 0) {
		panel = [NSOpenPanel openPanel];
		[panel setCanChooseFiles:NO];
		[panel setCanChooseDirectories:YES];
	} else {
		fprintf(stderr, "error: unknown mode %s\n", mode);
		goto usage;
	}
	/* TODO retain? docs say might be necessary */

	return;

usage:
	fprintf(stderr, "usage: %s [options] {open|save|opendir}\n", argv[0]);
	for (i = 0; flags[i].name != 0; i++)
		fprintf(stderr, "\t-%s%s - %s\n",
			flags[i].name,
			(flags[i].has_arg == required_argument) ? " string" : "",
			opthelp[i]);
	exit(usageExit);
}

int main(int argc, char *argv[])
{
	[NSApplication sharedApplication];
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	[NSApp activateIgnoringOtherApps:YES];
	init(argc, argv);
	id arp = [NSAutoreleasePool new];

	[panel setCanCreateDirectories:mkdir];
	[panel setShowsHiddenFiles:showhidden];
	if (initpath != NULL)
		[panel setDirectoryURL:[NSURL fileURLWithPath:STR(initpath)]];
	if (defname != NULL)
		[panel setNameFieldStringValue:STR(defname)];

	[panel setAllowedFileTypes:
		[NSArray arrayWithObjects:@"c",@"d",nil]];
	[panel setAllowsOtherFileTypes:YES];

	if ([panel runModal] == NSFileHandlingPanelOKButton) {
		printf("user selection made\n");
		printf("filename: %s\n", [[[panel URL] path] UTF8String]);
		printf("URL: %s\n", [[[panel URL] absoluteString] UTF8String]);
		if ([panel isKindOfClass:[NSOpenPanel class]]) {
			NSArray *filenames = [NSArray new];
			NSArray *urls = [NSArray new];
			id url;
			NSEnumerator *i;

			i = [[panel URLs] objectEnumerator];
			while (url = [i nextObject]) {
				filenames = [filenames arrayByAddingObject:[url path]];
				urls = [urls arrayByAddingObject:[url absoluteString]];
			}
			printf("filenames: [%s]\nURLs: [%s]\n",
				[[filenames componentsJoinedByString:@"\n\t"] UTF8String],
				[[urls componentsJoinedByString:@"\n\t"] UTF8String]);
		}
		printf("nameFieldStringValue: %s\n", [[panel nameFieldStringValue] UTF8String]);
	} else
		printf("user aborted selection\n");

	[arp release];
	return 0;
}
