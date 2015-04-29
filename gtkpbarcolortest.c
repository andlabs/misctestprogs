// 29 april 2015
#include <gtk/gtk.h>

GtkCssProvider *current = NULL;

const char CSSFormat[] = ".progressbar {\n"
	"background-image: linear-gradient(to bottom,\n"
	"	#%02X%02X%02X,\n"
	"	#%02X%02X%02X);\n"
"}\n";

void changeColor(GtkColorButton *cb, gpointer data)
{
	GtkStyleContext *context;
	GdkRGBA color;
	char *css;
	GError *err = NULL;

	context = gtk_widget_get_style_context(GTK_WIDGET(data));
	if (current != NULL) {
		gtk_style_context_remove_provider(context, GTK_STYLE_PROVIDER(current));
		g_object_unref(current);
	}
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(cb), &color);
	css = g_strdup_printf(CSSFormat,
		(int) (color.red * 255),
		(int) (color.green * 255),
		(int) (color.blue * 255),
		(int) (color.red * 255),
		(int) (color.green * 255),
		(int) (color.blue * 255));
	current = gtk_css_provider_new();
	if (gtk_css_provider_load_from_data(current, css, -1, &err) == FALSE)
		g_error("error parsing CSS: %s", err->message);
	gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(current), GTK_STYLE_PROVIDER_PRIORITY_USER);
	g_free(css);
}

int main(void)
{
	GtkWidget *window;
	GtkWidget *box;
	GtkWidget *colorbutton;
	GtkWidget *pbar;

	gtk_init(NULL, NULL);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 12);

	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
	gtk_container_add(GTK_CONTAINER(window), box);

	colorbutton = gtk_color_button_new();
	gtk_container_add(GTK_CONTAINER(box), colorbutton);

	pbar = gtk_progress_bar_new();
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar), 0.5);
	gtk_widget_set_hexpand(pbar, TRUE);
	gtk_widget_set_halign(pbar, GTK_ALIGN_FILL);
	gtk_container_add(GTK_CONTAINER(box), pbar);

	g_signal_connect(colorbutton, "color-set", G_CALLBACK(changeColor), pbar);

	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}
