// 23 april 2015
#include <gtk/gtk.h>

// menu.c
extern struct menu *makeMenu(void);
extern GMenuModel *menuModel(struct menu *);

// action.c
extern GActionGroup *makeActions(void);
