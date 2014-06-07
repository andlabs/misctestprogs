// 7 june 2014
// based on winfocuspanictest 5 june 2014
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
#include <commctrl.h>

HMODULE hInstance;
HICON hDefaultIcon;
HCURSOR hDefaultCursor;
HFONT controlfont;

void panic(char *fmt, ...);
TCHAR *toWideString(char *what);

HWND pbar;
HWND entry;
BOOL marquee = FALSE;

#define GETSTYLE() (GetWindowLongPtr(pbar, GWL_STYLE))
#define SETSTYLE(x) (SetWindowLongPtr(pbar, GWL_STYLE, (x)))

void marqueeOff(void)
{
	if (marquee) {
		SendMessage(pbar, PBM_SETMARQUEE, FALSE, 0);
		SETSTYLE(GETSTYLE() & ~PBS_MARQUEE);
		marquee = FALSE;
	}
}

int getValue(HWND hwnd)
{
	WCHAR *str;
	LRESULT n;

	n = SendMessage(entry, WM_GETTEXTLENGTH, 0, 0);
	str = (WCHAR *) malloc((n + 1) * sizeof (WCHAR));
	if (str == NULL)
		panic("error allocating %d bytes for entry text", n);
	SendMessage(entry, WM_GETTEXT, (WPARAM) (n + 1), (LPARAM) str);
	return _wtoi(str);
}

#define ID_SET 100
#define ID_MARQUEE 101
#define LASTBUTTON 102

LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int newpos;

	switch (msg) {
	case WM_COMMAND:
		if ((HIWORD(wParam) != BN_CLICKED) ||
			(LOWORD(wParam) >= LASTBUTTON))
			return DefWindowProc(hwnd, msg, wParam, lParam);
		switch (LOWORD(wParam)) {
		case ID_SET:
			newpos = getValue(entry);
			if (newpos < 0 || newpos > 100) {
				MessageBeep(0);
				return DefWindowProc(hwnd, msg, wParam, lParam);
			}
			marqueeOff();
			SendMessage(pbar, PBM_SETPOS, (WPARAM) newpos, 0);
			return 0;
		case ID_MARQUEE:
			SETSTYLE(GETSTYLE() | PBS_MARQUEE);
			SendMessage(pbar, PBM_SETMARQUEE, TRUE, 0);
			marquee = TRUE;
			return 0;
		}
		break;		// fall through to panic
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
	r.bottom = 100;
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

	HWND hwnd;

	pbar = CreateWindowEx(WS_EX_CLIENTEDGE | CXSTYLE,
		PROGRESS_CLASS, L"",
		PBS_SMOOTH | CSTYLE,
		10, 10, 300, 20,
		mainwin, NULL, hInstance, NULL);
	if (pbar == NULL)
		panic("error creating progress bar");
	SendMessage(pbar, PBM_SETRANGE32, 0, 100);

	entry = CreateWindowEx(WS_EX_CLIENTEDGE | CXSTYLE,
		L"edit", L"",
		ES_AUTOHSCROLL | CSTYLE,
		10, 40, 140, 20,
		mainwin, NULL, hInstance, NULL);
	if (entry == NULL)
		panic("error creating entry");
	SETFONT(entry);

	hwnd = CreateWindowEx(CXSTYLE,
		L"button", L"Set",
		BS_PUSHBUTTON | CSTYLE,
		160, 40, 70, 20,
		mainwin, (HMENU) ID_SET, hInstance, NULL);
	if (hwnd == NULL)
		panic("error creating Set button");
	SETFONT(hwnd);

	hwnd = CreateWindowEx(CXSTYLE,
		L"button", L"Marquee",
		BS_PUSHBUTTON | CSTYLE,
		240, 40, 70, 20,
		mainwin, (HMENU) ID_MARQUEE, hInstance, NULL);
	if (hwnd == NULL)
		panic("error creating Marquee button");
	SETFONT(hwnd);

	hwnd = CreateWindowEx(CXSTYLE,
		L"static", L"enter a number in the range [0,100]",
		SS_NOPREFIX | CSTYLE,
		10, 70, 300, 20,
		mainwin, NULL, hInstance, NULL);
	if (hwnd == NULL)
		panic("error creating static");
	SETFONT(hwnd);
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
	INITCOMMONCONTROLSEX icc;
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
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
	if (InitCommonControlsEx(&icc) == FALSE)
		panic("error initializing Common Controls");
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
