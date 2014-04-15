// 15 april 2014
#include <stdio.h>
#include <stdlib.h>
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_32
#define GLIB_VERSION_MAX_ALLOWED GLIB_VERSION_2_32
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_4
#define GDK_VERSION_MAX_ALLOWED GDK_VERSION_3_4
#include <gtk/gtk.h>

GtkWindow *win;
GtkEntry *width, *height;

void set(GtkWidget *widget, gpointer what)
{
	int wid, ht;

	wid = atoi(gtk_entry_get_text(width));
	ht = atoi(gtk_entry_get_text(height));
	gtk_window_resize(win, wid, ht);
}

void get(GtkWidget *widget, gpointer what)
{
	int wid, ht;

	gtk_window_get_size(win, &wid, &ht);
	printf("%d %d\n", wid, ht);
}

int main(void)
{
	GtkGrid *layout;
	GtkButton *bset, *bget;

	gtk_init(NULL, NULL);
	win = (GtkWindow *) gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(win, "Window Size Test");
	g_signal_connect(win, "delete-event", gtk_main_quit, NULL);

	layout = (GtkGrid *) gtk_grid_new();
#define ADD(widget, x, y, xspan) gtk_grid_attach(layout, (GtkWidget *) (widget), y, x, xspan, 1)

	width = (GtkEntry *) gtk_entry_new();
	ADD(width, 0, 0, 1);

	height = (GtkEntry *) gtk_entry_new();
	ADD(height, 0, 1, 1);

	bset = (GtkButton *) gtk_button_new();
	gtk_button_set_label(bset, "Set");
	g_signal_connect(bset, "clicked", G_CALLBACK(set), NULL);
	ADD(bset, 1, 0, 2);

	bget = (GtkButton *) gtk_button_new();
	gtk_button_set_label(bget, "Get");
	g_signal_connect(bget, "clicked", G_CALLBACK(get), NULL);
	ADD(bget, 2, 0, 2);

	gtk_container_add((GtkContainer *) win, (GtkWidget *) layout);
	gtk_widget_show_all((GtkWidget *) win);
	gtk_main();
	return 0;
}
