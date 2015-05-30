// 30 may 2015
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

HWND mainwin;
HWND dialog;

LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(int argc, char *argv[])
{
	WNDCLASSW wc;
	MSG msg;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	mainwin = CreateWindowExW(0,
		L"mainwin", L"mainwin",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		400, 400,
		NULL, NULL, GetModuleHandle(NULL), NULL);

	dialog = CreateWindowExW(WS_EX_CONTROLPARENT,
		WC_DIALOG, L"",
		WS_CHILD | WS_VISIBLE,
		100, 100, 200, 200,
		mainwin, NULL, GetModuleHandle(NULL), NULL);
	printf("%I32X\n", EnableThemeDialogTexture(dialog, ETDT_ENABLE | ETDT_USETABTEXTURE | ETDT_ENABLETAB));

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
