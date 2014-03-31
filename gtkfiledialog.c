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
static gboolean multisel = FALSE;
static gboolean showhidden = FALSE;
static gboolean confirm = FALSE;
static gboolean mkdir = FALSE;
static gboolean filter = FALSE;

#define flagBool(name, help) { #name, 0, 0, G_OPTION_ARG_NONE, &name, help, NULL }
static GOptionEntry flags[] = {
	flagBool(nonlocal, "allow nonlocal files"),
	flagBool(multisel, "allow multiple selection"),
	flagBool(showhidden, "show hidden files"),
	flagBool(confirm, "confirm on overwrite"),
	flagBool(mkdir, "provide new folder button"),
	flagBool(filter, "apply some test filters"),
	{ NULL },
};

static GtkFileChooserAction action;

char *slistToString(GSList *list)
{
	GString *str;

	if (list == NULL)
		return "[]";
	str = g_string_new("[");
	str = g_string_append(str, (gchar *) list->data);
	list = list->next;
	while (list != NULL) {
		str = g_string_append(str, "\n\t");
		str = g_string_append(str, (gchar *) list->data);
		list = list->next;
	}
	str = g_string_append(str, "]");
	return (char *) str->str;
}

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
	gtk_file_chooser_set_select_multiple(chooser, multisel);
	/* TODO seems to have no effect? */
	gtk_file_chooser_set_show_hidden(chooser, showhidden);
	gtk_file_chooser_set_do_overwrite_confirmation(chooser, confirm);
	gtk_file_chooser_set_create_folders(chooser, mkdir);

	if (filter) {
		GtkFileFilter *filter;

		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "C files");
		gtk_file_filter_add_pattern(filter, "*.c");
		gtk_file_chooser_add_filter(chooser, filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "Name only");
		gtk_file_chooser_add_filter(chooser, filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern(filter, "*.patternonly");
		gtk_file_chooser_add_filter(chooser, filter);
		gtk_file_chooser_add_filter(chooser, gtk_file_filter_new());
		/* this is how to make an All Files filter according to the docs */
		filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern(filter, "*");
		gtk_file_filter_set_name(filter, "All files");		/* but it doesn't have a name? TODO */
		gtk_file_chooser_add_filter(chooser, filter);
	}

	response = gtk_dialog_run(dialog);
	if (response == GTK_RESPONSE_ACCEPT)
		printf("user selection made\nfilename: %s\nURI: %s\nfilenames: %s\nURIs: %s\n",
			gtk_file_chooser_get_filename(chooser),
			gtk_file_chooser_get_uri(chooser),
			slistToString(gtk_file_chooser_get_filenames(chooser)),
			slistToString(gtk_file_chooser_get_uris(chooser)));
	else
		printf("user aborted selection (return: %d)\n", response);

	return 0;
}
