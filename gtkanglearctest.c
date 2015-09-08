// 8 september 2015
#define _GNU_SOURCE
#include <math.h>
#include <gtk/gtk.h>

static gboolean ondraw(GtkWidget *w, cairo_t *cr, gpointer data)
{
	cairo_set_source_rgb(cr, 0, 0, 0);

	cairo_new_path(cr);
	cairo_move_to(cr, 100.5, 100.5);
	cairo_arc_negative(cr, 100.5, 100.5, 50, 330 * (M_PI / 180.0), 30 * (M_PI / 180.0));
	cairo_line_to(cr, 100.5, 100.5);
	cairo_set_line_width(cr, 0.5);
	cairo_stroke(cr);

	cairo_new_path(cr);
	cairo_arc_negative(cr, 210.5, 100.5, 50, 330 * (M_PI / 180.0), 30 * (M_PI / 180.0));
	cairo_close_path(cr);
	cairo_set_line_width(cr, 0.5);
	cairo_stroke(cr);

	return FALSE;
}

int main(void)
{
	GtkWidget *mainwin;

	gtk_init(NULL, NULL);

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(mainwin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect_after(mainwin, "draw", G_CALLBACK(ondraw), NULL);

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
