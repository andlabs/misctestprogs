/* 31 march 2014 */
/* for mingw; link with -lcomdlg32 */
/* targets windows 2000 or newer */
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <strings.h>
#include <getopt.h>

/* cheating: we store the help string in the flag argument, collect them, then overwrite them with NULL in init() so getopt_long_only() will return val and not overwrite a string (apparently I'm not the first to think of this: GerbilSoft says GNU tools do this too) */
#define flagBool(name, help, short) { name, no_argument, (int *) help, short }
static struct option flags[] = {
	flagBool("help", "show help and quit", 'h'),
	{ 0, 0, 0, 0 },
};

static BOOL WINAPI (*action)(LPOPENFILENAME);

/* "If the buffer is too small, the function returns FALSE and the CommDlgExtendedError function returns FNERR_BUFFERTOOSMALL. In this case, the first two bytes of the lpstrFile buffer contain the required size, in bytes or characters." - so how do we re-get the filename? TODO */
#define ourBufSize 4096 + 2		/* for two null terminators */
static TCHAR filenames[ourBufSize];

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
		case 'h':		/* -help */
			usageExit = 0;
			goto usage;
		case '?':
			/* getopt_long_only() should have printed something since we did not set opterr to 0 */
			goto usage;
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
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = filenames;
		filenames[0] = '\0';
	ofn.nMaxFile = ourBufSize;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
//	ofn.Flags = 0;
	ofn.Flags = OFN_ENABLEHOOK;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
//	ofn.lpfnHook = NULL;
	ofn.lpfnHook = oldStyleHook;
	ofn.lpTemplateName = NULL;
	ofn.FlagsEx = 0;

	if ((*action)(&ofn) != 0) {
		/* TODO */
	} else {
		DWORD ret;

		ret = CommDlgExtendedError();
		if (ret == 0)
			printf("user aborted selection\n");
		else {
			printf("error in dialog box (return %d)\n", ret);
			return 1;
		}
	}

	return 0;
}
