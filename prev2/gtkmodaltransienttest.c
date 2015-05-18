// 4 june 2014
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

void showmsgbox(GtkButton *button, gpointer data)
{
	GtkMessageDialog *md;
	GtkWindow *transient = (GtkWindow *) data;
	GtkWindowGroup *prev, *new;

	if (transient != NULL) {
		prev = NULL;
		// removing a window from the default group throws up a Gtk-CRITICAL
		if (gtk_window_has_group(transient)) {
			prev = gtk_window_get_group(transient);
			gtk_window_group_remove_window(prev, transient);
		}
		new = gtk_window_group_new();
		gtk_window_group_add_window(new, transient);
	}
	md = (GtkMessageDialog *) gtk_message_dialog_new(
		transient,
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_OTHER,
		GTK_BUTTONS_OK,
		"This is a message box");
	gtk_message_dialog_format_secondary_text(md, "You should see it with the modality and transience mode that you selected.");
	gtk_dialog_run((GtkDialog *) md);
	gtk_widget_destroy((GtkWidget *) md);
	if (transient != NULL) {
		gtk_window_group_remove_window(new, transient);
		g_object_unref(new);
		if (prev != NULL)
			gtk_window_group_add_window(prev, transient);
	}
}

GtkWindow *mainwin;

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

	GtkWidget *toy;
#define ADDTOY(w, p) toy = (w); EXPAND(toy, FALSE, TRUE); ALIGN(toy, FILL, FILL); ADD(toy, p, 1, 1);

	GtkWindow *otherwin;

	otherwin = (GtkWindow *) gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(otherwin, "Other Window");
	g_signal_connect(otherwin, "delete-event", gtk_main_quit, NULL);
	ADDTOY(gtk_lock_button_new(g_simple_permission_new(TRUE)), RIGHT);
	ADDTOY(gtk_spinner_new(), RIGHT);
	ADDTOY(gtk_font_button_new(), RIGHT);
	ADDTOY(gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), BOTTOM);
	ADDTOY(gtk_toggle_button_new_with_label("Toggler"), LEFT);
	ADDTOY(gtk_switch_new(), LEFT);
	gtk_container_add((GtkContainer *) otherwin, (GtkWidget *) grid);
	gtk_widget_show_all((GtkWidget *) otherwin);

	grid = (GtkGrid *) gtk_grid_new();
	prev = NULL;

	GtkWidget *button;

	button = gtk_button_new_with_label("Application-Modal");
	g_signal_connect(button, "clicked", G_CALLBACK(showmsgbox), NULL);
	EXPAND(button, TRUE, FALSE);
	ALIGN(button, FILL, FILL);
	ADD(button, BOTTOM, 3, 1);
	button = gtk_button_new_with_label("Modal+Transient to This Window");
	g_signal_connect(button, "clicked", G_CALLBACK(showmsgbox), mainwin);
	EXPAND(button, TRUE, FALSE);
	ALIGN(button, FILL, FILL);
	ADD(button, BOTTOM, 3, 1);
	button = gtk_button_new_with_label("Modal+Transient to the Other Window");
	g_signal_connect(button, "clicked", G_CALLBACK(showmsgbox), otherwin);
	EXPAND(button, TRUE, FALSE);
	ALIGN(button, FILL, FILL);
	ADD(button, BOTTOM, 3, 1);

	ADDTOY(gtk_entry_new(), BOTTOM);
	ADDTOY(gtk_color_button_new(), RIGHT);
	ADDTOY(gtk_check_button_new_with_label("Check"), RIGHT);

	return (GtkWidget *) grid;
}

int main(int argc, char *argv[])
{
	GtkWidget *layout;

	init(&argc, &argv);

	mainwin = (GtkWindow *) gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(mainwin, "Main Window");
//	gtk_window_resize(mainwin, 320, 240);		// give it a useful initial size, rather than "as small as possible"
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
