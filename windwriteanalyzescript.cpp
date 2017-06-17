// 17 june 2017
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
#include <dwrite.h>
#include <shlwapi.h>
#include <sstream>

IDWriteFactory *dwfactory;
HWND fontCombo;
HWND textEdit;
HWND resultsEdit;
HFONT controlFont;

class analysis : public IDWriteTextAnalysisSource, public IDWriteTextAnalysisSink {
	ULONG refcount;
	WCHAR *text;
public:
	std::wostringstream ss;
	UINT32 len;

	analysis()
	{
		this->refcount = 1;
		this->len = (UINT32) SendMessageW(textEdit, WM_GETTEXTLENGTH, 0, 0);
		this->text = new WCHAR[this->len + 1];
		GetWindowTextW(textEdit, this->text, this->len + 1);
	}

	~analysis()
	{
		delete[] this->text;
	}

	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		static const QITAB table[] = {
			QITABENT(analysis, IDWriteTextAnalysisSource),
			QITABENT(analysis, IDWriteTextAnalysisSink),
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
			*textString = this->text + textPosition;
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
			*textString = this->text;
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
		this->ss << textPosition << L" - " << textLength;
		this->ss << L" ";
		this->ss << L"script " << scriptAnalysis->script;
		this->ss << L" ";
		this->ss << L"shapes " << scriptAnalysis->shapes;
		this->ss << L"\r\n";
		return S_OK;
	}
};

static void fillFontCombo(void)
{
	IDWriteFontCollection *fc;
	WCHAR localeName[LOCALE_NAME_MAX_LENGTH];
	int hasLocaleName;
	UINT32 i, n;
	LRESULT arial;

	dwfactory->GetSystemFontCollection(&fc, TRUE);
	hasLocaleName = GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH);

	n = fc->GetFontFamilyCount();
	for (i = 0; i < n; i++) {
		IDWriteFontFamily *family;
		IDWriteLocalizedStrings *names;
		UINT32 index;
		BOOL exists;
		UINT32 length;
		WCHAR *name;
		HRESULT hr;

		fc->GetFontFamily(i, &family);
		family->GetFamilyNames(&names);

		hr = S_OK;
		exists = FALSE;
		if (hasLocaleName != 0)
			hr = names->FindLocaleName(localeName, &index, &exists);
		if (hr != S_OK || (hr == S_OK && !exists))
			hr = names->FindLocaleName(L"en-us", &index, &exists);
		if (hr != S_OK || (hr == S_OK && !exists))
			index = 0;

		names->GetStringLength(index, &length);
		name = new WCHAR[length + 1];
		names->GetString(index, name, length + 1);
		SendMessageW(fontCombo, CB_ADDSTRING, 0, (LPARAM) name);
		delete[] name;

		names->Release();
		family->Release();
	}

	fc->Release();
	arial = SendMessageW(fontCombo, CB_FINDSTRINGEXACT, (WPARAM) (-1), (LPARAM) L"Arial");
	if (arial == (LRESULT) CB_ERR)
		arial = 0;
	SendMessageW(fontCombo, CB_SETCURSEL, (WPARAM) arial, 0);
}

static void analyze(void)
{
	analysis *a;
	IDWriteTextAnalyzer *da;

	a = new analysis;
	dwfactory->CreateTextAnalyzer(&da);
	da->AnalyzeScript(a, 0, a->len, a);
	SetWindowTextW(resultsEdit, a->ss.str().c_str());
	da->Release();
	a->Release();
}

static LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int y, height;

	switch (uMsg) {
	case WM_CREATE:
		y = 10;
		height = 240 - 10 - 10;
		fontCombo = CreateWindowExW(0,
			L"COMBOBOX", L"",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_SORT,
			10, y, 300, height,
			hwnd, (HMENU) 100, NULL, NULL);
		SendMessageW(fontCombo, WM_SETFONT, (WPARAM) controlFont, (LPARAM) TRUE);
		fillFontCombo();
		y += 20 + 5;
		height -= 20 + 5 + 5;
		textEdit = CreateWindowExW(WS_EX_CLIENTEDGE,
			L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | ES_WANTRETURN,
			10, y, 300, height / 2,
			hwnd, (HMENU) 101, NULL, NULL);
		SendMessageW(textEdit, WM_SETFONT, (WPARAM) controlFont, (LPARAM) TRUE);
		y += height / 2 + 5;
		resultsEdit = CreateWindowExW(WS_EX_CLIENTEDGE,
			L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY | ES_WANTRETURN,
			10, y, 300, height / 2,
			hwnd, (HMENU) 102, NULL, NULL);
		SendMessageW(resultsEdit, WM_SETFONT, (WPARAM) controlFont, (LPARAM) TRUE);
		break;
	case WM_COMMAND:
		analyze();
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	NONCLIENTMETRICSW ncm;
	RECT r;
	WNDCLASSW wc;
	HWND mainwin;
	MSG msg;

	ZeroMemory(&ncm, sizeof (NONCLIENTMETRICSW));
	ncm.cbSize = sizeof (NONCLIENTMETRICSW);
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof (NONCLIENTMETRICSW), &ncm, sizeof (NONCLIENTMETRICSW));
	controlFont = CreateFontIndirectW(&(ncm.lfMessageFont));

	CoInitialize(NULL);
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof (IDWriteFactory),
		(IUnknown **) (&dwfactory));

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	r.left = 0;
	r.top = 0;
	r.right = 320;
	r.bottom = 240;
	AdjustWindowRectEx(&r,
		WS_OVERLAPPEDWINDOW,
		FALSE,
		0);
	mainwin = CreateWindowExW(0,
		L"mainwin", L"mainwin",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		r.right - r.left, r.bottom - r.top,
		NULL, NULL, NULL, NULL);

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	DestroyWindow(mainwin);
	dwfactory->Release();
	CoUninitialize();
	DeleteObject(controlFont);
	return 0;
}
