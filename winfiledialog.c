/* 31 march 2014 */
/* for mingw; link with -lcomdlg32 */
/* targets windows 2000 or newer */
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <strings.h>
#include <getopt.h>

static BOOL WINAPI (*action)(LPOPENFILENAME);

/* "If the buffer is too small, the function returns FALSE and the CommDlgExtendedError function returns FNERR_BUFFERTOOSMALL. In this case, the first two bytes of the lpstrFile buffer contain the required size, in bytes or characters." - so how do we re-get the filename? TODO */
#define ourBufSize 4096 + 2		/* for two null terminators */
static TCHAR filenames[ourBufSize];

/*
void init(int *argc, char *(*argv[]))
{
	char *mode;

	if (*argc != 2)
		goto usage;
	mode = (*argv)[1];
	if (strcmp(mode, "open") == 0)
		action = GetOpenFileName;
	else if (strcmp(mode, "save") == 0)
		action = GetSaveFileName;
	else if (strcmp(mode, "opendir") == 0) {
		action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
		button = GTK_STOCK_OPEN;
	} else if (strcmp(mode, "savedir") == 0) {
		action = GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER;
		button = GTK_STOCK_SAVE;
	} else {
		fprintf(stderr, "error: unknown mode %s\n", mode);
		goto usage;
	}

	return;

usage:
	fprintf(stderr, "%s\n", g_option_context_get_help(flagscontext, FALSE, NULL));
	exit(1);
}
*/

int main(int argc, char *argv[])
{
	OPENFILENAME ofn;

//	init(&argc, &argv);
	action=GetOpenFileName;

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
	ofn.Flags = 0;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	ofn.FlagsEx = 0;

	printf("%d\n", (*action)(&ofn));
	return 0;
}
