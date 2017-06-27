// 26 june 2017
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define WINVER			0x0600
#define _WIN32_WINNT		0x0600
#define _WIN32_WINDOWS	0x0600
#define _WIN32_IE			0x0700
#define NTDDI_VERSION		0x06000000
#include <windows.h>
#include <usp10.h>
#include <d2d1.h>
#include <dwrite.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// build:
// msvc: cl winunicodeshapetest.cpp -EHsc -W4 -Zi -link -debug user32.lib kernel32.lib usp10.lib gdi32.lib ole32.lib dwrite.lib shell32.lib
// mingw: TODO

void die(const char *msg, HRESULT hr)
{
	fprintf(stderr, "%s: 0x%08I32X\n", msg, hr);
	exit(1);
}

void dieLE(const char *msg)
{
	DWORD le;
	HRESULT hr;

	le = GetLastError();
	hr = HRESULT_FROM_WIN32(le);
	if (le == 0)
		hr = E_FAIL;
	die(msg, hr);
}

struct scriptItemizeParams {
	WCHAR *pwcInChars;
	int cInChars;
	int cMaxItems;
	const SCRIPT_CONTROL *psControl;
	const SCRIPT_STATE *psState;
	SCRIPT_ITEM *pItems;
	OPENTYPE_TAG *pScriptTags;
	int *pcItems;
};

BOOL doScriptItemize(struct scriptItemizeParams *p)
{
	HRESULT hr;

	hr = ScriptItemize(p->pwcInChars, p->cInChars,
		p->cMaxItems, p->psControl, p->psState,
		p->pItems, p->pcItems);
	if (hr == S_OK)
		return TRUE;
	if (hr != E_OUTOFMEMORY)
		die("error calling ScriptItemize()", hr);
	return FALSE;
}

BOOL doScriptItemizeOpenType(struct scriptItemizeParams *p)
{
	HRESULT hr;

	hr = ScriptItemizeOpenType(p->pwcInChars, p->cInChars,
		p->cMaxItems, p->psControl, p->psState,
		p->pItems, p->pScriptTags, p->pcItems);
	if (hr == S_OK)
		return TRUE;
	if (hr != E_OUTOFMEMORY)
		die("error calling ScriptItemizeOpenType()", hr);
	return FALSE;
}

struct scriptShapeParams {
	HDC hdc;
	SCRIPT_CACHE *psc;
	const WCHAR *pwcChars;
	int cChars;
	int cMaxGlyphs;
	SCRIPT_ANALYSIS *psa;
	WORD *pwOutGlyphs;
	WORD *pwLogClust;
	SCRIPT_VISATTR *psva;
	int *pcGlyphs;

	OPENTYPE_TAG tagScript;
	OPENTYPE_TAG tagLangSys;
	int *rcRangeChars;
	TEXTRANGE_PROPERTIES **rpRangeProperties;
	int cRanges;
	SCRIPT_CHARPROP *pCharProps;
	SCRIPT_GLYPHPROP *pOutGlyphProps;
};

BOOL doScriptShape(struct scriptShapeParams *p)
{
	HRESULT hr;

	hr = ScriptShape(p->hdc, p->psc, p->pwcChars, p->cChars,
		p->cMaxGlyphs, p->psa,
		p->pwOutGlyphs, p->pwLogClust, p->psva,
		p->pcGlyphs);
	if (hr == S_OK)
		return TRUE;
	if (hr != E_OUTOFMEMORY)
		die("error calling ScriptShape()", hr);
	return FALSE;
}

BOOL doScriptShapeOpenType(struct scriptShapeParams *p)
{
	HRESULT hr;

	hr = ScriptShapeOpenType(p->hdc, p->psc, p->psa,
		p->tagScript, p->tagLangSys,
		p->rcRangeChars, p->rpRangeProperties, p->cRanges,
		p->pwcChars, p->cChars, p->cMaxGlyphs,
		p->pwLogClust, p->pCharProps, p->pwOutGlyphs,
		p->pOutGlyphProps, p->pcGlyphs);
	if (hr == S_OK)
		return TRUE;
	if (hr != E_OUTOFMEMORY)
		die("error calling ScriptShapeOpenType()", hr);
	return FALSE;
}

void uniscribeOnly(HDC dc, WCHAR *string, int len, BOOL (*itemize)(struct scriptItemizeParams *p), const char *label)
{
	SCRIPT_CONTROL scriptControl;
	SCRIPT_STATE scriptState;
	SCRIPT_ITEM *items;
	OPENTYPE_TAG *ottags;
	int nItems, nActualItems;
	SCRIPT_CACHE cache;
	int i;
	std::vector<WORD> glyphs;
	size_t gi;
	HRESULT hr;

	nItems = len + 2;
	for (;;) {
		struct scriptItemizeParams p;

		items = new SCRIPT_ITEM[nItems + 1];
		ottags = new OPENTYPE_TAG[nItems];
		ZeroMemory(items, (nItems + 1) * sizeof (SCRIPT_ITEM));
		ZeroMemory(&scriptControl, sizeof (SCRIPT_CONTROL));
		ZeroMemory(&scriptState, sizeof (SCRIPT_STATE));
		ZeroMemory(ottags, nItems * sizeof (OPENTYPE_TAG));
		p.pwcInChars = string;
		p.cInChars = len;
		p.cMaxItems = nItems;
		p.psControl = &scriptControl;
		p.psState = &scriptState;
		p.pItems = items;
		p.pScriptTags = ottags;
		p.pcItems = &nActualItems;
		if ((*itemize)(&p))
			break;
		delete[] ottags;
		delete[] items;
		nItems *= 2;
	}
	// TODO call ScriptLayout() here?
	// TODO reset cache every time ScriptShape() is called so as to have no cache whatsoever?
	cache = NULL;
	for (i = 0; i < nActualItems; i++) {
		int nChars;
		WORD *logclusts;
		WORD *glyphbuf;
		SCRIPT_VISATTR *sva;
		int nGlyphs, nActualGlyphs;
		int j;

		nChars = items[i + 1].iCharPos - items[i].iCharPos;
		logclusts = new WORD[nChars];
		nGlyphs = 1.5 * nChars + 16;
		for (;;) {
			glyphbuf = new WORD[nGlyphs];
			sva = new SCRIPT_VISATTR[nGlyphs];
			ZeroMemory(logclusts, nChars * sizeof (WORD));
			ZeroMemory(glyphbuf, nGlyphs * sizeof (WORD));
			ZeroMemory(sva, nGlyphs * sizeof (SCRIPT_VISATTR));
			hr = ScriptShape(dc, &cache,
				string + items[i].iCharPos, nChars,
				nGlyphs, &(items[i].a),
				glyphbuf, logclusts, sva, &nActualGlyphs);
			if (hr == S_OK)
				break;
			if (hr != E_OUTOFMEMORY)
				die("error calling ScriptShape()", hr);
			delete[] sva;
			delete[] glyphbuf;
			nGlyphs *= 2;
		}
		for (j = 0; j < nActualGlyphs; j++)
			glyphs.push_back(glyphbuf[j]);
		delete[] sva;
		delete[] glyphbuf;
		delete[] logclusts;
	}
	// TODO do we call ScriptLayout() *here* instead?
	delete[] ottags;
	delete[] items;

	printf("%s:", label);
	for (gi = 0; gi < glyphs.size(); gi++)
		printf(" %hu", glyphs[gi]);
	printf("\n");
}

int main(void)
{
	int argc;
	LPWSTR *argv;
	WCHAR *fontname, *string;
	int len;
	HDC dc;
	HFONT font, prevfont;
	HRESULT hr;

	// TODO would using wmain() be adequate?
	argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argv == NULL)
		dieLE("error getting command-line arguments");
	if (argc != 3) {
		fprintf(stderr, "usage: %ws font string\n", argv[0]);
		return 1;
	}
	fontname = argv[1];
	string = argv[2];
	len = wcslen(string);

	// DirectWrite requires COM
	hr = CoInitialize(NULL);
	if (hr != S_OK)
		die("error initializing COM", hr);
	// Uniscribe requires a device context with the font to use
	dc = GetDC(NULL);
	if (dc == NULL)
		dieLE("error getting screen HDC for Uniscribe");
	// TODO DEFAULT_CHARSET might affect the results we get
	font = CreateFontW(0, 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, fontname);
	if (font == NULL)
		dieLE("error creating font for Uniscribe");
	prevfont = (HFONT) SelectObject(dc, font);
	if (prevfont == NULL)
		dieLE("error selecting font into HDC for Uniscribe");

	uniscribeOnly(dc, string, len,
		doScriptItemize, "Uniscribe");
	uniscribeOnly(dc, string, len,
		doScriptItemizeOpenType, "Uniscribe OpenType");

	SelectObject(dc, prevfont);
	DeleteObject(font);
	ReleaseDC(NULL, dc);
	CoUninitialize();
	return 0;
}
