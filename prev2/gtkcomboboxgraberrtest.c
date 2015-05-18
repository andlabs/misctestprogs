// 5 june 2014
// scratch GTK+ program by pietro gagliardi 16-17 april 2014
// updated 27 may 2014
// uses code from 31 march 2014 and 15 april 2014
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_32
#define GLIB_VERSION_MAX_ALLOWED GLIB_VERSION_2_32
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_4
#define GDK_VERSION_MAX_ALLOWED GDK_VERSION_3_4
#include <gtk/gtk.h>

// flag variables here; use types gbooleean, gchar *, gint, gdouble

#define flagBool(name, help) { #name, 0, 0, G_OPTION_ARG_NONE, &name, help, NULL }
#define flagString(name, help) { #name, 0, 0, G_OPTION_ARG_STRING, &name, help, NULL }
#define flagInt(name, help) { #name, 0, 0, G_OPTION_ARG_INT, &name, help, NULL }
#define flagDouble(name, help) { #name, 0, 0, G_OPTION_ARG_DOUBLE, &name, help, NULL }
static GOptionEntry flags[] = {
	// options here
	{ NULL, 0, 0, 0, NULL, NULL, NULL },
};

void init(int *argc, char *(*argv[]));

char *args = "";		// other command-line arguments here, if any

gboolean parseArgs(int argc, char *argv[])
{
	// parse arguments here, if any
	// return TRUE if parsed successfully; FALSE otherwise
	// (returning FALSE will show usage and quit)
	return TRUE;
}

GtkWidget *buildUI(void)
{
	GtkGrid *grid;
	GtkWidget *prev = NULL;		// for the ADD() macro

	grid = (GtkGrid *) gtk_grid_new();

#define EXPAND(widget, h, v) gtk_widget_set_hexpand((GtkWidget *) (widget), (h)); gtk_widget_set_vexpand((GtkWidget *) (widget), (v))
#define ALIGN(widget, h, v) gtk_widget_set_halign((GtkWidget *) (widget), GTK_ALIGN_ ## h); gtk_widget_set_valign((GtkWidget *) (widget), GTK_ALIGN_ ## v)
#define ADD(widget, side, xspan, yspan) gtk_grid_attach_next_to(grid, (GtkWidget *) widget, prev, GTK_POS_ ## side, (xspan), (yspan)); prev = (GtkWidget *) (widget)
#define ADDNEXTTO(widget, nextto, side, xspan, yspan) gtk_grid_attach_next_to(grid, (GtkWidget *) widget, (GtkWidget *) (nextto), GTK_POS_ ## side, (xspan), (yspan))
	// call EXPAND(), ALIGN(FILL/START/END/CENTER), and ADD(LEFT/TOP/RIGHT/BOTTOM)/ADDNEXTTO() for each widget

	GtkComboBoxText *cb = (GtkComboBoxText *) gtk_combo_box_text_new();
	gtk_combo_box_text_append_text(cb, "1");
	gtk_combo_box_text_append_text(cb, "2");
	gtk_combo_box_text_append_text(cb, "3");
	EXPAND(cb, TRUE, FALSE);
	ALIGN(cb, FILL, FILL);
	ADD(cb, TOP, 1, 1);

	return (GtkWidget *) grid;
}

int main(int argc, char *argv[])
{
	GtkWindow *mainwin;
	GtkWidget *layout;

	init(&argc, &argv);

	mainwin = (GtkWindow *) gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(mainwin, "Main Window");
	gtk_window_resize(mainwin, 320, 240);		// give it a useful initial size, rather than "as small as possible"
	g_signal_connect(mainwin, "delete-event", gtk_main_quit, NULL);

	layout = buildUI();
	gtk_container_add((GtkContainer *) mainwin, layout);
	gtk_widget_show_all((GtkWidget *) mainwin);

	gtk_main();
	return 0;
}

void init(int *argc, char *(*argv[]))
{
	GError *flagserr = NULL;
	GOptionContext *flagscontext;

	flagscontext = g_option_context_new(args);
	g_option_context_add_main_entries(flagscontext, flags, NULL);
	// the next line also initializes GTK+
	g_option_context_add_group(flagscontext, gtk_get_option_group(TRUE));
	if (g_option_context_parse(flagscontext, argc, argv, &flagserr) != TRUE) {
		fprintf(stderr, "error: %s\n", flagserr->message);
		goto usage;
	}

	if (parseArgs(*argc, *argv) == TRUE)
		return;
	// otherwise fall through

usage:
	fprintf(stderr, "%s\n", g_option_context_get_help(flagscontext, FALSE, NULL));
	exit(1);
}
