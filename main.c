// 19 march 2014
#include <stdio.h>
#include <gtk/gtk.h>

GdkKeymap *def;

void dovirt(char *name, GdkModifierType mod)
{
	GdkModifierType from, to;
	gboolean mapped;

	from = mod;
	gdk_keymap_add_virtual_modifiers(def, &from);
	to = mod;
	mapped = gdk_keymap_map_virtual_modifiers(def, &to);
	printf("%s\t0x%X\t0x%X\t%d:0x%X\n",
		name, mod, from, mapped, to);
}

int main(void)
{
	gtk_init(NULL, NULL);
	def = gdk_keymap_get_default();
	printf("name\tmod\tadd\tmap\n");
	dovirt("meta", GDK_META_MASK);
	dovirt("super", GDK_SUPER_MASK);
	dovirt("hyper", GDK_HYPER_MASK);
	return 0;
}
