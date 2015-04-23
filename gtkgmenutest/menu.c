// 23 april 2015
#include "gtkgmenutest.h"

GMenuModel *makeMenu(void)
{
	GMenu *menubar;
	GMenu *fileMenu;
	GMenu *editMenu;
	GMenu *helpMenu;
	GMenuItem *item;

	menubar = g_menu_new();

	fileMenu = g_menu_new();
	item = g_menu_item_new_submenu("File", G_MENU_MODEL(fileMenu));
	g_menu_append_item(menubar, item);

	editMenu = g_menu_new();
	item = g_menu_item_new_submenu("Edit", G_MENU_MODEL(editMenu));
	g_menu_append_item(menubar, item);

	helpMenu = g_menu_new();
	item = g_menu_item_new_submenu("Help", G_MENU_MODEL(helpMenu));
	g_menu_append_item(menubar, item);

	return G_MENU_MODEL(menubar);
}
