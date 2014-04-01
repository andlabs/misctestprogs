/* 1 april 2014 */
/* thanks to http://www.lemoda.net/pango/list-fonts/ */
#include <stdio.h>
#include <gtk/gtk.h>

PangoFontMap *pfm;

int findFont(int argc, char *argv[])
{
	return 1;
}

int main(int argc, char *argv[])
{
	PangoFontFamily **families;
	int i, nFamilies;

	pfm = pango_cairo_font_map_get_default();

	if (argc > 1)
		return findFont(argc, argv);

	pango_font_map_list_families(pfm, &families, &nFamilies);
	for (i = 0; i < nFamilies; i++) {
		PangoFontFamily *family = families[i];

		printf("%5s %s\n",
			(pango_font_family_is_monospace(family) ? "true" : "false"),
			pango_font_family_get_name(family));
	}
	return 0;
}
