// 5 june 2016
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
#include <windows.h>
#include <d2d1.h>
#include <stdio.h>

ID2D1Factory *d2dfactory;

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
	D2D1_COLOR_F color;
	ID2D1SolidColorBrush *brush;
	D2D1_RECT_F rect;
	HRESULT hr;

	rt->BeginDraw();

	color.r = 1.0;
	color.g = 1.0;
	color.b = 1.0;
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

	rect.left = 10;
	rect.top = 10;
	rect.right = 180;
	rect.bottom = 180;
	rt->DrawRectangle(&rect, brush);

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

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	mainwin = CreateWindowExW(0,
		L"mainwin", L"mainwin",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		640, 480,
		NULL, NULL, NULL, NULL);

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
