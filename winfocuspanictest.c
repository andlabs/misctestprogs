// 5 june 2014
// scratch Windows program by pietro gagliardi 17 april 2014
// fixed typos and added toWideString() 1 may 2014
// borrows code from the scratch GTK+ program (16-17 april 2014) and from code written 31 march 2014 and 11-12 april 2014
#define _UNICODE
#define UNICODE
#define STRICT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>

HMODULE hInstance;
HICON hDefaultIcon;
HCURSOR hDefaultCursor;
HFONT controlfont;

void panic(char *fmt, ...);
TCHAR *toWideString(char *what);

HWND entry;
HWND checkbox;
HWND lastfocus = NULL;

void onActivate(HWND hwnd, WPARAM wParam)
{
	UINT state = (UINT) LOWORD(wParam);
	int minimized = HIWORD(wParam) != 0;

	if (minimized)
		return;
	if (state == WA_INACTIVE) {
		HWND old;

		old = GetFocus();
		if (old != NULL && IsChild(hwnd, old))
			lastfocus = old;
	} else {
		int shouldpanic;
		HWND result;

		if (lastfocus == NULL)
			return;
		shouldpanic = SendMessage(checkbox, BM_GETCHECK, 0, 0) == BST_CHECKED;
		SetLastError(0);		// just in case
		result = SetFocus(lastfocus);
		if (shouldpanic && result == NULL)
			panic("SetFocus() failed");
		printf("result %p new focus %p\n", result, GetFocus());
	}
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_ACTIVATE:
		onActivate(hwnd, wParam);
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	panic("oops: message %ud does not return anything; bug in wndproc()", msg);
}

HWND makeMainWindow(void)
{
	WNDCLASS cls;
	HWND hwnd;
	RECT r;

	ZeroMemory(&cls, sizeof (WNDCLASS));
	cls.lpszClassName = L"mainwin";
	cls.lpfnWndProc = wndproc;
	cls.hInstance = hInstance;
	cls.hIcon = hDefaultIcon;
	cls.hCursor = hDefaultCursor;
	cls.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClass(&cls) == 0)
		panic("error registering window class");
	r.left = 0;
	r.top = 0;
	r.right = 320;
	r.bottom = 70;
	if (AdjustWindowRectEx(&r, WS_OVERLAPPEDWINDOW, FALSE, 0) == 0)
		panic("AdjustWindowRectEx() failed");
	hwnd = CreateWindowEx(0,
		L"mainwin", L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		r.right - r.left, r.bottom - r.top,
		NULL, NULL, hInstance, NULL);
	if (hwnd == NULL)
		panic("opening main window failed");
	return hwnd;
}

void buildUI(HWND mainwin)
{
#define CSTYLE (WS_CHILD | WS_VISIBLE)
#define CXSTYLE (0)
#define SETFONT(hwnd) SendMessage(hwnd, WM_SETFONT, (WPARAM) controlfont, (LPARAM) TRUE);

	entry = CreateWindowEx(WS_EX_CLIENTEDGE | CXSTYLE,
		L"edit", L"Click here to focus",
		CSTYLE,
		10, 10, 300, 20,
		mainwin, (HMENU) 100, hInstance, NULL);
	if (entry == NULL)
		panic("error creating entry");
	SETFONT(entry);

	checkbox = CreateWindowEx(CXSTYLE,
		L"button", L"Panic",
		BS_AUTOCHECKBOX | CSTYLE,
		10, 40, 300, 20,
		mainwin, (HMENU) 101, hInstance, NULL);
	if (checkbox == NULL)
		panic("error creating checkbox");
	SETFONT(checkbox);
}

void initwin(void);
void firstShowWindow(HWND hwnd);

int main(int argc, char *argv[])
{
	HWND mainwin;
	MSG msg;

	initwin();

	mainwin = makeMainWindow();
	buildUI(mainwin);
	firstShowWindow(mainwin);

	for (;;) {
		BOOL gmret;

		gmret = GetMessage(&msg, NULL, 0, 0);
		if (gmret == -1)
			panic("error getting message");
		if (gmret == 0)
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

void initwin(void)
{
	NONCLIENTMETRICS ncm;

	hInstance = GetModuleHandle(NULL);
	if (hInstance == NULL)
		panic("error getting hInstance");
	hDefaultIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	if (hDefaultIcon == NULL)
		panic("error getting default window class icon");
	hDefaultCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	if (hDefaultCursor == NULL)
		panic("error getting default window cursor");
	ncm.cbSize = sizeof (NONCLIENTMETRICS);
	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
		sizeof (NONCLIENTMETRICS), &ncm, 0) == 0)
		panic("error getting non-client metrics for getting control font");
	controlfont = CreateFontIndirect(&ncm.lfMessageFont);
	if (controlfont == NULL)
		panic("error getting control font");
}

void panic(char *fmt, ...)
{
	char *msg;
	TCHAR *lerrmsg;
	char *fullmsg;
	va_list arg;
	DWORD lasterr;
	DWORD lerrsuccess;

	lasterr = GetLastError();
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	// according to http://msdn.microsoft.com/en-us/library/windows/desktop/ms680582%28v=vs.85%29.aspx
	lerrsuccess = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, lasterr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lerrmsg, 0, NULL);
	if (lerrsuccess == 0) {
		fprintf(stderr, "critical error: FormatMessage() failed in panic(): last error in panic(): %ld, last error from FormatMessage(): %ld\n", lasterr, GetLastError());
		abort();
	}
	fwprintf(stderr, L"\nlast error: %s\n", lerrmsg);
	va_end(arg);
	exit(1);
}

void firstShowWindow(HWND hwnd)
{
	// we need to get nCmdShow
	int nCmdShow;
	STARTUPINFO si;

	nCmdShow = SW_SHOWDEFAULT;
	GetStartupInfo(&si);
	if ((si.dwFlags & STARTF_USESHOWWINDOW) != 0)
		nCmdShow = si.wShowWindow;
	ShowWindow(hwnd, nCmdShow);
	if (UpdateWindow(hwnd) == 0)
		panic("UpdateWindow(hwnd) failed in first show");
}
