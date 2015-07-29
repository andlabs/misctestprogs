// 28 july 2015
// from gtkboxexpand2.c 15 july 2015
// from gtkboxexpand.c 14 july 2015
#include <gtk/gtk.h>

GtkWidget *b3;

static gboolean drawMidline(GtkWidget *w, cairo_t *cr, gpointer data)
{
	GtkAllocation a;
	GtkAllocation a3;
	double aa3, aa32;

	gtk_widget_get_allocation(w, &a);
	gtk_widget_get_allocation(b3, &a3);
	aa3 = a.width - a3.width;
	aa32 = aa3 / 2;
	// for cairo
	// don't add another 0.5 if aa3 was already odd
	if (((a.width - a3.width) % 2) == 0)
		aa32 += 0.5;
	cairo_set_line_width(cr, 1);
	cairo_move_to(cr, aa32, 0);
	cairo_line_to(cr, aa32, a.height);
	cairo_stroke(cr);
	cairo_set_line_width(cr, 1);
	cairo_move_to(cr, aa3 + 0.5, 0);
	cairo_line_to(cr, aa3 + 0.5, a.height);
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

	b3 = gtk_button_new_with_label("3");
	gtk_widget_set_hexpand(b3, FALSE);
	gtk_container_add(GTK_CONTAINER(hbox), b3);

	GtkSizeGroup *sg;

	sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget(sg, b1);
	gtk_size_group_add_widget(sg, b2);

	g_signal_connect(e1, "changed", G_CALLBACK(changed), b1);
	g_signal_connect(e2, "changed", G_CALLBACK(changed), b2);

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
