// 12 june 2014
// scratch GTK+ program by pietro gagliardi 16-17 april 2014
// updated 27 may 2014
// uses code from 31 march 2014 and 15 april 2014
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_32
#define GLIB_VERSION_MAX_ALLOWED GLIB_VERSION_2_32
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_4
#define GDK_VERSION_MAX_ALLOWED GDK_VERSION_3_4
#include <gtk/gtk.h>

// flag variables here; use types gbooleean, gchar *, gint, gdouble

#define flagBool(name, help) { #name, 0, 0, G_OPTION_ARG_NONE, &name, help, NULL }
#define flagString(name, help) { #name, 0, 0, G_OPTION_ARG_STRING, &name, help, NULL }
#define flagInt(name, help) { #name, 0, 0, G_OPTION_ARG_INT, &name, help, NULL }
#define flagDouble(name, help) { #name, 0, 0, G_OPTION_ARG_DOUBLE, &name, help, NULL }
static GOptionEntry flags[] = {
	// options here
	{ NULL, 0, 0, 0, NULL, NULL, NULL },
};

void init(int *argc, char *(*argv[]));

char *args = "";		// other command-line arguments here, if any

gboolean parseArgs(int argc, char *argv[])
{
	// parse arguments here, if any
	// return TRUE if parsed successfully; FALSE otherwise
	// (returning FALSE will show usage and quit)
	return TRUE;
}

// this method of getting themes out comes to us from GtkParasite

void addDir(char *dirname, GPtrArray *list)
{
	GDir *dir;

	dir = g_dir_open(dirname, 0, NULL);		// TODO check for file not found explicitly
	if (dir == NULL)
		return;
	for (;;) {
		const char *name;
		char *testpath;

		name = g_dir_read_name(dir);
		if (name == NULL)				// done or error (TODO)
			break;
		testpath = g_build_filename(dirname, name, "gtk-3.0", NULL);
		if (g_file_test(testpath, G_FILE_TEST_IS_DIR))
			g_ptr_array_add(list, g_strdup(name));
		g_free(testpath);
	}
	g_dir_close(dir);
}

// g_ptr_array_sort() passes pointers TO the pointers, not the pointers themselves
gint ptrstrcmp(gconstpointer xa, gconstpointer xb)
{
	char **a, **b;

	a = (char **) xa;
	b = (char **) xb;
	return (gint) strcmp(*a, *b);
}

guint populate(GtkComboBoxText *themes)
{
	GSettings *settings;
	gchar *deftheme;

	settings = g_settings_new("org.gnome.desktop.interface");
	deftheme = g_settings_get_string(settings, "gtk-theme");
	g_object_unref(settings);

	GPtrArray *list;
	guint i;
	guint def = 0;
	gboolean deffound = FALSE;

	list = g_ptr_array_new();

	// TODO this is probably incomplete but alas GtkParasite gets the data directory at /configure time/ (not even compile time!)
	addDir("/usr/share/themes", list);
	addDir("/usr/local/share/themes", list);

	char *userthemes;

	userthemes = g_build_filename(g_get_user_data_dir(), "themes", NULL);
	addDir(userthemes, list);
	g_free(userthemes);

	g_ptr_array_sort(list, ptrstrcmp);
	for (i = 0; i < list->len; i++) {
		char *what;

		what = g_ptr_array_index(list, i);
		gtk_combo_box_text_append_text(themes, what);
		if (strcmp(what, deftheme) == 0) {
			if (deffound) {
				fprintf(stderr, "multiple versions of default theme %s found\n", deftheme);
				exit(1);
			}
			deffound = TRUE;
			def = i;
		}
	}
	if (!deffound) {
		fprintf(stderr, "could not find default theme %s\n", deftheme);
		exit(1);
	}
	return def;
}

void changeTheme(GtkComboBox *cb, gpointer data)
{
	g_object_set(gtk_settings_get_default(),
		"gtk-theme-name", gtk_combo_box_text_get_active_text((GtkComboBoxText *) cb),
		NULL);
}

GtkWidget *buildUI(void)
{
	GtkGrid *grid;
	GtkWidget *prev = NULL;		// for the ADD() macro

	grid = (GtkGrid *) gtk_grid_new();

#define EXPAND(widget, h, v) gtk_widget_set_hexpand((GtkWidget *) (widget), (h)); gtk_widget_set_vexpand((GtkWidget *) (widget), (v))
#define ALIGN(widget, h, v) gtk_widget_set_halign((GtkWidget *) (widget), GTK_ALIGN_ ## h); gtk_widget_set_valign((GtkWidget *) (widget), GTK_ALIGN_ ## v)
#define ADD(widget, side, xspan, yspan) gtk_grid_attach_next_to(grid, (GtkWidget *) widget, prev, GTK_POS_ ## side, (xspan), (yspan)); prev = (GtkWidget *) (widget)
#define ADDNEXTTO(widget, nextto, side, xspan, yspan) gtk_grid_attach_next_to(grid, (GtkWidget *) widget, (GtkWidget *) (nextto), GTK_POS_ ## side, (xspan), (yspan))
	// call EXPAND(), ALIGN(FILL/START/END/CENTER), and ADD(LEFT/TOP/RIGHT/BOTTOM)/ADDNEXTTO() for each widget

	GtkComboBoxText *themes;
	guint defaulttheme;

	themes = (GtkComboBoxText *) gtk_combo_box_text_new();
	defaulttheme = populate(themes);
	// g_object_bind_property() doesn't seem to work for this one
	g_signal_connect(themes, "changed", G_CALLBACK(changeTheme), NULL);
	gtk_combo_box_set_active((GtkComboBox *) themes, (gint) defaulttheme);
	EXPAND(themes, TRUE, FALSE);
	ALIGN(themes, FILL, FILL);
	ADD(themes, TOP, 1, 1);

	GtkCheckButton *dark;

	dark = (GtkCheckButton *) gtk_check_button_new_with_label("Dark Theme");
	g_object_bind_property(dark, "active",
		gtk_settings_get_default(), "gtk-application-prefer-dark-theme",
		G_BINDING_BIDIRECTIONAL);
	EXPAND(dark, FALSE, FALSE);
	ALIGN(dark, FILL, FILL);
	ADD(dark, RIGHT, 1, 1);

	GtkLayout *layout;
	GtkCssProvider *provider;
	GError *err = NULL;

#define CSS "GtkLayout { background-color: transparent; }"

	layout = (GtkLayout *) gtk_layout_new(NULL, NULL);
	provider = gtk_css_provider_new();
	if (gtk_css_provider_load_from_data(provider, CSS, -1, &err) == FALSE) {
		fprintf(stderr, "error loading custom CSS: %s\n", err->message);
		exit(1);
	}
	gtk_style_context_add_provider(gtk_widget_get_style_context((GtkWidget *) layout),
		(GtkStyleProvider *) provider,
		GTK_STYLE_PROVIDER_PRIORITY_USER);
	EXPAND(layout, TRUE, TRUE);
	ALIGN(layout, FILL, FILL);
	prev = (GtkWidget *) themes;
	ADD(layout, BOTTOM, 1, 1);

	layout = (GtkLayout *) gtk_layout_new(NULL, NULL);
	// this one won't have the style
	EXPAND(layout, TRUE, TRUE);
	ALIGN(layout, FILL, FILL);
	ADD(layout, RIGHT, 1, 1);

	return (GtkWidget *) grid;
}

int main(int argc, char *argv[])
{
	GtkWindow *mainwin;
	GtkWidget *layout;

	init(&argc, &argv);

	mainwin = (GtkWindow *) gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(mainwin, "Main Window");
	gtk_window_resize(mainwin, 320, 240);		// give it a useful initial size, rather than "as small as possible"
	g_signal_connect(mainwin, "delete-event", gtk_main_quit, NULL);

	layout = buildUI();
	gtk_container_add((GtkContainer *) mainwin, layout);
	gtk_widget_show_all((GtkWidget *) mainwin);

	gtk_main();
	return 0;
}

void init(int *argc, char *(*argv[]))
{
	GError *flagserr = NULL;
	GOptionContext *flagscontext;

	flagscontext = g_option_context_new(args);
	g_option_context_add_main_entries(flagscontext, flags, NULL);
	// the next line also initializes GTK+
	g_option_context_add_group(flagscontext, gtk_get_option_group(TRUE));
	if (g_option_context_parse(flagscontext, argc, argv, &flagserr) != TRUE) {
		fprintf(stderr, "error: %s\n", flagserr->message);
		goto usage;
	}

	if (parseArgs(*argc, *argv) == TRUE)
		return;
	// otherwise fall through

usage:
	fprintf(stderr, "%s\n", g_option_context_get_help(flagscontext, FALSE, NULL));
	exit(1);
}
