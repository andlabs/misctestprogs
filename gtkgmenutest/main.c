// 23 april 2015
#include "gtkgmenutest.h"

// #qo pkg-config: gtk+-3.0

GtkWidget *menu1, *menu2;
GtkWidget *lastLabel;

int main(void)
{
	GtkWidget *mainwin;
	GtkWidget *box;
	struct menu *menu;

	gtk_init(NULL, NULL);

	menu = makeMenu();

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(mainwin), "Multiple Menus Test");
	g_signal_connect(mainwin, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(mainwin), box);

	gtk_container_add(GTK_CONTAINER(box), gtk_label_new("Menubar 1"));
	menu1 = gtk_menu_bar_new_from_model(menuModel(menu));
	gtk_container_add(GTK_CONTAINER(box), menu1);

	gtk_container_add(GTK_CONTAINER(box), gtk_label_new("Menubar 2"));
	menu2 = gtk_menu_bar_new_from_model(menuModel(menu));
	gtk_container_add(GTK_CONTAINER(box), menu2);

	lastLabel = gtk_label_new("No previous item selected.");
	gtk_container_add(GTK_CONTAINER(box), lastLabel);

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
