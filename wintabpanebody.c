// 20 may 2015
// based on wintabprintclient.c 18 may 2015
// based on wintabparentwinebug.c 3 may 2015
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

void die(char *s)
{
	// TODO
}

HWND mainwin;

#define BGCOLOR RGB(0x0A, 0x24, 0x6A)
#define PCOLOR RGB(0x6A, 0x24, 0x0A)

LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NMHDR *nm = (NMHDR *) lParam;
	PAINTSTRUCT ps;
	HDC dc;
	POINT pt;
	RECT r;
	HTHEME theme;

	switch (uMsg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		dc = BeginPaint(hwnd, &ps);
#define X 20
#define Y 20
#define X2 240
#define YT 40
#define WIDTH 200
#define HEIGHT 440
		theme = OpenThemeData(hwnd, L"tab");
		r.left = X;
		r.top = Y;
		r.right = r.left + WIDTH;
		r.bottom = YT - 5;
		DrawTextW(dc, L"TABP_PANE", -1, &r, DT_LEFT | DT_TOP);
		r.left = X;
		r.top = YT;
		r.right = r.left + WIDTH;
		r.bottom = r.top + HEIGHT;
		DrawThemeBackground(theme, dc,
			TABP_PANE, 0,
			&r, NULL);
		r.left = X2;
		r.top = Y;
		r.right = r.left + WIDTH;
		r.bottom = YT - 5;
		DrawTextW(dc, L"TABP_BODY", -1, &r, DT_LEFT | DT_TOP);
		r.left = X2;
		r.top = YT;
		r.right = r.left + WIDTH;
		r.bottom = r.top + HEIGHT;
		DrawThemeBackground(theme, dc,
			TABP_BODY, 0,
			&r, NULL);
		CloseThemeData(theme);
		EndPaint(hwnd, &ps);
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static void makeWindows(void)
{
	mainwin = CreateWindowExW(0,
		L"mainwin", L"Full Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 500,
		NULL, NULL, GetModuleHandle(NULL), NULL);
}

int main(int argc, char *argv[])
{
	WNDCLASSW wc;
	MSG msg;
	HBRUSH b;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	// if printing client doesn't print the tab background, this color will bleed through instead
	b = CreateSolidBrush(BGCOLOR);
	wc.hbrBackground = b;
	RegisterClassW(&wc);

	makeWindows();

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
