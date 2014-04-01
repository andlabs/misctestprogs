/* 1 april 2014 */
/* thanks to http://www.lemoda.net/pango/list-fonts/ */
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

PangoFontMap *pfm;

#define BOOLSTR(b) ((b) ? "true" : "false")

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

void printDescription(PangoFontDescription *desc)
{
	printf("desc->family: %s\n", pango_font_description_get_family(desc));
	printf("desc->style: ");
	switch (pango_font_description_get_style(desc)) {
	case PANGO_STYLE_NORMAL:
		printf("normal");
		break;
	case PANGO_STYLE_OBLIQUE:
		printf("oblique");
		break;
	case PANGO_STYLE_ITALIC:
		printf("italic");
		break;
	default:
		fprintf(stderr, "!!! unknown style\n");
	}
	printf("\n");
	printf("desc->variant: ");
	switch (pango_font_description_get_variant(desc)) {
	case PANGO_VARIANT_NORMAL:
		printf("normal");
		break;
	case PANGO_VARIANT_SMALL_CAPS:
		printf("small caps");
		break;
	default:
		fprintf(stderr, "!!! unknown variant\n");
	}
	printf("\n");
	/* TODO these should use strings */
	printf("desc->weight: %d\n", pango_font_description_get_weight(desc));
	printf("desc->stretch: %d\n", pango_font_description_get_stretch(desc));
	printf("desc->size: %d (absolute == %s)\n", pango_font_description_get_size(desc), BOOLSTR(pango_font_description_get_size_is_absolute(desc)));
	printf("desc->gravity: %d\n", pango_font_description_get_gravity(desc));
	/* TODO use pango_font_description_get_set_fields */
}

void describeFace(PangoFontFace *face)
{
	int *sizes;
	int i, nSizes;

	printf("face name: %s\n", pango_font_face_get_face_name(face));
	pango_font_face_list_sizes(face, &sizes, &nSizes);
	printf("sizes:");
	for (i = 0; i < nSizes; i++)
		printf(" %d", sizes[i]);
	printf("\n");
	g_free(sizes);
	printDescription(pango_font_face_describe(face));
	printf("synthesized: %s\n", BOOLSTR(pango_font_face_is_synthesized(face)));
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
	for (i = 0; i < nFaces; i++) {
		describeFace(faces[i]);
		if (i != nFaces - 1)
			printf("\n");
	}

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
			BOOLSTR(pango_font_family_is_monospace(family)),
			pango_font_family_get_name(family));
	}
	g_free(families);
	return 0;
}
