// 31 march 2015
#include <gtk/gtk.h>

GtkWidget *foreground;
GtkWidget *background;
GtkWidget *button;
GtkCssProvider *current = NULL;

const char CSSFormat[] = "* {"
	"color: #%02X%02X%02X; "
	"background-color:#%02X%02X%02X; "
"}\n";

void changeColor(GtkColorButton *cb, gpointer data)
{
	GtkStyleContext *context;
	GdkRGBA fg, bg;
	char *css;
	GError *err = NULL;

	context = gtk_widget_get_style_context(button);
	if (current != NULL) {
		gtk_style_context_remove_provider(context, GTK_STYLE_PROVIDER(current));
		g_object_unref(current);
	}
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(foreground), &fg);
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(background), &bg);
	css = g_strdup_printf(CSSFormat,
		(int) (fg.red * 255),
		(int) (fg.green * 255),
		(int) (fg.blue * 255),
		(int) (bg.red * 255),
		(int) (bg.green * 255),
		(int) (bg.blue * 255));
	current = gtk_css_provider_new();
	if (gtk_css_provider_load_from_data(current, css, -1, &err) == FALSE)
		g_error("error parsing CSS: %s", err->message);
	gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(current), GTK_STYLE_PROVIDER_PRIORITY_USER);
	g_free(css);
}

gboolean quit(GtkWidget *w, GdkEvent *e, gpointer data)
{
	gtk_main_quit();
	return TRUE;
}

int main(void)
{
	GtkWidget *mainwin;
	GtkWidget *grid;

	gtk_init(NULL, NULL);

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(mainwin), "Foreground Colors with CSS");
	gtk_container_set_border_width(GTK_CONTAINER(mainwin), 12);
	g_signal_connect(mainwin, "delete-event", G_CALLBACK(quit), NULL);

	grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
	gtk_container_add(GTK_CONTAINER(mainwin), grid);

	button = gtk_button_new_with_label("Color Me!");
	gtk_widget_set_hexpand(button, TRUE);
	gtk_widget_set_halign(button, GTK_ALIGN_FILL);
	gtk_widget_set_vexpand(button, TRUE);
	gtk_widget_set_halign(button, GTK_ALIGN_FILL);
	gtk_grid_attach_next_to(GTK_GRID(grid),
		button, NULL,
		GTK_POS_BOTTOM, 1, 1);

	foreground = gtk_color_button_new();
	g_signal_connect(foreground, "color-set", G_CALLBACK(changeColor), NULL);
	gtk_widget_set_hexpand(foreground, TRUE);
	gtk_widget_set_halign(foreground, GTK_ALIGN_FILL);
	gtk_grid_attach_next_to(GTK_GRID(grid),
		foreground, button,
		GTK_POS_BOTTOM, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(grid),
		gtk_label_new("Foreground"), foreground,
		GTK_POS_LEFT, 1, 1);

	background = gtk_color_button_new();
	g_signal_connect(background, "color-set", G_CALLBACK(changeColor), NULL);
	gtk_widget_set_hexpand(background, TRUE);
	gtk_widget_set_halign(background, GTK_ALIGN_FILL);
	gtk_grid_attach_next_to(GTK_GRID(grid),
		background, foreground,
		GTK_POS_BOTTOM, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(grid),
		gtk_label_new("Background"), background,
		GTK_POS_LEFT, 1, 1);

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
