// 2 april 2015
#include <gtk/gtk.h>

// #qo pkg-config: gtk+-3.0

#define MyDialogType (MyDialog_get_type())
#define MyDialog(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), MyDialogType, MyDialog))
#define IsMyDialog(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), MyDialogType))
#define MyDialogClass(class) (G_TYPE_CHECK_CLASS_CAST((class), MyDialogType, MyDialogClass))
#define IsMyDialogClass(class) (G_TYPE_CHECK_CLASS_TYPE((class), MyDialog))
#define GetMyDialogClass(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), MyDialogType, MyDialogClass))
typedef struct MyDialog MyDialog;
typedef struct MyDialogClass MyDialogClass;
struct MyDialog {
	GtkDialog parent_instance;
	struct MyDialogPrivate *priv;
};
struct MyDialogClass {
	GtkDialogClass parent_class;
};
static GType MyDialog_get_type(void);

typedef struct MyDialogPrivate MyDialogPrivate;

struct MyDialogPrivate {
};

G_DEFINE_TYPE_WITH_CODE(MyDialog, MyDialog, GTK_TYPE_DIALOG,
	G_ADD_PRIVATE(MyDialog))

static void MyDialog_init(MyDialog *o)
{
	o->priv = MyDialog_get_instance_private(o);

	gtk_widget_init_template(GTK_WIDGET(o));
}

static void MyDialog_dispose(GObject *obj)
{
	G_OBJECT_CLASS(MyDialog_parent_class)->dispose(obj);
}

static void MyDialog_finalize(GObject *obj)
{
	G_OBJECT_CLASS(MyDialog_parent_class)->finalize(obj);
}

static void MyDialog_class_init(MyDialogClass *class)
{
	gchar *contents;
	gsize len;
	GBytes *bytes;
	GError *err = NULL;

	G_OBJECT_CLASS(class)->dispose = MyDialog_dispose;
	G_OBJECT_CLASS(class)->finalize = MyDialog_finalize;

	if (g_file_get_contents("main.ui", &contents, &len, &err) == FALSE)
		g_error("error reading main.ui: %s", err->message);
	bytes = g_bytes_new_take(contents, len);
	gtk_widget_class_set_template(GTK_WIDGET_CLASS(class), bytes);
}

GtkWidget *newMyDialog(void)
{
	return GTK_WIDGET(g_object_new(MyDialogType, NULL));
}

void clicked(GtkButton *b, gpointer data)
{
	GtkWindow *parent = GTK_WINDOW(data);
	GtkWidget *dialog;

	dialog = newMyDialog();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
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
