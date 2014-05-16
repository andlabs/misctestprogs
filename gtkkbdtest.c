// 16 may 2014
// scratch GTK+ program by pietro gagliardi 16-17 april 2014
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

GtkDrawingArea *area;
GtkTextView *report;

void appendreport(char *fmt, ...)
{
	va_list arg;
	GtkTextBuffer *buf;
	GtkTextIter iter;
	GtkTextMark *mark;
	char *text;

	va_start(arg, fmt);
	text = g_strdup_vprintf(fmt, arg);
	if (text == NULL) {
		fprintf(stderr, "error allocating memory for format string \"%s\"\n", fmt);
		abort();
	}
	buf = gtk_text_view_get_buffer(report);
	gtk_text_buffer_get_end_iter(buf, &iter);
	gtk_text_buffer_insert(buf, &iter, text, -1);
	gtk_text_buffer_get_end_iter(buf, &iter);		// new end
	gtk_text_buffer_move_mark_by_name(buf, "insert", &iter);
	mark = gtk_text_buffer_get_mark(buf, "insert");
	gtk_text_view_scroll_to_mark(report, mark, 0, TRUE, 0.0, 1.0);	// scroll to end
	g_free(text);
	va_end(arg);
}

#define ev(n) \
	gboolean n(GtkWidget *w, GdkEvent *_e, gpointer unused) \
	{ \
		GdkEventKey *e = (GdkEventKey *) _e; \
		appendreport("%s: %x %x\n", #n , e->hardware_keycode, e->keyval); \
		return FALSE; \
	}
ev(press)
ev(release)

gboolean draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	gint w, h;
	cairo_pattern_t *gradient;
	double startx = 0, starty = 0;
	double endx, endy;

	gtk_widget_get_size_request(widget, &w, &h);
	endx = w;
	endy = h;
	cairo_device_to_user(cr, &startx, &starty);
	cairo_device_to_user(cr, &endx, &endy);
	gradient = cairo_pattern_create_linear(startx, starty, startx, endy);		// vertical gradient, so x1 == x0
	cairo_pattern_add_color_stop_rgb(gradient, 0.0, 1.0, 0.0, 0.0);
	cairo_pattern_add_color_stop_rgb(gradient, 0.5, 0.0, 1.0, 0.0);
	cairo_pattern_add_color_stop_rgb(gradient, 1.0, 0.0, 0.0, 1.0);
	cairo_set_source(cr, gradient);
	cairo_rectangle(cr, 0, 0, w, h);
	cairo_fill(cr);
	cairo_pattern_destroy(gradient);
	return FALSE;
}

gboolean resize(GtkWidget *w, GdkEvent *_e, gpointer data)
{
	GdkEventConfigure *e = (GdkEventConfigure *) _e;

	gtk_widget_set_size_request(w, e->width, e->height);
	return FALSE;
}

gboolean click(GtkWidget *w, GdkEvent *e, gpointer data)
{
	gtk_widget_grab_focus(w);
	return FALSE;
}

GtkGrid *buildUI(void)
{
	GtkGrid *grid;

	grid = (GtkGrid *) gtk_grid_new();

#define ADD(widget, x, y, xspan) gtk_grid_attach(grid, (GtkWidget *) (widget), x, y, xspan, 1)
	// call ADD() for each widget
#define EXPAND(w, h, v) gtk_widget_set_hexpand((GtkWidget *) (w), h); gtk_widget_set_vexpand((GtkWidget *) w, v);
#define ALIGN(w, h, v) gtk_widget_set_halign((GtkWidget *) (w), h); gtk_widget_set_valign((GtkWidget *) w, v);

	area = (GtkDrawingArea *) gtk_drawing_area_new();
	gtk_widget_set_can_focus((GtkWidget *) area, TRUE);
	g_signal_connect(area, "key-press-event", G_CALLBACK(press), NULL);
	g_signal_connect(area, "key-release-event", G_CALLBACK(release), NULL);
	g_signal_connect(area, "draw", G_CALLBACK(draw), NULL);
	g_signal_connect(area, "configure-event", G_CALLBACK(resize), NULL);
	gtk_widget_add_events((GtkWidget *) area, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(area, "button-press-event", G_CALLBACK(click), NULL);
	EXPAND(area, TRUE, TRUE);
	ALIGN(area, GTK_ALIGN_FILL, GTK_ALIGN_FILL);
	ADD(area, 0, 0, 1);

	report = (GtkTextView *) gtk_text_view_new();
	gtk_text_view_set_editable(report, FALSE);
	gtk_text_view_set_wrap_mode(report, GTK_WRAP_WORD);
	appendreport("Click the gradient at the left and press a key...\n");
	GtkScrolledWindow *sw = (GtkScrolledWindow *) gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add((GtkContainer *) sw, (GtkWidget *) report);
	EXPAND(sw, TRUE, TRUE);
	ALIGN(sw, GTK_ALIGN_FILL, GTK_ALIGN_FILL);
	gtk_grid_attach_next_to(grid, (GtkWidget *) sw, (GtkWidget *) area, GTK_POS_RIGHT, 1, 1);

	return grid;
}

int main(int argc, char *argv[])
{
	GtkWindow *mainwin;
	GtkGrid *layout;

	init(&argc, &argv);

	mainwin = (GtkWindow *) gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(mainwin, "Main Window");
	g_signal_connect(mainwin, "delete-event", gtk_main_quit, NULL);

	layout = buildUI();
	gtk_container_add((GtkContainer *) mainwin, (GtkWidget *) layout);
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
