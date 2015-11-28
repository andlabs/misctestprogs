// 28 november 2015
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

LRESULT CALLBACK dlgproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NCACTIVATE)
		printf("WM_NCACTIVATE caught\n");
	if (uMsg == WM_CREATE)
		printf("WM_CREATE\n");
	if (uMsg == WM_NCCREATE)
		printf("WM_NCCREATE\n");
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK aproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_ENABLE)
		printf("WM_ENABLE\n");
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	WNDCLASSW wc;
	HWND a, b;
	MSG msg;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"a";
	wc.lpfnWndProc = aproc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClassW(&wc) == 0)
		abort();
	a = CreateWindowExW(0,
		L"a", L"This should be disabled",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL);

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"b";
	wc.lpfnWndProc = dlgproc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClassW(&wc) == 0)
		abort();
	b = CreateWindowExW(0,
		L"b", L"dlgproc",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		a, NULL, NULL, NULL);

	printf("b created\n");

	if (a == NULL || b == NULL)
		abort();

	EnableWindow(a, FALSE);

	ShowWindow(a, SW_SHOWDEFAULT);
	UpdateWindow(a);
//	ShowWindow(b, SW_SHOWDEFAULT);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
