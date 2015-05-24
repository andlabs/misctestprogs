// 24 may 2015
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

#define frameClass L"test_frame_class"
#define childClass L"test_child_class"

HWND frame;
HWND mdiClient;

LRESULT CALLBACK frameWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		if (lParam != 0)
			break;
		if (HIWORD(wParam) != 0)
			break;
		if (LOWORD(wParam) == 100)
			CreateWindowExW(WS_EX_MDICHILD,
				childClass, L"test child",
				WS_CHILD | WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME,
				CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT,
				mdiClient, NULL, GetModuleHandle(NULL), NULL);
		if (LOWORD(wParam) == 101)
			PostQuitMessage(0);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}
	return DefFrameProcW(hwnd, mdiClient, uMsg, wParam, lParam);
}

void registerClasses(void)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = frameClass;
	wc.lpfnWndProc = frameWndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = childClass;
	wc.lpfnWndProc = DefMDIChildProcW;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);
}

void makeWindows(void)
{
	CLIENTCREATESTRUCT ccs;
	HMENU menubar;
	HMENU fileMenu;
	HMENU windowMenu;

	frame = CreateWindowExW(0,
		frameClass, L"Frame",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, GetModuleHandle(NULL), NULL);

	menubar = CreateMenu();
	fileMenu = CreatePopupMenu();
	AppendMenuW(menubar, MF_POPUP | MF_STRING, (UINT_PTR) fileMenu, L"File");
	AppendMenuW(fileMenu, MF_STRING, 100, L"New");
	AppendMenuW(fileMenu, MF_STRING, 101, L"Quit");
	windowMenu = CreatePopupMenu();
	AppendMenuW(menubar, MF_POPUP | MF_STRING, (UINT_PTR) windowMenu, L"Window");
	SetMenu(frame, menubar);

	ZeroMemory(&ccs, sizeof (CLIENTCREATESTRUCT));
	ccs.hWindowMenu = windowMenu;
	ccs.idFirstChild = 500;

	mdiClient = CreateWindowExW(0,
		L"mdiclient", L"",
		WS_CHILD | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE | MDIS_ALLCHILDSTYLES,
		0, 0, 0, 0,
		frame, NULL, GetModuleHandle(NULL), &ccs);
}

int main(void)
{
	MSG msg;

	registerClasses();
	makeWindows();
	ShowWindow(frame, SW_SHOWDEFAULT);
	UpdateWindow(frame);
	while (GetMessageW(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
