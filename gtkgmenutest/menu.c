// 23 april 2015
#include "gtkgmenutest.h"

GMenuModel *makeMenu(void)
{
	GMenu *menubar;
	GMenu *fileMenu;
	GMenu *editMenu;
	GMenu *helpMenu;
	GMenu *section;
	GMenuItem *item;

	menubar = g_menu_new();

	fileMenu = g_menu_new();
	item = g_menu_item_new_submenu("File", G_MENU_MODEL(fileMenu));
	g_menu_append_item(menubar, item);

	section = g_menu_new();
	item = g_menu_item_new("New", "libui.test");
	g_menu_append_item(section, item);
	item = g_menu_item_new("Open", "libui.test");
	g_menu_append_item(section, item);
	g_menu_append_section(fileMenu, NULL, G_MENU_MODEL(section));

	section = g_menu_new();
	item = g_menu_item_new("Quit", "libui.test");
	g_menu_append_item(section, item);
	g_menu_append_section(fileMenu, NULL, G_MENU_MODEL(section));

	editMenu = g_menu_new();
	item = g_menu_item_new_submenu("Edit", G_MENU_MODEL(editMenu));
	g_menu_append_item(menubar, item);

	section = g_menu_new();
	item = g_menu_item_new("Undo...", "libui.test");
	g_menu_append_item(section, item);
	g_menu_append_section(editMenu, NULL, G_MENU_MODEL(section));

	section = g_menu_new();
	item = g_menu_item_new("Check Me\tTest", "libui.checkable");
	g_menu_append_item(section, item);
	item = g_menu_item_new("A&ccele&&rator T_es__t", "libui.test");
	g_menu_append_item(section, item);
	g_menu_append_section(editMenu, NULL, G_MENU_MODEL(section));

	section = g_menu_new();
	item = g_menu_item_new("Preferences...", "libui.test");
	g_menu_append_item(section, item);
	g_menu_append_section(editMenu, NULL, G_MENU_MODEL(section));

	helpMenu = g_menu_new();
	item = g_menu_item_new_submenu("Help", G_MENU_MODEL(helpMenu));
	g_menu_append_item(menubar, item);

	section = g_menu_new();
	item = g_menu_item_new("Help", "libui.test");
	g_menu_append_item(section, item);
	g_menu_append_section(helpMenu, NULL, G_MENU_MODEL(section));

	section = g_menu_new();
	item = g_menu_item_new("About", "libui.test");
	g_menu_append_item(section, item);
	g_menu_append_section(helpMenu, NULL, G_MENU_MODEL(section));

	return G_MENU_MODEL(menubar);
}
