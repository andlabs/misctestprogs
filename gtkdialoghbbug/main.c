// 2 april 2015
#include <gtk/gtk.h>

// #qo pkg-config: gtk+-3.0

void clicked(GtkButton *b, gpointer data)
{
	GtkWindow *parent = GTK_WINDOW(data);
	GtkBuilder *builder;
	GtkWidget *dialog;

	builder = gtk_builder_new_from_file("main.ui");
	dialog = GTK_WIDGET(gtk_builder_get_object(builder, "dialog"));
	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	g_object_unref(builder);
}

void destroy(GtkWidget *w, gpointer data)
{
	gtk_main_quit();
}

int main(void)
{
	GtkWidget *mainwin;
	GtkWidget *button;

	gtk_init(NULL, NULL);
	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_resize(GTK_WINDOW(mainwin), 200, 200);
	g_signal_connect(mainwin, "destroy", G_CALLBACK(destroy), NULL);
	button = gtk_button_new_with_label("Click");
	g_signal_connect(button, "clicked", G_CALLBACK(clicked), mainwin);
	gtk_container_add(GTK_CONTAINER(mainwin), button);
	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
