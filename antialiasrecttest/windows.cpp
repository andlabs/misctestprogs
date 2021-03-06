// 22 january 2017
// based on ../wind2dmingwtest.cpp
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define INITGUID
#define WINVER			0x0600
#define _WIN32_WINNT		0x0600
#define _WIN32_WINDOWS	0x0600
#define _WIN32_IE			0x0700
#define NTDDI_VERSION		0x06000000
#define CHARTYPE wchar_t
#define TOCHAR(s) L##s
#include <windows.h>
#include <windowsx.h>
#include <d2d1.h>
#include <stdio.h>
#include "shared.h"

ID2D1Factory *d2dfactory;
HWND chooser;
HWND antialias;
HWND sameColor;
HWND ra;

void loghr(const char *f, HRESULT hr)
{
	char hrs[20];

	OutputDebugStringA(f);
	OutputDebugStringA(" ");
	sprintf(hrs, "0x%08I32X\n", hr);
	OutputDebugStringA(hrs);
}

HRESULT doPaint(ID2D1RenderTarget *rt)
{
	COLORREF cref;
	D2D1_COLOR_F color;
	ID2D1SolidColorBrush *brush;
	D2D1_RECT_F rect;
	const struct rect *cr;
	int cc = 0;
	D2D1_ANTIALIAS_MODE curaa, newaa;
	HRESULT hr;

	rt->BeginDraw();

	cref = GetSysColor(COLOR_BTNFACE);
	color.r = ((double) GetRValue(cref)) / 255;
	color.g = ((double) GetGValue(cref)) / 255;
	color.b = ((double) GetBValue(cref)) / 255;
	color.a = 1.0;
	rt->Clear(&color);

	curaa = rt->GetAntialiasMode();
	newaa = D2D1_ANTIALIAS_MODE_ALIASED;
	if (SendMessageW(antialias, BM_GETCHECK, 0, 0) != BST_UNCHECKED)
		newaa = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
	rt->SetAntialiasMode(newaa);

	cr = rectList[SendMessageW(chooser, CB_GETCURSEL, 0, 0)].rects;
	while (cr->x < 50) {
		color.r = fillcolors[cc].r;
		color.g = fillcolors[cc].g;
		color.b = fillcolors[cc].b;
		color.a = fillcolors[cc].a;
		hr = rt->CreateSolidColorBrush(&color, NULL, &brush);
		if (hr != S_OK) {
			loghr("CreateSolidColorBrush()", hr);
			rt->SetAntialiasMode(curaa);
			rt->EndDraw(NULL, NULL);
			return hr;
		}

		rect.left = cr->x;
		rect.top = cr->y;
		rect.right = cr->x + cr->width;
		rect.bottom = cr->y + cr->height;
		rt->FillRectangle(&rect, brush);

		brush->Release();
		cr++;
		if (SendMessageW(sameColor, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
			cc = (cc + 1) % 4;
	}

	rt->SetAntialiasMode(curaa);
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

LRESULT CALLBACK rawndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
	case WM_DESTROY:
		rt->Release();
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hc;

	switch (uMsg) {
	case WM_COMMAND:
		// TODO make this more specific?
		hc = (HWND) lParam;
		if (hc == chooser || hc == antialias || hc == sameColor)
			InvalidateRect(ra, NULL, TRUE);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(int argc, char *argv[])
{
	D2D1_FACTORY_OPTIONS opts;
	WNDCLASSW wc;
	RECT r;
	HWND mainwin;
	COMBOBOXINFO cbi;
	int i;
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

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);
	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"rectArea";
	wc.lpfnWndProc = rawndproc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	r.left = 0;
	r.top = 0;
	r.right = 640;
	r.bottom = 480;
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

	// convert to client
	r.right = 640;
	r.bottom = 480;
	// and set everything up
	r.left = 7;
	r.right -= 7;
	r.top = 7;
	chooser = CreateWindowExW(0,
		L"COMBOBOX", L"",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		r.left, r.top,
		r.right - r.left, 150,
		mainwin, (HMENU) 100, NULL, NULL);
	ZeroMemory(&cbi, sizeof (COMBOBOXINFO));
	cbi.cbSize = sizeof (COMBOBOXINFO);
	SendMessageW(chooser, CB_GETCOMBOBOXINFO, 0, (LPARAM) (&cbi));
	// this I figured out is right (or at least seems to be right) on my own
	GetWindowRect(cbi.hwndCombo, &(cbi.rcItem));
	r.top += (cbi.rcItem.bottom - cbi.rcItem.top);

	antialias = CreateWindowExW(0,
		L"BUTTON", L"Antialias",
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		r.left, r.top,
		(r.right - r.left) / 2, 15,
		mainwin, (HMENU) 101, NULL, NULL);
	SendMessageW(antialias, BM_SETCHECK, BST_CHECKED, 0);

	sameColor = CreateWindowExW(0,
		L"BUTTON", L"Same Color",
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		r.left + (r.right - r.left) / 2, r.top,
		(r.right - r.left) / 2, 15,
		mainwin, (HMENU) 102, NULL, NULL);
	SendMessageW(sameColor, BM_SETCHECK, BST_UNCHECKED, 0);
	r.top += 15;

	r.top += 4;
	r.bottom -= 7;
	ra = CreateWindowExW(0,
		L"rectArea", L"",
		WS_CHILD | WS_VISIBLE,
		r.left, r.top,
		r.right - r.left, r.bottom - r.top,
		mainwin, (HMENU) 103, NULL, NULL);

	for (i = 0; rectList[i].name != NULL; i++)
		SendMessageW(chooser, CB_ADDSTRING, 0, (LPARAM) (rectList[i].name));
	SendMessageW(chooser, CB_SETCURSEL, 0, 0);

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	d2dfactory->Release();
	CoUninitialize();
	return msg.wParam;
}
