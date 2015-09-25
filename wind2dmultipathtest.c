// 24 september 2015
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define CINTERFACE
#define COBJMACROS
// see https://github.com/golang/go/issues/9916#issuecomment-74812211
#define INITGUID
#define _USE_MATH_DEFINES
// get Windows version right; right now Windows Vista
#define WINVER 0x0600				/* according to Microsoft's winnls.h */
#define _WIN32_WINNT 0x0600		/* according to Microsoft's sdkddkver.h */
#define _WIN32_WINDOWS 0x0600		/* according to Microsoft's pdh.h */
#define _WIN32_IE 0x0700			/* according to Microsoft's sdkddkver.h */
#define NTDDI_VERSION 0x06000000	/* according to Microsoft's sdkddkver.h */
#include <windows.h>
#include <commctrl.h>
#include <stdint.h>
#include <uxtheme.h>
#include <string.h>
#include <wchar.h>
#include <windowsx.h>
#include <vsstyle.h>
#include <vssym32.h>
#include <stdarg.h>
#include <oleacc.h>
#include <stdio.h>
#include <shobjidl.h>
#include <math.h>
#include <d2d1.h>

static ID2D1Factory *d2dfactory = NULL;

HRESULT initDraw(void)
{
	D2D1_FACTORY_OPTIONS opts;

	ZeroMemory(&opts, sizeof (D2D1_FACTORY_OPTIONS));
	// TODO make this an option
	opts.debugLevel = D2D1_DEBUG_LEVEL_NONE;
	return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&IID_ID2D1Factory,
		&opts,
		(void **) (&d2dfactory));
}

void uninitDraw(void)
{
	ID2D1Factory_Release(d2dfactory);
}

ID2D1HwndRenderTarget *makeHWNDRenderTarget(HWND hwnd)
{
	D2D1_RENDER_TARGET_PROPERTIES props;
	D2D1_HWND_RENDER_TARGET_PROPERTIES hprops;
	HDC dc;
	RECT r;
	ID2D1HwndRenderTarget *rt;

	// we need a DC for the DPI
	// we *could* just use the screen DPI but why when we have a window handle and its DC has a DPI
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
	hprops.presentOptions = D2D1_PRESENT_OPTIONS_NONE;

	ID2D1Factory_CreateHwndRenderTarget(d2dfactory,
		&props,
		&hprops,
		&rt);
	return rt;
}

void initdraw(void);
void uninitdraw(void);
HRESULT draw(ID2D1RenderTarget *);

ID2D1HwndRenderTarget *rt = NULL;

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr;
	RECT client;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	D2D1_SIZE_U size;

	switch (uMsg) {
	case WM_PAINT:
		if (rt == NULL)
			rt = makeHWNDRenderTarget(hwnd);
		hr = draw((ID2D1RenderTarget *) rt);
		switch (hr) {
		case S_OK:
			ValidateRect(hwnd, NULL);
			break;
		case D2DERR_RECREATE_TARGET:
			// DON'T validate the rect
			// instead, simply drop the render target
			// we'll get another WM_PAINT and make the render target again
			ID2D1HwndRenderTarget_Release(rt);
			rt = NULL;
			break;
		}
		return 0;
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOSIZE) != 0)
			break;
		if (rt == NULL)
			rt = makeHWNDRenderTarget(hwnd);
		else {
			GetClientRect(hwnd, &client);
			size.width = client.right - client.left;
			size.height = client.bottom - client.top;
			// don't track the error; we'll get that in EndDraw()
			// see https://msdn.microsoft.com/en-us/library/windows/desktop/dd370994%28v=vs.85%29.aspx
			ID2D1HwndRenderTarget_Resize(rt, &size);
		}
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	WNDCLASSW wc;
	HWND mainwin;
	MSG msg;

	CoInitialize(NULL);
	initDraw();
	initdraw();

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	mainwin = CreateWindowExW(0,
		L"mainwin", L"mainwin",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, GetModuleHandle(NULL), NULL);
	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	if (rt != NULL)
		ID2D1HwndRenderTarget_Release(rt);
	uninitdraw();
	uninitDraw();
	CoUninitialize();
	return 0;
}

ID2D1EllipseGeometry *topleft[4];
ID2D1GeometryGroup *topleftGroup;
ID2D1EllipseGeometry *topright[4];
ID2D1GeometryGroup *toprightGroup;
ID2D1PathGeometry *bottomleft;
ID2D1PathGeometry *bottomright;

ID2D1EllipseGeometry *mkcircle(int x, int y, int rad)
{
	D2D1_ELLIPSE e;
	ID2D1EllipseGeometry *eg;

	e.point.x = x;
	e.point.y = y;
	e.radiusX = rad;
	e.radiusY = rad;
	ID2D1Factory_CreateEllipseGeometry(d2dfactory,
		&e,
		&eg);
	return eg;
}

void addCircle(ID2D1GeometrySink *sink, int x, int y, int rad)
{
	D2D1_POINT_2F begin;
	D2D1_ARC_SEGMENT a;

	begin.x = x + rad;
	begin.y = y;
	ID2D1GeometrySink_BeginFigure(sink,
		begin,
		D2D1_FIGURE_BEGIN_FILLED);

	// TODO why doesn't a single arc work?

	a.point.x = x - rad;
	a.point.y = y;
	a.size.width = rad;
	a.size.height = rad;
	a.rotationAngle = 180;
	a.sweepDirection = D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
	a.arcSize = D2D1_ARC_SIZE_LARGE;
	ID2D1GeometrySink_AddArc(sink,
		&a);

	a.point.x = x + rad;
	a.point.y = y;
	a.size.width = rad;
	a.size.height = rad;
	a.rotationAngle = 180;
	a.sweepDirection = D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
	a.arcSize = D2D1_ARC_SIZE_LARGE;
	ID2D1GeometrySink_AddArc(sink,
		&a);

	ID2D1GeometrySink_EndFigure(sink,
		D2D1_FIGURE_END_CLOSED);
}

void initdraw(void)
{
	ID2D1GeometrySink *sink;

	topleft[0] = mkcircle(120, 120, 25);
	topleft[1] = mkcircle(120, 120, 50);
	topleft[2] = mkcircle(120, 120, 75);
	topleft[3] = mkcircle(120, 120, 100);
	ID2D1Factory_CreateGeometryGroup(d2dfactory,
		D2D1_FILL_MODE_ALTERNATE,
		(ID2D1Geometry **) topleft,
		4,
		&topleftGroup);

	topright[0] = mkcircle(340, 120, 25);
	topright[1] = mkcircle(340, 120, 50);
	topright[2] = mkcircle(340, 120, 75);
	topright[3] = mkcircle(340, 120, 100);
	ID2D1Factory_CreateGeometryGroup(d2dfactory,
		D2D1_FILL_MODE_WINDING,
		(ID2D1Geometry **) topright,
		4,
		&toprightGroup);

	ID2D1Factory_CreatePathGeometry(d2dfactory,
		&bottomleft);
	ID2D1PathGeometry_Open(bottomleft,
		&sink);
	ID2D1GeometrySink_SetFillMode(sink,
		D2D1_FILL_MODE_ALTERNATE);
	addCircle(sink, 120, 340, 25);
	addCircle(sink, 120, 340, 50);
	addCircle(sink, 120, 340, 75);
	addCircle(sink, 120, 340, 100);
	ID2D1GeometrySink_Close(sink);

	ID2D1Factory_CreatePathGeometry(d2dfactory,
		&bottomright);
	ID2D1PathGeometry_Open(bottomright,
		&sink);
	ID2D1GeometrySink_SetFillMode(sink,
		D2D1_FILL_MODE_WINDING);
	addCircle(sink, 340, 340, 25);
	addCircle(sink, 340, 340, 50);
	addCircle(sink, 340, 340, 75);
	addCircle(sink, 340, 340, 100);
	ID2D1GeometrySink_Close(sink);
}

void uninitdraw(void)
{
	ID2D1PathGeometry_Release(bottomright);

	ID2D1PathGeometry_Release(bottomleft);

	ID2D1GeometryGroup_Release(toprightGroup);
	ID2D1EllipseGeometry_Release(topright[3]);
	ID2D1EllipseGeometry_Release(topright[2]);
	ID2D1EllipseGeometry_Release(topright[1]);
	ID2D1EllipseGeometry_Release(topright[0]);

	ID2D1GeometryGroup_Release(topleftGroup);
	ID2D1EllipseGeometry_Release(topleft[3]);
	ID2D1EllipseGeometry_Release(topleft[2]);
	ID2D1EllipseGeometry_Release(topleft[1]);
	ID2D1EllipseGeometry_Release(topleft[0]);
}

HRESULT draw(ID2D1RenderTarget *rt)
{
	COLORREF rgb;
	D2D1_COLOR_F color;
	ID2D1SolidColorBrush *fillBrush, *drawBrush;

	ID2D1RenderTarget_BeginDraw(rt);

	color.r = 0.82;
	color.g = 0.86;
	color.b = 0.96;
	color.a = 1.0;
	ID2D1RenderTarget_CreateSolidColorBrush(rt,
		&color,
		NULL,
		&fillBrush);

	color.r = 0.25;
	color.g = 0.39;
	color.b = 0.69;
	color.a = 1.0;
	ID2D1RenderTarget_CreateSolidColorBrush(rt,
		&color,
		NULL,
		&drawBrush);

	rgb = GetSysColor(COLOR_BTNFACE);
	color.r = ((float) (rgb & 0xFF)) / 255;
	color.g = ((float) ((rgb & 0xFF00) >> 8)) / 255;
	color.b = ((float) ((rgb & 0xFF0000) >> 16)) / 255;
	color.a = 1.0;
	ID2D1RenderTarget_Clear(rt, &color);

	ID2D1RenderTarget_FillGeometry(rt,
		(ID2D1Geometry *) topleftGroup,
		(ID2D1Brush *) fillBrush,
		NULL);
	ID2D1RenderTarget_DrawGeometry(rt,
		(ID2D1Geometry *) topleftGroup,
		(ID2D1Brush *) drawBrush,
		1.0,
		NULL);

	ID2D1RenderTarget_FillGeometry(rt,
		(ID2D1Geometry *) toprightGroup,
		(ID2D1Brush *) fillBrush,
		NULL);
	ID2D1RenderTarget_DrawGeometry(rt,
		(ID2D1Geometry *) toprightGroup,
		(ID2D1Brush *) drawBrush,
		1.0,
		NULL);

	ID2D1RenderTarget_FillGeometry(rt,
		(ID2D1Geometry *) bottomleft,
		(ID2D1Brush *) fillBrush,
		NULL);
	ID2D1RenderTarget_DrawGeometry(rt,
		(ID2D1Geometry *) bottomleft,
		(ID2D1Brush *) drawBrush,
		1.0,
		NULL);

	ID2D1RenderTarget_FillGeometry(rt,
		(ID2D1Geometry *) bottomright,
		(ID2D1Brush *) fillBrush,
		NULL);
	ID2D1RenderTarget_DrawGeometry(rt,
		(ID2D1Geometry *) bottomright,
		(ID2D1Brush *) drawBrush,
		1.0,
		NULL);

	ID2D1SolidColorBrush_Release(fillBrush);
	ID2D1SolidColorBrush_Release(drawBrush);

	return ID2D1RenderTarget_EndDraw(rt, NULL, NULL);
}
