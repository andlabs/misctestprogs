// 21 march 2015
#define UNICODE
#define _UNICODE
#define STRICT
#define WINVER 0x0600
#define _WIN32_WINNT 0x0600
#define _WIN32_WINDOWS 0x0600
#include <windows.h>
#include <stdio.h>

HWND button;
BOOL enabled = FALSE;

// MinGW-w64 doesn't seem to have these in its user32.lib
// TODO file bug report
BOOL (*WINAPI pShutdownBlockReasonCreate)(HWND hWnd, LPCWSTR pwszReason);
BOOL (*WINAPI pShutdownBlockReasonDestroy)(HWND hWnd);

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && lParam == (LPARAM) button) {
			if (!enabled) {
				printf("create  %d | ", (*pShutdownBlockReasonCreate)(hwnd, L"Test shutdown block message"));
				printf("%d\n", GetLastError());
				enabled = TRUE;
				SetWindowTextW(button, L"Disable");
			} else {
				printf("destroy %d | ", (*pShutdownBlockReasonDestroy)(hwnd));
				printf("%d\n", GetLastError());
				enabled = FALSE;
				SetWindowTextW(button, L"Enable");
			}
			return 0;
		}
		break;
	case WM_QUERYENDSESSION:
		if (enabled)
			return FALSE;
		break;
	case WM_CLOSE:
		if (!enabled)
			PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	WNDCLASSW wc;
	HWND mainwin;
	MSG msg;
	HANDLE user32;

	user32 = LoadLibraryW(L"user32.dll");
	pShutdownBlockReasonCreate = (BOOL (*WINAPI)(HWND hWnd, LPCWSTR pwszReason)) GetProcAddress(user32, "ShutdownBlockReasonCreate");
	pShutdownBlockReasonDestroy = (BOOL (*WINAPI)(HWND hWnd)) GetProcAddress(user32, "ShutdownBlockReasonDestroy");
	printf("%p %p\n", pShutdownBlockReasonCreate, pShutdownBlockReasonDestroy);

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	mainwin = CreateWindowExW(0,
		L"mainwin", L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		200, 200,
		NULL, NULL, GetModuleHandle(NULL), NULL);
	button = CreateWindowExW(0,
		L"button", L"Enable",
		BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
		20, 20, 140, 140,
		mainwin, NULL, GetModuleHandle(NULL), NULL);

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
