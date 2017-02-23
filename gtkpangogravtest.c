// 23 february 2017
#include <gtk/gtk.h>

GtkWidget *combo;
const char text[] = "english text \xE5\x92\x8C\xE6\x96\x87\xE5\x92\x8C\xE6\x96\x87 english text";

gboolean drawtext(GtkWidget *w, cairo_t *cr, gpointer data)
{
	PangoContext *context;
	PangoLayout *layout;

	context = gtk_widget_create_pango_context(w);
	pango_context_set_base_gravity(context, (PangoGravity) gtk_combo_box_get_active(GTK_COMBO_BOX(combo)));
	layout = pango_layout_new(context);
	pango_layout_set_text(layout, text, -1);
	cairo_move_to(cr, 0, 0);
	pango_cairo_show_layout(cr, layout);
	g_object_unref(layout);
	g_object_unref(context);
	return GDK_EVENT_PROPAGATE;
}

int main(void)
{
	GtkWidget *mainwin;
	GtkWidget *box;
	GtkWidget *da;

	gtk_init(NULL, NULL);

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(mainwin), 400, 400);
	g_signal_connect(mainwin, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(mainwin), box);

	combo = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, "South");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, "East");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, "North");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, "West");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, "Auto");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
	gtk_widget_set_hexpand(combo, TRUE);
	gtk_widget_set_halign(combo, GTK_ALIGN_FILL);
	gtk_container_add(GTK_CONTAINER(box), combo);

	da = gtk_drawing_area_new();
	g_signal_connect(da, "draw", G_CALLBACK(drawtext), NULL);
	g_signal_connect_swapped(combo, "changed", G_CALLBACK(gtk_widget_queue_draw), da);
	gtk_widget_set_hexpand(da, TRUE);
	gtk_widget_set_halign(da, GTK_ALIGN_FILL);
	gtk_widget_set_vexpand(da, TRUE);
	gtk_widget_set_valign(da, GTK_ALIGN_FILL);
	gtk_container_add(GTK_CONTAINER(box), da);

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
