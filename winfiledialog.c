/* 31 march 2014 */
/* for mingw; link with -lcomdlg32 */
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <strings.h>
#include <getopt.h>

static BOOL WINAPI (*action)(LPOPENFILENAME);

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

	(*action)(&ofn);
	return 0;
}
