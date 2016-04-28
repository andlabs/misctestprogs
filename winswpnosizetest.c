// 28 april 2016
#define UNICODE
#define _UNICODE
#define STRICT
#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	RECT r;
	LRESULT res;

	switch (uMsg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, TRUE);
		GetWindowRect(hwnd, &r);
		printf("= %d\n", SetWindowPos(hwnd, NULL,
			r.left, r.top,
			r.right - r.left, r.bottom - r.top,
			SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER));
		printf("- %d\n", GetLastError());
		break;
	case WM_WINDOWPOSCHANGING:
		printf("< 0x%X\n", wp->flags);
		res = DefWindowProcW(hwnd, uMsg, wParam, lParam);
		printf("> 0x%X %d\n", wp->flags, res);
		return res;
	case WM_WINDOWPOSCHANGED:
		printf("0x%X\n", wp->flags);
		break;
	case WM_GETMINMAXINFO:
		printf("gmmi\n");
		if (GetWindowLongPtrW(hwnd, GWLP_USERDATA))
			((MINMAXINFO *) lParam)->ptMinTrackSize.y = 600;
		break;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	WNDCLASSW wc;
	HWND hwnd;
	MSG msg;

	printf("0x%X\n", SWP_NOSIZE);

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"class";
	wc.lpfnWndProc = wndproc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	hwnd = CreateWindowExW(0,
		L"class", L"window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		300, 300,
		NULL, NULL, NULL, NULL);

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	SetTimer(hwnd, 1, 3000, NULL);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return 0;
}
