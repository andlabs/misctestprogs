// 30 january 2015
#include <gtk/gtk.h>

int main(void)
{
	GtkWidget *window, *table;
	GtkListStore *list;
	GtkTreeIter iter;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;

	gtk_init(NULL, NULL);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	table = gtk_tree_view_new();
	list = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(table), GTK_TREE_MODEL(list));
	gtk_list_store_append(list, &iter);
	gtk_list_store_set(list, &iter, 0, "First Row", -1);
	gtk_list_store_append(list, &iter);
	gtk_list_store_set(list, &iter, 0, "Second Row", -1);

	// the magic
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer,
		"font", "Zapf Chancery 18",
		"weight", PANGO_WEIGHT_BOLD,
		NULL);

	col = gtk_tree_view_column_new_with_attributes("Rows", renderer,
		"text", 0,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(table), col);

	gtk_container_add(GTK_CONTAINER(window), table);
	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}
