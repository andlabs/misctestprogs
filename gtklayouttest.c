/* pietro gagliardi - 7-8 april 2014 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_4
#define GDK_VERSION_MAX_ALLOWED GDK_VERSION_3_4
#include <gtk/gtk.h>

gboolean windowonly = FALSE;
gboolean drawingarea = FALSE;
gboolean viewport = FALSE;
gboolean drawsig = FALSE;
gboolean binwin = FALSE;

void parseargs(int argc, char *argv[])
{
	if (argc != 2 && argc != 3)
		goto usage;

#define extra(str) (argc == 3 && strcmp(argv[2], str) == 0)
#define noextra(cond) if (!(cond) && argc == 3) goto usage;
	if (strcmp(argv[1], "windowOnly") == 0) {
		noextra(FALSE);
		windowonly = TRUE;
		return;
	} else if (strcmp(argv[1], "drawingArea") == 0) {
		drawingarea = TRUE;
		viewport = extra("viewport");
		noextra(viewport);
		return;
	} else if (strcmp(argv[1], "layout") == 0) {
		binwin = extra("drawbin");
		drawsig = binwin || extra("draw");
		noextra(drawsig);
		return;
	}

usage:
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "\t%s windowOnly\n", argv[0]);
	fprintf(stderr, "\t%s drawingArea [viewport]\n", argv[0]);
	fprintf(stderr, "\t%s layout [draw|drawbin]\n", argv[0]);
	exit(1);
}

gboolean draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	double x, y, w, h;

	if (binwin)
		cr = gdk_cairo_create(gtk_layout_get_bin_window((GtkLayout *) widget));

	cairo_clip_extents(cr, &x, &y, &w, &h);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_rgba(cr, 0, 0, 0, 0);
	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);

	if (binwin)
		cairo_destroy(cr);

	return FALSE;
}

int main(int argc, char *argv[])
{
	GtkWidget *w;
	GtkWidget *q;

	parseargs(argc, argv);
	gtk_init(NULL, NULL);

	w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(w, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	if (!windowonly) {
		if (drawingarea) {
			q = gtk_drawing_area_new();
			if (viewport) {
				GtkWidget *sw;

				sw = gtk_scrolled_window_new(NULL, NULL);
				gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), q);
				q = sw;
			}
		} else {
			q = gtk_layout_new(NULL, NULL);
			if (drawsig)
				g_signal_connect(q, "draw", G_CALLBACK(draw), NULL);
		}
		gtk_container_add(GTK_CONTAINER(w), q);
	}

	gtk_widget_show_all(w);

	gtk_main();
	return 0;
}
