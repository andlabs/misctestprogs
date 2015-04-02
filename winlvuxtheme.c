// 2 april 2015
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define CINTERFACE
// get Windows version right; right now Windows XP
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_WINDOWS 0x0501		/* according to Microsoft's winperf.h */
#define _WIN32_IE 0x0600			/* according to Microsoft's sdkddkver.h */
#define NTDDI_VERSION 0x05010000	/* according to Microsoft's sdkddkver.h */
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

void die(char *why)
{
	fprintf(stderr, "error %s: %I32u\n", why, GetLastError());
	abort();
}

void diehr(char *why, HRESULT hr)
{
	fprintf(stderr, "error %s: 0x%I32X", why, hr);
	if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
		fprintf(stderr, " (windows %I32u)", (DWORD) HRESULT_CODE(hr));
	fprintf(stderr, "\n");
//TODO	abort();
}

#define itemText L"This is the item text that will be drawn"
#define xoff 20
#define yoff 20

int drawItem(HDC dc, HTHEME theme, int ypos, int state)
{
	SIZE size;
	RECT r;
	HRESULT hr;

fprintf(stderr, "%d\n", state);
	hr = GetThemePartSize(theme, dc,
		LVP_LISTITEM, state,
		NULL, TS_DRAW, &size);
	if (hr != S_OK)
		diehr("getting theme part size", hr);
	r.left = xoff;
	r.top = ypos;
size.cx=150;
size.cy=18;
	r.right = r.left + size.cx;
	r.bottom = r.top + size.cy;
	hr = DrawThemeBackground(theme, dc,
		LVP_LISTITEM, state,
		&r, NULL);
	if (hr != S_OK)
		diehr("drawing theme background", hr);
	hr = DrawThemeText(theme, dc,
		LVP_LISTITEM, state,
		itemText, -1,
		DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE,
		0, &r);
	if (hr != S_OK)
		diehr("drawing theme text", hr);
	return ypos + size.cy;
}

void draw(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC dc;
	HTHEME theme;
	int ypos;
	HRESULT hr;

	dc = BeginPaint(hwnd, &ps);
	if (dc == NULL)
		die("beginning paint");

	// TODO really listview?
	theme = OpenThemeData(hwnd, L"listview");
	if (theme == NULL)
		die("opening theme");

	ypos = yoff;
#define DO(x) fprintf(stderr, "%s %d\n", #x , x); ypos = drawItem(dc, theme, ypos, x)
	DO(LISS_DISABLED);
	DO(LISS_HOT);
	DO(LISS_HOTSELECTED);
	DO(LISS_NORMAL);
	DO(LISS_SELECTED);
	DO(LISS_SELECTEDNOTFOCUS);

	hr = CloseThemeData(theme);
	if (hr != S_OK)
		diehr("closing theme", hr);
	EndPaint(hwnd, &ps);
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_PAINT:
		draw(hwnd);
		return 0;
	case WM_THEMECHANGED:
		if (InvalidateRect(hwnd, NULL, TRUE) == 0)
			die("redrawing on theme change");
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
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
	if (hInstance == NULL)
		die("getting program HINSTANCE");

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	if (wc.hIcon == NULL)
		die("getting default icon");
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	if (wc.hCursor == NULL)
		die("getting default cursor");
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClassW(&wc) == 0)
		die("registering window class");

	mainwin = CreateWindowExW(0,
		L"mainwin", L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);
	if (mainwin == NULL)
		die("opening main window");

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	if (UpdateWindow(mainwin) == 0)
		die("opening main window for the first time");

	while (GetMessageW(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
