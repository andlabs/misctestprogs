/* 31 march 2014 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

static gboolean nonlocal = FALSE;

#define flagBool(name, help) { #name, 0, 0, G_OPTION_ARG_NONE, &name, help, NULL }
static GOptionEntry flags[] = {
	flagBool(nonlocal, "allow nonlocal files"),
	{ NULL },
};

static GtkFileChooserAction action;

void init(int *argc, char *(*argv[]))
{
	GError *flagserr = NULL;
	GOptionContext *flagscontext;
	char *mode;

	flagscontext = g_option_context_new("{open|save|opendir|savedir} - test GTK+ open/save dialogs");
	g_option_context_add_main_entries(flagscontext, flags, NULL);
	/* the next line also initializes GTK+ */
	g_option_context_add_group(flagscontext, gtk_get_option_group(TRUE));
	if (g_option_context_parse(flagscontext, argc, argv, &flagserr) != TRUE) {
		fprintf(stderr, "error: %s\n", flagserr->message);
		goto usage;
	}

	if (*argc != 2)
		goto usage;
	mode = (*argv)[1];
	if (strcmp(mode, "open") == 0)
		action = GTK_FILE_CHOOSER_ACTION_OPEN;
	else if (strcmp(mode, "save") == 0)
		action = GTK_FILE_CHOOSER_ACTION_SAVE;
	else if (strcmp(mode, "opendir") == 0)
		action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	else if (strcmp(mode, "savedir") == 0)
		action = GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER;
	else {
		fprintf(stderr, "error: unknown mode %s\n", mode);
		goto usage;
	}

	return;

usage:
	fprintf(stderr, "%s\n", g_option_context_get_help(flagscontext, FALSE, NULL));
	exit(1);
}

int main(int argc, char *argv[])
{
	init(&argc, &argv);
	return 0;
}
