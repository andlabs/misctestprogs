// 9 june 2016
#include <gtk/gtk.h>

GtkWidget *hideAll;
GtkWidget *hideThis, *this, *showThis;
GtkWidget *showAll;

void clicked(GtkButton *b, gpointer data)
{
	if (GTK_WIDGET(b) == hideAll) {
		gtk_widget_hide(hideThis);
		gtk_widget_hide(this);
		gtk_widget_hide(showThis);
	}
	if (GTK_WIDGET(b) == showAll) {
		gtk_widget_show(hideThis);
		gtk_widget_show(this);
		gtk_widget_show(showThis);
	}
	if (GTK_WIDGET(b) == hideThis)
		gtk_widget_hide(this);
	if (GTK_WIDGET(b) == showThis)
		gtk_widget_show(this);
	if (GTK_WIDGET(b) == this) {
		GtkAlign align;

		align = gtk_widget_get_halign(this);
		align++;
		if (align >= 5)
			align = 0;
		gtk_widget_set_halign(this, align);
	}
}

int main(void)
{
	GtkWidget *w;
	GtkGrid *grid;

	gtk_init(NULL, NULL);
	w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(w, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	grid = GTK_GRID(gtk_grid_new());
	gtk_container_add(GTK_CONTAINER(w), GTK_WIDGET(grid));

	this = gtk_button_new_with_label("This");
	gtk_widget_set_hexpand(this, FALSE);
	gtk_widget_set_halign(this, 0);
	g_signal_connect(this, "clicked", G_CALLBACK(clicked), NULL);
	gtk_grid_attach(grid, this,
		0, 0,
		1, 1);

	hideAll = gtk_button_new_with_label("Hide All");
//	gtk_widget_set_hexpand(hideAll, TRUE);
//	gtk_widget_set_vexpand(hideAll, TRUE);
	g_signal_connect(hideAll, "clicked", G_CALLBACK(clicked), NULL);
	gtk_grid_attach_next_to(grid,
		hideAll, this, GTK_POS_TOP,
		1, 1);

	showAll = gtk_button_new_with_label("Show All");
//	gtk_widget_set_hexpand(showAll, TRUE);
//	gtk_widget_set_vexpand(showAll, TRUE);
	g_signal_connect(showAll, "clicked", G_CALLBACK(clicked), NULL);
	gtk_grid_attach_next_to(grid,
		showAll, this, GTK_POS_BOTTOM,
		1, 1);

	hideThis = gtk_button_new_with_label("Hide This");
	gtk_widget_set_hexpand(hideThis, TRUE);
	gtk_widget_set_vexpand(hideThis, TRUE);
	g_signal_connect(hideThis, "clicked", G_CALLBACK(clicked), NULL);
	gtk_grid_attach_next_to(grid,
		hideThis, this, GTK_POS_LEFT,
		1, 1);

	showThis = gtk_button_new_with_label("Show This");
	gtk_widget_set_hexpand(showThis, TRUE);
	gtk_widget_set_vexpand(showThis, TRUE);
	g_signal_connect(showThis, "clicked", G_CALLBACK(clicked), NULL);
	gtk_grid_attach_next_to(grid,
		showThis, this, GTK_POS_RIGHT,
		1, 1);

	gtk_widget_show_all(w);
	gtk_main();
	return 0;
}
