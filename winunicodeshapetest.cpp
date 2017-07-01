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
#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <functional>

// build:
// msvc: cl winunicodeshapetest.cpp -EHsc -W4 -wd4100 -Zi -link -debug user32.lib kernel32.lib usp10.lib gdi32.lib ole32.lib dwrite.lib shell32.lib shlwapi.lib
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

void printGlyphs(const char *label, const std::vector<UINT16> &glyphs)
{
	size_t i;

	printf("%35s:", label);
	for (i = 0; i < glyphs.size(); i++)
		printf(" %hu", glyphs[i]);
	printf("\n");
}

IDWriteFactory *dwfactory;

class featurePreparer {
	std::vector<OPENTYPE_TAG> tags;
	std::vector<LONG> values;
	bool prepared;
	OPENTYPE_FEATURE_RECORD *otfr;
	TEXTRANGE_PROPERTIES *tr;
	DWRITE_FONT_FEATURE *dff;
	DWRITE_TYPOGRAPHIC_FEATURES *dtf;
public:
	TEXTRANGE_PROPERTIES **usRangeProperties;
	int *usRangeChars;
	int usRanges;
	DWRITE_TYPOGRAPHIC_FEATURES **dwFeatures;
	UINT32 *dwFeatureRangeLengths;
	UINT32 dwFeatureRanges;

	featurePreparer()
	{
		this->prepared = false;
		this->otfr = NULL;
		this->tr = NULL;
		this->usRangeProperties = NULL;
		this->usRangeChars = NULL;
		this->usRanges = 0;
		this->dwFeatures = NULL;
		this->dwFeatureRangeLengths = NULL;
		this->dwFeatureRanges = 0;
	}

	~featurePreparer()
	{
		if (this->prepared) {
			delete[] this->dwFeatureRangeLengths;
			delete[] this->dwFeatures;
			delete this->dtf;
			delete[] this->dff;
			delete[] this->usRangeChars;
			delete[] this->usRangeProperties;
			delete this->tr;
			delete[] this->otfr;
		}
	}

	void add(char a, char b, char c, char d, LONG value)
	{
		this->tags.push_back((OPENTYPE_TAG) DWRITE_MAKE_OPENTYPE_TAG(a, b, c, d));
		this->values.push_back(value);
	}

	// TODO this probably isn't the correct use of the features array
	void prepare(size_t len)
	{
		size_t i, n;

		this->prepared = true;
		n = this->tags.size();
		this->otfr = new OPENTYPE_FEATURE_RECORD[n];
		this->dff = new DWRITE_FONT_FEATURE[n];
		for (i = 0; i < n; i++) {
			this->otfr[i].tagFeature = this->tags[i];
			this->otfr[i].lParameter = this->values[i];
			this->dff[i].nameTag = (DWRITE_FONT_FEATURE_TAG) (this->tags[i]);
			this->dff[i].parameter = this->values[i];
		}
		this->tr = new TEXTRANGE_PROPERTIES;
		this->tr->potfRecords = this->otfr;
		this->tr->cotfRecords = n;
		this->usRangeProperties = new TEXTRANGE_PROPERTIES *[1];
		this->usRangeProperties[0] = this->tr;
		this->usRangeChars = new int[1];
		this->usRangeChars[0] = len;
		this->usRanges = 1;
		this->dtf = new DWRITE_TYPOGRAPHIC_FEATURES;
		this->dtf->features = this->dff;
		this->dtf->featureCount = n;
		this->dwFeatures = new DWRITE_TYPOGRAPHIC_FEATURES *[1];
		this->dwFeatures[0] = this->dtf;
		this->dwFeatureRangeLengths = new UINT32[1];
		this->dwFeatureRangeLengths[0] = len;
		this->dwFeatureRanges = 1;
	}
};

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

void uniscribeTest(HDC dc, WCHAR *string, int len, featurePreparer *features, BOOL (*itemize)(struct scriptItemizeParams *p), BOOL (*shape)(struct scriptShapeParams *p), const char *label)
{
	SCRIPT_CONTROL scriptControl;
	SCRIPT_STATE scriptState;
	SCRIPT_ITEM *items;
	OPENTYPE_TAG *ottags;
	int nItems, nActualItems;
	SCRIPT_CACHE cache;
	int i;
	std::vector<UINT16> glyphs;			// UINT16 == WORD (TODO provide proof)

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
		SCRIPT_CHARPROP *charProps;
		SCRIPT_GLYPHPROP *glyphProps;
		int nGlyphs, nActualGlyphs;
		int j;

		nChars = items[i + 1].iCharPos - items[i].iCharPos;
		logclusts = new WORD[nChars];
		charProps = new SCRIPT_CHARPROP[nChars];
		nGlyphs = 1.5 * nChars + 16;
		for (;;) {
			scriptShapeParams p;

			glyphbuf = new WORD[nGlyphs];
			sva = new SCRIPT_VISATTR[nGlyphs];
			glyphProps = new SCRIPT_GLYPHPROP[nGlyphs];
			ZeroMemory(logclusts, nChars * sizeof (WORD));
			ZeroMemory(glyphbuf, nGlyphs * sizeof (WORD));
			ZeroMemory(sva, nGlyphs * sizeof (SCRIPT_VISATTR));
			ZeroMemory(charProps, nChars * sizeof (SCRIPT_CHARPROP));
			ZeroMemory(glyphProps, nGlyphs * sizeof (SCRIPT_GLYPHPROP));
			p.hdc = dc;
			p.psc = &cache;
			p.pwcChars = string + items[i].iCharPos;
			p.cChars = nChars;
			p.cMaxGlyphs = nGlyphs;
			p.psa = &(items[i].a);
			p.pwOutGlyphs = glyphbuf;
			p.pwLogClust = logclusts;
			p.psva = sva;
			p.pcGlyphs = &nActualGlyphs;
			p.tagScript = ottags[i];
			// 'dflt' in little-endian; see https://github.com/emacs-mirror/emacs/blob/master/src/w32uniscribe.c
			p.tagLangSys = 0x746C6664;
			p.rcRangeChars = NULL;
			p.rpRangeProperties = NULL;
			p.cRanges = 0;
			if (features != NULL) {
				p.rcRangeChars = features->usRangeChars;
				p.rpRangeProperties = features->usRangeProperties;
				p.cRanges = features->usRanges;
			}
			p.pCharProps = charProps;
			p.pOutGlyphProps = glyphProps;
			if ((*shape)(&p))
				break;
			delete[] glyphProps;
			delete[] sva;
			delete[] glyphbuf;
			nGlyphs *= 2;
		}
		for (j = 0; j < nActualGlyphs; j++)
			glyphs.push_back(glyphbuf[j]);
		delete[] glyphProps;
		delete[] charProps;
		delete[] sva;
		delete[] glyphbuf;
		delete[] logclusts;
	}
	// TODO do we call ScriptLayout() *here* instead?
	delete[] ottags;
	delete[] items;

	printGlyphs(label, glyphs);
}

bool scriptAnalysisEqual(DWRITE_SCRIPT_ANALYSIS a, DWRITE_SCRIPT_ANALYSIS b)
{
	return (a.script == b.script) && (a.shapes == b.shapes);
}

class analysisSourceSink : public IDWriteTextAnalysisSource, public IDWriteTextAnalysisSink {
	ULONG refcount;
public:
	WCHAR *string;
	int len;
	std::vector<DWRITE_SCRIPT_ANALYSIS> results;

	analysisSourceSink(WCHAR *string, int len)
	{
		this->refcount = 1;
		this->string = string;
		this->len = len;
		this->results.resize(this->len);
	}

	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		static const QITAB table[] = {
			QITABENT(analysisSourceSink, IDWriteTextAnalysisSource),
			QITABENT(analysisSourceSink, IDWriteTextAnalysisSink),
			{ 0 },
		};

		return QISearch(this, table, riid, ppvObject);
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		this->refcount++;
		return this->refcount;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		this->refcount--;
		if (this->refcount == 0) {
			delete this;
			return 0;
		}
		return this->refcount;
	}

	// IDWriteTextAnalysisSource
	virtual HRESULT STDMETHODCALLTYPE GetLocaleName(UINT32 textPosition, UINT32 *textLength, const WCHAR **localeName)
	{
		// TODO stub this properly (harfbuzz gets away with this)
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GetNumberSubstitution(UINT32 textPosition, UINT32 *textLength, IDWriteNumberSubstitution **numberSubstitution)
	{
		*textLength = this->len - textPosition;
		*numberSubstitution = NULL;
		return S_OK;
	}

	// TODO will this work for natively-RTL scripts like Arabic?
	virtual DWRITE_READING_DIRECTION STDMETHODCALLTYPE GetParagraphReadingDirection(void)
	{
		return DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
	}

	virtual HRESULT STDMETHODCALLTYPE GetTextAtPosition(UINT32 textPosition, const WCHAR **textString, UINT32 *textLength)
	{
		// in case textPosition is past the end, which can happen (this return value is documented on MSDN)
		*textString = NULL;
		*textLength = 0;
		if (textPosition < this->len) {
			*textString = this->string + textPosition;
			*textLength = this->len - textPosition;
		}
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GetTextBeforePosition(UINT32 textPosition, const WCHAR **textString, UINT32 *textLength)
	{
		// in case textPosition is at the beginning, which can happen (this return value is documented on MSDN)
		// harfbuzz does the past-the-end test (TODO is it correct?)
		*textString = NULL;
		*textLength = 0;
		if (textPosition > 0 && textPosition <= this->len) {
			*textString = this->string;
			*textLength = textPosition;
		}
		return S_OK;
	}

	// IDWriteTextAnalysisSink
	virtual HRESULT STDMETHODCALLTYPE SetBidiLevel(UINT32 textPosition, UINT32 textLength, UINT8 explicitLevel, UINT8 resolvedLevel)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE SetLineBreakpoints(UINT32 textPosition, UINT32 textLength, const DWRITE_LINE_BREAKPOINT *lineBreakpoints)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE SetNumberSubstitution(UINT32 textPosition, UINT32 textLength, IDWriteNumberSubstitution *numberSubstitution)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE SetScriptAnalysis(UINT32 textPosition, UINT32 textLength, const DWRITE_SCRIPT_ANALYSIS *scriptAnalysis)
	{
		UINT32 i;

		for (i = 0; i < textLength; i++)
			this->results[textPosition + i] = *scriptAnalysis;
		return S_OK;
	}

	// other functions
	void processResults(std::function<void(DWRITE_SCRIPT_ANALYSIS scriptAnalysis, UINT32 start, UINT32 end)> each)
	{
		DWRITE_SCRIPT_ANALYSIS prev;
		int i;
		UINT32 start;

		if (this->len == 0)
			return;
		prev = this->results[0];
		start = 0;
		for (i = 1; i < this->len; i++) {
			if (scriptAnalysisEqual(prev, this->results[i]))
				continue;
			each(prev, start, i);
			start = i;
			prev = this->results[i];
		}
		// and handle the last one
		each(prev, start, i);
	}
};

void directwriteAnalyzerTest(IDWriteFontFace *fontFace, WCHAR *string, int len, featurePreparer *features)
{
	IDWriteTextAnalyzer *analyzer;
	analysisSourceSink *ss;
	const DWRITE_TYPOGRAPHIC_FEATURES **dwfeatures;
	UINT32 *dwfeatureRangeLengths;
	UINT32 dwfeatureRanges;
	std::vector<UINT16> glyphs;
	HRESULT hr;

	hr = dwfactory->CreateTextAnalyzer(&analyzer);
	if (hr != S_OK)
		die("error creating IDWriteTextAnalyzer", hr);
	ss = new analysisSourceSink(string, len);

	dwfeatures = NULL;
	dwfeatureRangeLengths = NULL;
	dwfeatureRanges = 0;
	if (features != NULL) {
		// TODO WTF IS THIS C++? or is this the DirectWrite devs failing?
		dwfeatures = (const DWRITE_TYPOGRAPHIC_FEATURES **) (features->dwFeatures);
		dwfeatureRangeLengths = features->dwFeatureRangeLengths;
		dwfeatureRanges = features->dwFeatureRanges;
	}

	hr = analyzer->AnalyzeScript(ss, 0, len, ss);
	if (hr != S_OK)
		die("error analyzing scripts for DirectWrite", hr);
	ss->processResults([&](DWRITE_SCRIPT_ANALYSIS scriptAnalysis, UINT32 start, UINT32 end) {
		UINT16 *clusterMap;
		DWRITE_SHAPING_TEXT_PROPERTIES *textProps;
		UINT16 *glyphIndices;
		DWRITE_SHAPING_GLYPH_PROPERTIES *glyphProps;
		UINT32 nGlyphs, nActualGlyphs;
		UINT32 i;

		clusterMap = new UINT16[end - start];
		textProps = new DWRITE_SHAPING_TEXT_PROPERTIES[end - start];
		nGlyphs = (3 * (end - start) / 2 + 16);
		for (;;) {
			glyphIndices = new UINT16[nGlyphs];
			glyphProps = new DWRITE_SHAPING_GLYPH_PROPERTIES[nGlyphs];
			ZeroMemory(clusterMap, (end - start) * sizeof (UINT16));
			ZeroMemory(textProps, (end - start) * sizeof (DWRITE_SHAPING_TEXT_PROPERTIES));
			ZeroMemory(glyphIndices, nGlyphs * sizeof (UINT16));
			ZeroMemory(glyphProps, nGlyphs * sizeof (DWRITE_SHAPING_GLYPH_PROPERTIES));
			hr = analyzer->GetGlyphs(string + start, end - start,
				fontFace, FALSE, FALSE,
				&scriptAnalysis, NULL, NULL,
				dwfeatures, dwfeatureRangeLengths, dwfeatureRanges,
				nGlyphs, clusterMap, textProps,
				glyphIndices, glyphProps, &nActualGlyphs);
			if (hr == S_OK)
				break;
			if (hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
				die("error analyzing text with DirectWrite", hr);
			delete[] glyphProps;
			delete[] glyphIndices;
			nGlyphs *= 2;
		}
		for (i = 0; i < nActualGlyphs; i++)
			glyphs.push_back(glyphIndices[i]);
		delete[] glyphProps;
		delete[] glyphIndices;
		delete[] textProps;
		delete[] clusterMap;
	});
	printGlyphs("DirectWrite IDWriteTextAnalyzer", glyphs);

	delete ss;
	analyzer->Release();
}

int main(void)
{
	int argc;
	LPWSTR *argv;
	WCHAR *fontname, *string;
	int len;
	HDC dc;
	HFONT font, prevfont;
	IDWriteFontCollection *sysfc;
	UINT32 index;
	BOOL exists;
	IDWriteFontFamily *dwfamily;
	IDWriteFont *dwfont;
	IDWriteFontFace *dwface;
	featurePreparer *features;
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
	font = CreateFontW(0, 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontname);
	if (font == NULL)
		dieLE("error creating font for Uniscribe");
	prevfont = (HFONT) SelectObject(dc, font);
	if (prevfont == NULL)
		dieLE("error selecting font into HDC for Uniscribe");
	// and initialize DirectWrite
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof (IDWriteFactory),
		(IUnknown **) (&dwfactory));
	if (hr != S_OK)
		die("error initializing DirectWrite", hr);
	// and load the font *there*
	hr = dwfactory->GetSystemFontCollection(&sysfc, TRUE);
	if (hr != S_OK)
		die("error loading DirectWrite system font collection", hr);
	hr = sysfc->FindFamilyName(fontname, &index, &exists);
	if (hr != S_OK)
		die("error finding DirectWrite font family", hr);
	if (!exists)
		die("font not found in DirectWrite system font collection", E_FAIL);
	hr = sysfc->GetFontFamily(index, &dwfamily);
	if (hr != S_OK)
		die("error loading DirectWrite font family", hr);
	hr = dwfamily->GetFirstMatchingFont(DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		&dwfont);
	if (hr != S_OK)
		die("error loading DirectWrite font object", hr);
	hr = dwfont->CreateFontFace(&dwface);
	if (hr != S_OK)
		die("error creating DirectWrite font face", hr);

	// first, uniscribe only; no features are used
//	uniscribeTest(dc, string, len, NULL,
//		doScriptItemize, doScriptShape, "Uniscribe");

	// next, unprepared features (NULL values)
	features = new featurePreparer;
//features->add('z','e','r','o',1);
features->add('f','r','a','c',1);features->prepare(len);
//	uniscribeTest(dc, string, len, features,
//		doScriptItemizeOpenType, doScriptShapeOpenType, "Uniscribe OpenType");
	directwriteAnalyzerTest(dwface, string, len, features);
	delete features;

	dwface->Release();
	dwfont->Release();
	dwfamily->Release();
	sysfc->Release();
	dwfactory->Release();
	SelectObject(dc, prevfont);
	DeleteObject(font);
	ReleaseDC(NULL, dc);
	CoUninitialize();
	return 0;
}
