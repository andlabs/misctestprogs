// 18 december 2015
#define UNICODE
#define _UNICODE
#define STRICT
#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC dc;
	RECT r;

	if (GetWindowLongPtrW(hwnd, GWLP_USERDATA) == 0) {
		if (uMsg == WM_CREATE) {
			printf("Handling WM_CREATE\n");
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, 1);
		}
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	printf("Message %I32u\n", uMsg);

	dc = GetDC(hwnd);
	if (dc != NULL) {
		GetDeviceCaps(dc, LOGPIXELSX);
		GetDeviceCaps(dc, LOGPIXELSY);
		ReleaseDC(hwnd, dc);
	} else
		printf("GetDC() failed: %I32u\n", GetLastError());

	if (GetClientRect(hwnd, &r) == 0)
		printf("GetClientRect() failed: %I32u\n", GetLastError());

	if (uMsg == WM_CLOSE)
		PostQuitMessage(0);
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
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	RegisterClassW(&wc);

	mainwin = CreateWindowExW(0,
		L"mainwin", L"mainwin",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		320, 240,
		NULL, NULL, NULL, NULL);
	printf("CreateWindowEx() returned\n");

	ShowWindow(mainwin, SW_SHOW);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return 0;
}
