// 23 april 2015
#include "gtkgmenutest.h"

GActionGroup *makeActions(void)
{
	GSimpleActionGroup *group;
	GSimpleAction *test;
	GSimpleAction *checkable;

	group = g_simple_action_group_new();

	test = g_simple_action_new("test", G_VARIANT_TYPE_ANY);
	g_action_map_add_action(G_ACTION_MAP(group), G_ACTION(test));

	// will not leak memory (thanks hergertme in irc.gimp.net/#gtk+)
	checkable = g_simple_action_new_stateful("checkable", G_VARIANT_TYPE_ANY, g_variant_new_boolean(FALSE));
	g_action_map_add_action(G_ACTION_MAP(group), G_ACTION(checkable));

	return G_ACTION_GROUP(group);
}
