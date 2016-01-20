// 19 january 2016
#include <gtk/gtk.h>

int main(void)
{
	GtkWidget *mainwin;
	GtkWidget *vbox;
	GtkWidget *sizeSlider;
	GtkWidget *sw;
	GtkWidget *list;
	GtkListStore *fontStore;
	GtkTreeViewColumn *col;
	GtkCellRenderer *r;
	PangoFontMap *pcfm;
	PangoFontFamily **fams;
	int i, n;

	gtk_init(NULL, NULL);

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(mainwin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(mainwin), 12);

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
	gtk_container_add(GTK_CONTAINER(mainwin), vbox);

	sizeSlider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 4, 96, 1);
	gtk_range_set_value(GTK_RANGE(sizeSlider), 10);
	gtk_widget_set_vexpand(sizeSlider, FALSE);
	gtk_container_add(GTK_CONTAINER(vbox), sizeSlider);

	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_IN);
	gtk_widget_set_hexpand(sw, TRUE);
	gtk_widget_set_vexpand(sw, TRUE);
	gtk_container_add(GTK_CONTAINER(vbox), sw);

	fontStore = gtk_list_store_new(3,
		G_TYPE_STRING,
		G_TYPE_STRING,
		PANGO_TYPE_FONT_DESCRIPTION);
	list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(fontStore));
	r = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes("Family", r,
		"text", 0,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), col);
	r = gtk_cell_renderer_text_new();
	g_object_bind_property(gtk_range_get_adjustment(GTK_RANGE(sizeSlider)), "value",
		r, "size-points",
		G_BINDING_SYNC_CREATE);
	col = gtk_tree_view_column_new_with_attributes("Sample", r,
		"text", 1,
		"font-desc", 2,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), col);
	gtk_container_add(GTK_CONTAINER(sw), list);

	pcfm = pango_cairo_font_map_get_default();
	pango_font_map_list_families(pcfm, &fams, &n);
	for (i = 0; i < n; i++) {
		PangoFontFace **faces;
		int j, m;

		pango_font_family_list_faces(fams[i], &faces, &m);
		for (j = 0; j < m; j++) {
			PangoFontDescription *desc;
			GtkTreeIter iter;

			desc = pango_font_face_describe(faces[j]);
			if (pango_font_description_get_variant(desc) != PANGO_VARIANT_SMALL_CAPS) {
				pango_font_description_free(desc);
				continue;
			}
			gtk_list_store_append(fontStore, &iter);
			gtk_list_store_set(fontStore, &iter,
				0, pango_font_description_get_family(desc),
				1, "The quick brown fox jumped over the lazy dog.",
				2, desc,
				-1);
		}
		g_free(faces);
	}
	g_free(fams);

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
