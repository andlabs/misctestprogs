// 20 june 2017
// windwritefeatureclobber.cpp 11 june 2017
// wind2dmingwtest.cpp 5 june 2016
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
#include <d2d1.h>
#include <dwrite.h>
#include <stdio.h>
#include <float.h>

ID2D1Factory *d2dfactory;
IDWriteFactory *dwfactory;
IDWriteTextFormat *labelFormat, *textFormat;

void loghr(const char *f, HRESULT hr)
{
	fprintf(stderr, "%s 0x%08I32X\n", f, hr);
}

#define margins 10.0
#define labelSize (9.0 * (96.0 / 72.0))
#define textSize (48.0 * (96.0 / 72.0))
#define ypadding 2.0

typedef HRESULT (*doTypographyFeaturesFunc)(IDWriteTypography *t);

//static const WCHAR *linebase = L"afford afire aflight";
//#define C 0x062A
#define C 0x063A
static const WCHAR linebase[] = { C, C, C, C, C, C, C, C, C, C, 0 };

HRESULT doLine(ID2D1RenderTarget *rt, const WCHAR *desc, double *y, ID2D1Brush *brush, doTypographyFeaturesFunc f)
{
	IDWriteTextLayout *layout;
	static const WCHAR *line;
	D2D1_POINT_2F pt;
	HRESULT hr;

	hr = dwfactory->CreateTextLayout(desc, wcslen(desc),
		labelFormat,
		FLT_MAX, FLT_MAX,
		&layout);
	if (hr != S_OK) {
		loghr("CreateTextLayout() label", hr);
		return hr;
	}
	pt.x = margins;
	pt.y = *y;
	rt->DrawTextLayout(pt, layout, brush, D2D1_DRAW_TEXT_OPTIONS_NONE);
	layout->Release();
	*y += labelSize + ypadding;
	line = linebase;
	hr = dwfactory->CreateTextLayout(line, wcslen(line),
		textFormat,
		FLT_MAX, FLT_MAX,
		&layout);
	if (hr != S_OK) {
		loghr("CreateTextLayout() text", hr);
		rt->EndDraw(NULL, NULL);
		return hr;
	}
	if (f != NULL) {
		IDWriteTypography *t;
		DWRITE_TEXT_RANGE range;

		hr = dwfactory->CreateTypography(&t);
		if (hr != S_OK) {
			loghr("CreateTypography()", hr);
			layout->Release();
			return hr;
		}
		hr = (*f)(t);
		if (hr != S_OK && hr != S_FALSE) {
			loghr("user function", hr);
			t->Release();
			layout->Release();
			return hr;
		}
		if (hr == S_FALSE) {
			t->Release();
			t = NULL;
		}
		range.startPosition = 0;
		range.length = wcslen(line);
		hr = layout->SetTypography(t, range);
		if (t != NULL)
			t->Release();
		if (hr != S_OK) {
			loghr("SetTypography()", hr);
			layout->Release();
			return hr;
		}
	}
	pt.x = margins;
	pt.y = *y;
	rt->DrawTextLayout(pt, layout, brush, D2D1_DRAW_TEXT_OPTIONS_NONE);
	layout->Release();
	*y += textSize + (2 * ypadding);
	return S_OK;
}

#define otherFeatureName L"frac"
#define otherFeature ((DWRITE_FONT_FEATURE_TAG) DWRITE_MAKE_OPENTYPE_TAG('f', 'r', 'a', 'c'))

HRESULT doPaint(ID2D1RenderTarget *rt)
{
	COLORREF syscolor;
	D2D1_COLOR_F color;
	ID2D1SolidColorBrush *brush;
	IDWriteTypography *typography;
	double y;
	HRESULT hr;

	rt->BeginDraw();

	syscolor = GetSysColor(COLOR_BTNFACE);
	color.r = ((double) GetRValue(syscolor)) / 255.0;
	color.g = ((double) GetGValue(syscolor)) / 255.0;
	color.b = ((double) GetBValue(syscolor)) / 255.0;
	color.a = 1.0;
	rt->Clear(&color);

	color.r = 0.0;
	color.g = 0.0;
	color.b = 0.0;
	color.a = 1.0;
	hr = rt->CreateSolidColorBrush(&color, NULL, &brush);
	if (hr != S_OK) {
		loghr("CreateSolidColorBrush()", hr);
		rt->EndDraw(NULL, NULL);
		return hr;
	}

	y = margins;
	hr = doLine(rt, L"No call to SetTypography()", &y, brush,
		NULL);
	if (hr != S_OK) {
		rt->EndDraw(NULL, NULL);
		return hr;
	}
	hr = doLine(rt, L"NULL IDWriteTypography", &y, brush,
		[](IDWriteTypography *t) -> HRESULT {
			return S_FALSE;
		});
	if (hr != S_OK) {
		rt->EndDraw(NULL, NULL);
		return hr;
	}
	hr = doLine(rt, L"Empty IDWriteTypography", &y, brush,
		[](IDWriteTypography *t) -> HRESULT {
			return S_OK;
		});
	if (hr != S_OK) {
		rt->EndDraw(NULL, NULL);
		return hr;
	}
	hr = doLine(rt, L"IDWriteTypography with liga", &y, brush,
		[](IDWriteTypography *t) -> HRESULT {
			DWRITE_FONT_FEATURE ff;

			ff.nameTag = (DWRITE_FONT_FEATURE_TAG) DWRITE_MAKE_OPENTYPE_TAG('l', 'i', 'g', 'a');
			ff.parameter = 1;
			return t->AddFontFeature(ff);
		});
	if (hr != S_OK) {
		rt->EndDraw(NULL, NULL);
		return hr;
	}
	hr = doLine(rt, L"IDWriteTypography with " otherFeatureName, &y, brush,
		[](IDWriteTypography *t) -> HRESULT {
			DWRITE_FONT_FEATURE ff;

			ff.nameTag = otherFeature;
			ff.parameter = 1;
			return t->AddFontFeature(ff);
		});
	if (hr != S_OK) {
		rt->EndDraw(NULL, NULL);
		return hr;
	}
	hr = doLine(rt, L"IDWriteTypography with liga and " otherFeatureName, &y, brush,
		[](IDWriteTypography *t) -> HRESULT {
			DWRITE_FONT_FEATURE ff;
			HRESULT hr;

			ff.nameTag = (DWRITE_FONT_FEATURE_TAG) DWRITE_MAKE_OPENTYPE_TAG('l', 'i', 'g', 'a');
			ff.parameter = 1;
			hr = t->AddFontFeature(ff);
			if (hr != S_OK)
				return hr;
			ff.nameTag = otherFeature;
			ff.parameter = 1;
			return t->AddFontFeature(ff);
		});
	if (hr != S_OK) {
		rt->EndDraw(NULL, NULL);
		return hr;
	}

	brush->Release();
	hr = rt->EndDraw(NULL, NULL);
	if (hr != S_OK && hr != D2DERR_RECREATE_TARGET)
		loghr("EndDraw()", hr);
	return hr;
}

ID2D1HwndRenderTarget *mkRenderTarget(HWND hwnd)
{
	D2D1_RENDER_TARGET_PROPERTIES props;
	D2D1_HWND_RENDER_TARGET_PROPERTIES hprops;
	HDC dc;
	RECT r;
	ID2D1HwndRenderTarget *rt;
	HRESULT hr;

	dc = GetDC(hwnd);
	ZeroMemory(&props, sizeof (D2D1_RENDER_TARGET_PROPERTIES));
	props.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
	props.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
	props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_UNKNOWN;
	props.dpiX = GetDeviceCaps(dc, LOGPIXELSX);
	props.dpiY = GetDeviceCaps(dc, LOGPIXELSY);
	props.usage = D2D1_RENDER_TARGET_USAGE_NONE;
	props.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
	ReleaseDC(hwnd, dc);

	GetClientRect(hwnd, &r);
	ZeroMemory(&hprops, sizeof (D2D1_HWND_RENDER_TARGET_PROPERTIES));
	hprops.hwnd = hwnd;
	hprops.pixelSize.width = r.right - r.left;
	hprops.pixelSize.height = r.bottom - r.top;
	// according to Rick Brewster, some drivers will misbehave if we don't specify this (see http://stackoverflow.com/a/33222983/3408572)
	hprops.presentOptions = D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS;

	hr = d2dfactory->CreateHwndRenderTarget(
		&props,
		&hprops,
		&rt);
	if (hr != S_OK) {
		loghr("CreateHwndRenderTarget()", hr);
		return NULL;
	}
	return rt;
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR userdata;
	ID2D1HwndRenderTarget *rt = NULL;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	RECT client;
	D2D1_SIZE_U size;

	userdata = GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (userdata == 0) {
		if (uMsg == WM_CREATE)
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, 1);
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	if (userdata == 1) {
		rt = mkRenderTarget(hwnd);
		if (rt == NULL) {
			PostQuitMessage(1);
			return DefWindowProcW(hwnd, uMsg, wParam, lParam);
		}
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) rt);
	} else
		rt = (ID2D1HwndRenderTarget *) userdata;

	switch (uMsg) {
	case WM_PAINT:
		switch (doPaint(rt)) {
		case S_OK:
			ValidateRect(hwnd, NULL);
			return 0;
		case D2DERR_RECREATE_TARGET:
			rt->Release();
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, 1);
			return 0;
		}
		rt->Release();
		PostQuitMessage(1);
		break;
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOSIZE) != 0)
			break;
		GetClientRect(hwnd, &client);
		size.width = client.right - client.left;
		size.height = client.bottom - client.top;
		rt->Resize(&size);
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
	case WM_CLOSE:
		rt->Release();
		PostQuitMessage(0);
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(int argc, char *argv[])
{
	D2D1_FACTORY_OPTIONS opts;
	WNDCLASSW wc;
	HWND mainwin;
	MSG msg;
	HRESULT hr;

	CoInitialize(NULL);
	ZeroMemory(&opts, sizeof (D2D1_FACTORY_OPTIONS));
	opts.debugLevel = D2D1_DEBUG_LEVEL_NONE;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
		IID_ID2D1Factory,
		&opts,
		(void **) (&d2dfactory));
	if (hr != S_OK) {
		loghr("D2D1CreateFactory()", hr);
		return 1;
	}
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof (IDWriteFactory),
		(IUnknown **) (&dwfactory));
	if (hr != S_OK) {
		loghr("DWriteCreateFactory()", hr);
		return 1;
	}
	hr = dwfactory->CreateTextFormat(L"Segoe UI", NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		labelSize,
		L"",
		&labelFormat);
	if (hr != S_OK) {
		loghr("CreateTextFormat() label", hr);
		return 1;
	}
	hr = dwfactory->CreateTextFormat(L"Constantia", NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		textSize,
		L"",
		&textFormat);
	if (hr != S_OK) {
		loghr("CreateTextFormat() label", hr);
		return 1;
	}

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	mainwin = CreateWindowExW(0,
		L"mainwin", L"mainwin",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		640, 580,
		NULL, NULL, NULL, NULL);

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	textFormat->Release();
	labelFormat->Release();
	dwfactory->Release();
	d2dfactory->Release();
	CoUninitialize();
	return msg.wParam;
}
