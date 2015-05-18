// 23 april 2015
#include "gtkgmenutest.h"

struct menu {
	GMenu *menubar;
	GMenu *fileMenu;
	GMenuItem *newItem;
	GMenuItem *openItem;
	GMenuItem *quitItem;
	GMenu *editMenu;
	GMenuItem *undoItem;
	GMenuItem *checkItem;
	GMenuItem *accelItem;
	GMenuItem *prefsItem;
	GMenu *helpMenu;
	GMenuItem *helpItem;
	GMenuItem *aboutItem;
};

struct menu *makeMenu(void)
{
	struct menu *m;
	GMenuItem *item;
	GMenu *section;

	m = g_new0(struct menu, 1);

	m->menubar = g_menu_new();

	m->fileMenu = g_menu_new();
	item = g_menu_item_new_submenu("File", G_MENU_MODEL(m->fileMenu));
	g_menu_append_item(m->menubar, item);

	section = g_menu_new();
	m->newItem = g_menu_item_new("New", NULL);
	g_menu_append_item(section, m->newItem);
	m->openItem = g_menu_item_new("Open", NULL);
	g_menu_append_item(section, m->openItem);
	g_menu_append_section(m->fileMenu, NULL, G_MENU_MODEL(section));

	section = g_menu_new();
	m->quitItem = g_menu_item_new("Quit", NULL);
	g_menu_append_item(section, m->quitItem);
	g_menu_append_section(m->fileMenu, NULL, G_MENU_MODEL(section));

	m->editMenu = g_menu_new();
	item = g_menu_item_new_submenu("Edit", G_MENU_MODEL(m->editMenu));
	g_menu_append_item(m->menubar, item);

	section = g_menu_new();
	m->undoItem = g_menu_item_new("Undo...", NULL);
	g_menu_append_item(section, m->undoItem);
	g_menu_append_section(m->editMenu, NULL, G_MENU_MODEL(section));

	section = g_menu_new();
	m->checkItem = g_menu_item_new("Check Me\tTest", NULL);
	g_menu_append_item(section, m->checkItem);
	m->accelItem = g_menu_item_new("A&ccele&&rator T_es__t", NULL);
	g_menu_append_item(section, m->accelItem);
	g_menu_append_section(m->editMenu, NULL, G_MENU_MODEL(section));

	section = g_menu_new();
	m->prefsItem = g_menu_item_new("Preferences...", NULL);
	g_menu_append_item(section, m->prefsItem);
	g_menu_append_section(m->editMenu, NULL, G_MENU_MODEL(section));

	m->helpMenu = g_menu_new();
	item = g_menu_item_new_submenu("Help", G_MENU_MODEL(m->helpMenu));
	g_menu_append_item(m->menubar, item);

	section = g_menu_new();
	m->helpItem = g_menu_item_new("Help", NULL);
	g_menu_append_item(section, m->helpItem);
	g_menu_append_section(m->helpMenu, NULL, G_MENU_MODEL(section));

	section = g_menu_new();
	m->aboutItem = g_menu_item_new("About", NULL);
	g_menu_append_item(section, m->aboutItem);
	g_menu_append_section(m->helpMenu, NULL, G_MENU_MODEL(section));

	return m;
}

GMenuModel *menuModel(struct menu *m)
{
	return G_MENU_MODEL(m->menubar);
}
