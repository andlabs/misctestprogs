/* 31 march 2014 */
/* for mingw; link with -lcomdlg32 */
/* targets windows 2000 or newer */
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

static int filter = 0;
static char *defname = NULL;
static char *initpath = NULL;
static int multisel = 0;
static int confirmnew = 0;
static int oldstyle = 0;

/* cheating: we store the help string in the flag argument, collect them, then overwrite them with NULL in init() so getopt_long_only() will return val and not overwrite a string (apparently I'm not the first to think of this: GerbilSoft says GNU tools do this too) */
#define flagBool(name, help, short) { name, no_argument, (int *) help, short }
#define flagString(name, help, short) { name, required_argument, (int *) help, short }
static struct option flags[] = {
	flagBool("filter", "apply some test filters", 'f'),
	flagString("defname", "specify a default filename", 'd'),
	flagString("initpath", "specify an initial filename", 'p'),
	flagBool("multisel", "allow multiple selection", 'm'),
	flagBool("confirmnew", "confirm on create", 'C'),
	flagBool("oldstyle", "use old-style dialog", 'o'),
	flagBool("help", "show help and quit", 'h'),
	{ 0, 0, 0, 0 },
};

static BOOL WINAPI (*action)(LPOPENFILENAME);

/* "If the buffer is too small, the function returns FALSE and the CommDlgExtendedError function returns FNERR_BUFFERTOOSMALL. In this case, the first two bytes of the lpstrFile buffer contain the required size, in bytes or characters." - so how do we re-get the filename? TODO */
#define ourBufSize 4096 + 2		/* for two null terminators */
static TCHAR filenames[ourBufSize];

#ifdef UNICODE
#define SFMT "%S"
#define SNPRINTF snwprintf
#else
#define SFMT "%s"
#define SNPRINTF snprintf
#endif

UINT_PTR CALLBACK oldStyleHook(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	/* MSDN says to do this */
	return FALSE;
}

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
		case 'f':		/* -filter */
			filter = 1;
			break;
		case 'd':
			defname = optarg;
			break;
		case 'p':
			initpath = optarg;
			break;
		case 'm':
			multisel = 1;
			break;
		case 'C':
			confirmnew = 1;
			break;
		case 'o':
			oldstyle = 1;
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
		action = GetOpenFileName;
	else if (strcmp(mode, "save") == 0)
		action = GetSaveFileName;
	else {
		fprintf(stderr, "error: unknown mode %s\n", mode);
		goto usage;
	}

	return;

usage:
	fprintf(stderr, "usage: %s [options] {open|save}\n", argv[0]);
	for (i = 0; flags[i].name != 0; i++)
		fprintf(stderr, "\t-%s%s - %s\n",
			flags[i].name,
			(flags[i].has_arg == required_argument) ? " string" : "",
			opthelp[i]);
	exit(usageExit);
}

int main(int argc, char *argv[])
{
	OPENFILENAME ofn;

	init(argc, argv);

	ofn.lStructSize = sizeof (ofn);
	ofn.hwndOwner = NULL;
	ofn.hInstance = NULL;

	ofn.lpstrFilter = NULL;
	if (filter)
		ofn.lpstrFilter = TEXT(
			"C files\0*.c\0"
			/* can't really have name only, filter only, or nothing filters because that would imply \0\0 which ends the list */
			"All Files\0*.*\0\0");		/* MSDN says to use this string for links */

	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;

	if (defname != NULL && initpath != NULL)
		SNPRINTF(filenames, ourBufSize, TEXT(SFMT "\\" SFMT), initpath, defname);
	else if (defname != NULL)
		SNPRINTF(filenames, ourBufSize, TEXT(SFMT), defname);
	else if (initpath != NULL)
		/* this is flaky; it either uses initpath as defname or outright crashes */
		SNPRINTF(filenames, ourBufSize, TEXT(SFMT), initpath);
	else
		filenames[0] = '\0';
	ofn.lpstrFile = filenames;
	ofn.nMaxFile = ourBufSize;

	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;

	ofn.Flags = 0;
	if (multisel)
		ofn.Flags |= OFN_ALLOWMULTISELECT;
	if (confirmnew)
		ofn.Flags |= OFN_CREATEPROMPT;
	if (oldstyle) {
		/* needed for an old-style dialog */
		ofn.Flags |= OFN_ENABLEHOOK;
		ofn.lpfnHook = oldStyleHook;
	} else {
		ofn.Flags |= OFN_EXPLORER;
		ofn.lpfnHook = NULL;
	}

	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpTemplateName = NULL;
	ofn.FlagsEx = 0;

	if ((*action)(&ofn) != 0) {
		printf("user selection made\n");
		if (!multisel)
			printf("filename: " SFMT "\n", filenames);
		else {
			int i;
			int divider;

			printf("filenames: [");
			divider = '\0';
			if (oldstyle)
				divider = ' ';

			/* isolate the directory */
			for (i = 0; filenames[i] != divider; i++)
				;
			filenames[i] = '\0';		/* null-terminate */
			i++;

			/* TODO if one filename is selected, take differnet logic */

			while (filenames[i] != divider) {
				int start = i;

				/* do the above but for the current filename */
				for (; filenames[i] != divider; i++)
					;
				filenames[i] = '\0';
				printf(SFMT "\\" SFMT, filenames, &filenames[start]);
				i++;
				if (filenames[i] != divider)
					printf("\n\t");
			}

			printf("]\n");
		}
	} else {
		DWORD ret;

		ret = CommDlgExtendedError();
		if (ret == 0)
			printf("user aborted selection\n");
		else {
			fprintf(stderr, "error in dialog box (return %d)\n", ret);
			return 1;
		}
	}

	return 0;
}
