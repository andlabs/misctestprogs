/* 7 april 2014 */
#include <gtk/gtk.h>

gboolean draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	double x, y, w, h;

	/* note: even if I get the bin_window's cairo_t it still doesn't work */
	
	cairo_clip_extents(cr, &x, &y, &w, &h);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_rgba(cr, 0, 0, 0, 0);
	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);
	return FALSE;
}

int main(void)
{
	GtkWindow *w;
	void *q;

	gtk_init(NULL, NULL);

	w = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	q = gtk_layout_new(NULL, NULL);
	g_signal_connect(q, "draw", draw, NULL);
	gtk_container_add(w, q);

	gtk_widget_show_all(w);

	gtk_main();
	return 0;
}


