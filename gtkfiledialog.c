/* 31 march 2014 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* only care about versions of GLib/GTK+ I'm targeting */
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_32
#define GLIB_VERSION_MAX_ALLOWED GLIB_VERSION_2_32
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_4
#define GDK_VERSION_MAX_ALLOWED GDK_VERSION_3_4
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
	GtkFileChooserDialog *chooserdialog;
	GtkFileChooser *chooser;
	GtkDialog *dialog;
	gint response;

	init(&argc, &argv);

	chooserdialog = (GtkFileChooserDialog *) gtk_file_chooser_dialog_new(
		"Dialog Test", NULL, action,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	/* give proper polymorphic types to avoid compiler warnings */
	chooser = (GtkFileChooser *) chooserdialog;
	dialog = (GtkDialog *) chooserdialog;

	gtk_file_chooser_set_local_only(chooser, !nonlocal);

	response = gtk_dialog_run(dialog);
	if (response == GTK_RESPONSE_ACCEPT) {
		printf("user selection made\nfilename: %s\nURI: %s\n",
			gtk_file_chooser_get_filename(chooser),
			gtk_file_chooser_get_uri(chooser));
	} else
		printf("user aborted selection (return: %d)\n", response);

	return 0;
}
