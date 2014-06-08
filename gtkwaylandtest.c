// 8 june 2014
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

GtkWidget *layout;
GtkWidget *top, *bottom, *left, *right;

GtkWidget *buildUI(void)
{
	GtkWidget *w;
	gint xw, xh;

	layout = gtk_layout_new(NULL, NULL);

#define ADD() \
	gtk_container_add(GTK_CONTAINER(layout), w); \
	gtk_widget_get_preferred_width(w, NULL, &xw); \
	gtk_widget_get_preferred_height(w, NULL, &xh); \
	gtk_widget_set_size_request(w, xw, xh);

	w = gtk_label_new("TOP");
	ADD();
	top = w;

	w = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(w), "BOTTOM");
	ADD();
	bottom = w;

	w = gtk_button_new_with_label("LEFT");
	ADD();
	left = w;

	w = gtk_check_button_new_with_label("RIGHT");
	ADD();
	right = w;

	return (GtkWidget *) layout;
}

gboolean resize(GtkWidget *w, GdkEvent *event, gpointer data)
{
	GtkLayout *l = (GtkLayout *) layout;
	gint width, height;
	gint sw, sh;
	GtkAllocation s;

#define SR(widget) gtk_widget_get_allocation((widget), &s); sw = s.width; sh = s.height
	gtk_window_get_size(GTK_WINDOW(w), &width, &height);
	SR(top);
#define MOVE gtk_layout_move
	MOVE(l, top, (width - sw) / 2, 0);
	SR(left);
	MOVE(l, left, 0, (height - sh) / 2);
	SR(bottom);
	MOVE(l, bottom, (width - sw) / 2, height - sh);
	SR(right);
	MOVE(l, right, width - sw, (height - sh) / 2);
	return FALSE;
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
	g_signal_connect(mainwin, "configure-event", G_CALLBACK(resize), NULL);

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
