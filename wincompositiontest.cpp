// 12 december 2016
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
// get Windows version right; right now Windows Vista
// unless otherwise stated, all values from Microsoft's sdkddkver.h
// TODO is all of this necessary? how is NTDDI_VERSION used?
// TODO plaform update sp2
#define WINVER			0x0600	/* from Microsoft's winnls.h */
#define _WIN32_WINNT		0x0600
#define _WIN32_WINDOWS	0x0600	/* from Microsoft's pdh.h */
#define _WIN32_IE			0x0700
#define NTDDI_VERSION		0x06000000
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <windowsx.h>
#include <shobjidl.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <usp10.h>
#include <msctf.h>
#include <textstor.h>
#include <olectl.h>
#include <shlwapi.h>
#include <dwmapi.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <inttypes.h>
#include <vector>
#include <map>
#include <string>

#pragma comment(linker, \
	"\"/manifestdependency:type='Win32' " \
	"name='Microsoft.Windows.Common-Controls' " \
	"version='6.0.0.0' " \
	"processorArchitecture='*' " \
	"publicKeyToken='6595b64144ccf1df' " \
	"language='*'\"")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dwmapi.lib")

#define HR(call) printf("%s -> 0x%I32X\n", #call, call)

// TODO this is incorrect when maximized
MARGINS computeMargins(HWND hwnd)
{
	RECT r;
	MARGINS m;

	ZeroMemory(&r, sizeof (RECT));
	AdjustWindowRectEx(&r,
		GetWindowStyle(hwnd),
		FALSE,
		GetWindowExStyle(hwnd));

	ZeroMemory(&m, sizeof (MARGINS));
	m.cxLeftWidth = abs(r.left);
	m.cxRightWidth = r.right;
	// give it 2.5x the room so we can shove stuff in there
	m.cyTopHeight = abs(r.top) * 2.5;
	m.cyBottomHeight = r.bottom;
	return m;
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	MARGINS margins;
	BOOL dwmHandled;
	LRESULT lResult;

	dwmHandled = DwmDefWindowProc(hwnd, uMsg, wParam, lParam, &lResult);
	switch (uMsg) {
	case WM_CREATE:
		GetWindowRect(hwnd, &r);
		SetWindowPos(hwnd, NULL,
			r.left, r.top,
			r.right - r.left, r.bottom - r.top,
			SWP_FRAMECHANGED);
		// TODO if we pass SWP_NOOWNERZORDER || SWP_NOZORDER, the default frame is not correctly inhibited
		break;
	case WM_ACTIVATE:
		margins = computeMargins(hwnd);
		HR(DwmExtendFrameIntoClientArea(hwnd, &margins));
		break;
	case WM_NCCALCSIZE:
		if (wParam != (WPARAM) FALSE)
			return 0;
		break;
	case WM_NCHITTEST:
		if (dwmHandled)
			return lResult;
		// DWM did not handle it; we have to do it ourselves
		{
			RECT r, frame;
			POINT p;
			MARGINS m;

			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);
			GetWindowRect(hwnd, &r);

			ZeroMemory(&frame, sizeof (RECT));
			AdjustWindowRectEx(&frame,
				GetWindowStyle(hwnd) & ~WS_CAPTION,
				FALSE,
				GetWindowExStyle(hwnd));

			m = computeMargins(hwnd);

			lResult = HTNOWHERE;
			if (p.y >= r.top && p.y < (r.top + m.cyTopHeight))
				lResult = HTTOP;
			else if (p.y >= (r.bottom - m.cyBottomHeight) && p.y < r.bottom)
				lResult = HTBOTTOM;

			if (p.x >= r.left && p.x < (r.left + m.cxLeftWidth))
				switch (lResult) {
				case HTNOWHERE:
					lResult = HTLEFT;
					break;
				case HTTOP:
					lResult = HTTOPLEFT;
					break;
				case HTBOTTOM:
					lResult = HTBOTTOMLEFT;
					break;
				}
			else if (p.x >= (r.right - m.cxRightWidth) && p.x < r.right)
				switch (lResult) {
				case HTNOWHERE:
					lResult = HTRIGHT;
					break;
				case HTTOP:
					lResult = HTTOPRIGHT;
					break;
				case HTBOTTOM:
					lResult = HTBOTTOMRIGHT;
					break;
				}

			if (lResult == HTTOP)
				if (p.y >= (r.top - frame.top))
					lResult = HTCAPTION;

			if (lResult != HTNOWHERE)
				return lResult;
		}
		// we can't handle it; give it to DefWindowProcW() and hope for the best
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}
	if (dwmHandled)
		return lResult;
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	WNDCLASSW wc;
	HWND mainwin;
	MSG msg;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	RegisterClassW(&wc);

	mainwin = CreateWindowExW(0,
		L"mainwin", L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		400, 400,
		NULL, NULL, GetModuleHandle(NULL), NULL);

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return 0;
}
