/* 1 april 2014 */
/* thanks to http://www.lemoda.net/pango/list-fonts/ */
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

PangoFontMap *pfm;

char *mergeargs(int argc, char *argv[])
{
	GString *str;
	int i;

	str = g_string_new(argv[1]);
	for (i = 2; i < argc; i++) {
		str = g_string_append(str, " ");
		str = g_string_append(str, argv[i]);
	}
	return str->str;
}

void describeFace(PangoFontFace *face)
{
	printf("face name: %s\n", pango_font_face_get_face_name(face));
	printf("\n");
}

int findFont(int argc, char *argv[])
{
	int status = 0;
	char *familyname;
	PangoFontFamily **families = NULL;
	int i, nFamilies;
	PangoFontFamily *family;
	PangoFontFace **faces = NULL;
	int nFaces;

	familyname = mergeargs(argc, argv);
	pango_font_map_list_families(pfm, &families, &nFamilies);
	for (i = 0; i < nFamilies; i++) {
		family = families[i];
		if (strcmp(pango_font_family_get_name(family), familyname) == 0)
			goto familyFound;
	}
	fprintf(stderr, "could not find family %s (use %s by itself to get a full list of installed families)\n", familyname, argv[0]);
	status = 1;
	goto end;

familyFound:
	pango_font_family_list_faces(family, &faces, &nFaces);
	for (i = 0; i < nFaces; i++)
		describeFace(faces[i]);

end:
	if (faces != NULL)
		g_free(faces);
	if (families != NULL)
		g_free(families);
	return status;
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
	g_free(families);
	return 0;
}
