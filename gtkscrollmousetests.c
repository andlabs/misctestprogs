// 16 december 2015
#include <gtk/gtk.h>

static gboolean areaWidget_motion_notify_event(GtkWidget *w, GdkEventMotion *e, gpointer data)
{
	char *msg;

	msg = g_strdup_printf("X %g Y %g", e->x, e->y);
	gtk_button_set_label(GTK_BUTTON(data), msg);
	g_free(msg);
	return GDK_EVENT_PROPAGATE;
}

gboolean areaWidget_enter_notify_event(GtkWidget *w, GdkEventCrossing *e, gpointer data)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data), TRUE);
	return GDK_EVENT_PROPAGATE;
}

gboolean areaWidget_leave_notify_event(GtkWidget *w, GdkEventCrossing *e, gpointer data)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data), FALSE);
	return GDK_EVENT_PROPAGATE;
}

int main(void)
{
	GtkWidget *mainwin;
	GtkWidget *box;
	GtkWidget *sw;
	GtkWidget *area;
	GtkWidget *label;

	gtk_init(NULL, NULL);

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_resize(GTK_WINDOW(mainwin), 320, 240);
	g_signal_connect(mainwin, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(mainwin), box);

	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_hexpand(sw, TRUE);
	gtk_widget_set_halign(sw, GTK_ALIGN_FILL);
	gtk_widget_set_vexpand(sw, TRUE);
	gtk_widget_set_valign(sw, GTK_ALIGN_FILL);
	gtk_container_add(GTK_CONTAINER(box), sw);

	label = gtk_toggle_button_new_with_label("");
	gtk_widget_set_hexpand(label, TRUE);
	gtk_widget_set_halign(label, GTK_ALIGN_FILL);
	gtk_container_add(GTK_CONTAINER(box), label);

	area = gtk_drawing_area_new();
	gtk_widget_set_size_request(area, 5000, 5000);
	gtk_container_add(GTK_CONTAINER(sw), area);

	gtk_widget_add_events(area,
		GDK_POINTER_MOTION_MASK |
		GDK_BUTTON_MOTION_MASK |
		GDK_BUTTON_PRESS_MASK |		// apparently GDK_BUTTON_MOTION_MASK won't work without it
		GDK_ENTER_NOTIFY_MASK |
		GDK_LEAVE_NOTIFY_MASK);
	g_signal_connect(area, "motion-notify-event",
		G_CALLBACK(areaWidget_motion_notify_event), label);
	g_signal_connect(area, "enter-notify-event",
		G_CALLBACK(areaWidget_enter_notify_event), label);
	g_signal_connect(area, "leave-notify-event",
		G_CALLBACK(areaWidget_leave_notify_event), label);

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
