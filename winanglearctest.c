// 8 september 2015
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

HPEN pen;

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC dc;
	PAINTSTRUCT ps;
	HPEN prevpen;

	switch (uMsg) {
	case WM_PAINT:
		dc = BeginPaint(hwnd, &ps);
		prevpen = SelectObject(dc, pen);
		SetDCPenColor(dc, RGB(0, 0, 0));

		BeginPath(dc);
		MoveToEx(dc, 100, 100, NULL);
		AngleArc(dc, 100, 100, 50, 30, 300);
		LineTo(dc, 100, 100);
		EndPath(dc);
		StrokePath(dc);

		BeginPath(dc);
		AngleArc(dc, 210, 100, 50, 30, 300);
		CloseFigure(dc);
		EndPath(dc);
		StrokePath(dc);

		SelectObject(dc, prevpen);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	HINSTANCE hInstance;
	WNDCLASSW wc;
	HWND mainwin;
	MSG msg;

	hInstance = GetModuleHandle(NULL);
	pen = GetStockObject(DC_PEN);

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	mainwin = CreateWindowExW(0,
		L"mainwin", L"mainwin",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		400, 400,
		NULL, NULL, hInstance, NULL);

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return 0;
}
