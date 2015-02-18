// 17 february 2015
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define CINTERFACE
#define COBJMACROS
// get Windows version right; right now Windows XP
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_WINDOWS 0x0501		/* according to Microsoft's winperf.h */
#define _WIN32_IE 0x0600			/* according to Microsoft's sdkddkver.h */
#define NTDDI_VERSION 0x05010000	/* according to Microsoft's sdkddkver.h */
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

HWND dtp;
BOOL returnColors = FALSE;

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
#define CTLCOLOR(which) \
	case which: \
		printf("%s %p\n", #which, (HWND) lParam); \
		if (returnColors) { \
			SetBkColor((HDC) wParam, GetSysColor(COLOR_ACTIVECAPTION)); \
			return (LRESULT) GetSysColorBrush(COLOR_GRADIENTACTIVECAPTION); \
		} \
		break; /* fall through to DefWindowProc() */
	CTLCOLOR(WM_CTLCOLORMSGBOX)
	CTLCOLOR(WM_CTLCOLOREDIT)
	CTLCOLOR(WM_CTLCOLORLISTBOX)
	CTLCOLOR(WM_CTLCOLORBTN)
	CTLCOLOR(WM_CTLCOLORDLG)
	CTLCOLOR(WM_CTLCOLORSCROLLBAR)
	CTLCOLOR(WM_CTLCOLORSTATIC)
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main(int argc, char *argv[])
{
	INITCOMMONCONTROLSEX icc;
	WNDCLASSW wc;
	HWND mainwin;
	MSG msg;

	returnColors = argc > 1;

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_DATE_CLASSES;
	if (InitCommonControlsEx(&icc) == 0) {
		fprintf(stderr, "InitCommonControlsEx() failed: %I32u\n", GetLastError());
		return 1;
	}

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.hInstance = GetModuleHandle(NULL);
	if (RegisterClassW(&wc) == 0) {
		fprintf(stderr, "RegisterClassW() failed: %I32u\n", GetLastError());
		return 1;
	}

	mainwin = CreateWindowExW(0,
		L"mainwin", L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		320, 240,
		NULL, NULL, GetModuleHandle(NULL), NULL);
	if (mainwin == NULL) {
		fprintf(stderr, "create main window failed: %I32u", GetLastError());
		return 1;
	}

	dtp = CreateWindowExW(0,
		DATETIMEPICK_CLASSW, L"",
		DTS_LONGDATEFORMAT | WS_CHILD | WS_VISIBLE,
		20, 20, 200, 180,
		mainwin, NULL, GetModuleHandle(NULL), NULL);
	if (dtp == NULL) {
		fprintf(stderr, "create date-time picker failed: %I32u\n", GetLastError());
		return 1;
	}
	printf("dtp %p\n", dtp);

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
