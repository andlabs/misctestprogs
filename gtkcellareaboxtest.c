// 18 june 2016
#include <gtk/gtk.h>

int main(void)
{
	GtkWidget *mainwin, *sv, *tv;
	GtkListStore *store;
	GtkTreeIter iter;
	GdkRGBA c1, c2, c3, c4;
	GtkCellAreaBox *area;
	GtkCellRenderer *r;
	GtkTreeViewColumn *col;

	gtk_init(NULL, NULL);

	store = gtk_list_store_new(8,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING,
		GDK_TYPE_RGBA,
		GDK_TYPE_RGBA,
		GDK_TYPE_RGBA,
		GDK_TYPE_RGBA);
	gtk_list_store_append(store, &iter);
	c1 = (GdkRGBA){ 1.0, 0.25, 0.0, 1.0 };
	c2 = (GdkRGBA){ 0.0, 0.25, 1.0, 1.0 };
	c3 = (GdkRGBA){ 1.0, 0.0, 0.25, 1.0 };
	c4 = (GdkRGBA){ 0.25, 1.0, 0.0, 1.0 };
	gtk_list_store_set(store, &iter,
		0, "Test",
		1, "Longer string",
		2, "Keep going",
		3, "Separate column",
		4, &c1, 5, &c2, 6, &c3, 7, &c4,
		-1);

	area = GTK_CELL_AREA_BOX(gtk_cell_area_box_new());
	r = gtk_cell_renderer_text_new();
	gtk_cell_area_box_pack_start(area, r, TRUE, TRUE, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(area), r, "text", 0);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(area), r, "background-rgba", 4);
	r = gtk_cell_renderer_text_new();
	gtk_cell_area_box_pack_start(area, r, FALSE, TRUE, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(area), r, "text", 1);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(area), r, "background-rgba", 5);

	tv = gtk_tree_view_new();
	gtk_tree_view_set_model(GTK_TREE_VIEW(tv), GTK_TREE_MODEL(store));

	col = gtk_tree_view_column_new_with_area(GTK_CELL_AREA(area));
	gtk_tree_view_column_set_title(col, "First Column");
	gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);

	r = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes("Second Column", r,
		"text", 2,
		"background-rgba", 6,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);

	r = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes("Third Column", r,
		"text", 3,
		"background-rgba", 7,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col);

	sv = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(sv), tv);

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(mainwin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_container_add(GTK_CONTAINER(mainwin), sv);

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
