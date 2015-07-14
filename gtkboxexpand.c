// 14 july 2015
#include <gtk/gtk.h>

static gboolean drawMidline(GtkWidget *w, cairo_t *cr, gpointer data)
{
	GtkAllocation a;

	gtk_widget_get_allocation(w, &a);
	cairo_move_to(cr, a.width / 2, 0);
	cairo_line_to(cr, a.width / 2, a.height);
	cairo_stroke(cr);
	return GDK_EVENT_PROPAGATE;
}

static void changed(GtkEditable *editable, gpointer data)
{
	gtk_button_set_label(GTK_BUTTON(data), gtk_entry_get_text(GTK_ENTRY(editable)));
	gtk_widget_queue_resize(GTK_WIDGET(data));
}

int main(void)
{
	GtkWidget *mainwin;
	GtkWidget *vbox;
	GtkWidget *e1, *e2;
	GtkWidget *da;
	GtkWidget *hbox;
	GtkWidget *b1, *b2;

	gtk_init(NULL, NULL);

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(mainwin, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(mainwin), vbox);

	e1 = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(vbox), e1);

	e2 = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(vbox), e2);

	da = gtk_drawing_area_new();
	gtk_widget_set_size_request(da, -1, 30);
	g_signal_connect(da, "draw", G_CALLBACK(drawMidline), NULL);
	gtk_widget_set_vexpand(da, TRUE);
	gtk_widget_set_valign(da, GTK_ALIGN_FILL);
	gtk_container_add(GTK_CONTAINER(vbox), da);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add(GTK_CONTAINER(vbox), hbox);

	b1 = gtk_button_new_with_label("Button 1");
	gtk_widget_set_hexpand(b1, TRUE);
	gtk_widget_set_halign(b1, GTK_ALIGN_FILL);
	gtk_container_add(GTK_CONTAINER(hbox), b1);

	b2 = gtk_button_new_with_label("Button 2");
	gtk_widget_set_hexpand(b2, TRUE);
	gtk_widget_set_halign(b2, GTK_ALIGN_FILL);
	gtk_container_add(GTK_CONTAINER(hbox), b2);

	g_signal_connect(e1, "changed", G_CALLBACK(changed), b1);
	g_signal_connect(e2, "changed", G_CALLBACK(changed), b2);

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
