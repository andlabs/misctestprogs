// 14 august 2014
// scratch Windows program by pietro gagliardi 17 april 2014
// fixed typos 1 may 2014
// borrows code from the scratch GTK+ program (16-17 april 2014) and from code written 31 march 2014 and 11-12 april 2014
#define _UNICODE
#define UNICODE
#define STRICT
#define _GNU_SOURCE		// needed to declare asprintf()/vasprintf()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>
#include <commctrl.h>		// needed for InitCommonControlsEx() (thanks Xeek in irc.freenode.net/#winapi for confirming)

#ifdef  _MSC_VER
#error sorry! the scratch windows program relies on mingw-only functionality! (specifically: asprintf())
#endif

HMODULE hInstance;
HICON hDefaultIcon;
HCURSOR hDefaultCursor;
HFONT controlfont;

void panic(char *fmt, ...);
void init(void);

LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	panic("oops: message %ud does not return anything; bug in wndproc()", msg);
}

HWND makeMainWindow(void)
{
	WNDCLASS cls;
	HWND hwnd;

	ZeroMemory(&cls, sizeof (WNDCLASS));
	cls.lpszClassName = L"mainwin";
	cls.lpfnWndProc = wndproc;
	cls.hInstance = hInstance;
	cls.hIcon = hDefaultIcon;
	cls.hCursor = hDefaultCursor;
	cls.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClass(&cls) == 0)
		panic("error registering window class");
	hwnd = CreateWindowEx(0,
		L"mainwin", L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		300, 300,
		NULL, NULL, hInstance, NULL);
	if (hwnd == NULL)
		panic("opening main window failed");
	return hwnd;
}

HWND tab;

void buildUI(HWND mainwin)
{
#define SETFONT(hwnd) SendMessage(hwnd, WM_SETFONT, (WPARAM) controlfont, (LPARAM) TRUE);
	// build GUI here; use CSTYLE and CXSTYLE in CreateWindowEx() and call SETFONT() on each new widget

	HWND edit;

	tab = CreateWindow(WC_TABCONTROL, L"", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
		0, 0, 200, 200,
		mainwin, (HMENU) 100, hInstance, NULL);
	if (tab == NULL)
		panic("error making tab");
	SETFONT(tab);

	TCITEM item;

	ZeroMemory(&item, sizeof (TCITEM));
	item.mask = TCIF_TEXT;
	item.pszText = L"Tab";
	if (SendMessage(tab, TCM_INSERTITEM, 0, (LPARAM) (&item)) == (LRESULT) -1)
		panic("error adding tab");

	edit = CreateWindowEx(WS_EX_CLIENTEDGE, L"edit", L"", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
		40, 40, 100, 100,
		tab, (HMENU) 101, hInstance, NULL);
	if (edit == NULL)
		panic("error making edit");
	SETFONT(edit);
}

void tabEnterChildren(HWND hwnd)
{
	DWORD style, xstyle;

	style = (DWORD) GetWindowLongPtrW(hwnd, GWL_STYLE);
	xstyle = (DWORD) GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
	style &= ~((DWORD) WS_TABSTOP);
	xstyle |= WS_EX_CONTROLPARENT;
	SetWindowLongPtrW(hwnd, GWL_STYLE, (LONG_PTR) style);
	SetWindowLongPtrW(hwnd, GWL_EXSTYLE, (LONG_PTR) xstyle);
}

void tabLeaveChildren(HWND hwnd)
{
	DWORD style, xstyle;

	style = (DWORD) GetWindowLongPtrW(hwnd, GWL_STYLE);
	xstyle = (DWORD) GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
	style |= WS_TABSTOP;
	xstyle &= ~((DWORD) WS_EX_CONTROLPARENT);
	SetWindowLongPtrW(hwnd, GWL_STYLE, (LONG_PTR) style);
	SetWindowLongPtrW(hwnd, GWL_EXSTYLE, (LONG_PTR) xstyle);
}

void firstShowWindow(HWND hwnd);

int main(int argc, char *argv[])
{
	HWND mainwin;
	MSG msg;

	init();

	mainwin = makeMainWindow();
	buildUI(mainwin);
	firstShowWindow(mainwin);

	for (;;) {
		BOOL gmret;
		HWND active, focus;

		gmret = GetMessage(&msg, NULL, 0, 0);
		if (gmret == -1)
			panic("error getting message");
		if (gmret == 0)
			break;
		active = GetActiveWindow();
		if (active != NULL) {
			BOOL idm;

			focus = GetFocus();
			if (focus == tab)
				tabEnterChildren(tab);
			idm = IsDialogMessageW(active, &msg);
			if (focus == tab)
				tabLeaveChildren(tab);
			if (idm != 0)
				continue;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

DWORD iccFlags =
//	ICC_ANIMATE_CLASS |			// animation control
//	ICC_BAR_CLASSES |				// toolbar, statusbar, trackbar, tooltip
//	ICC_COOL_CLASSES |			// rebar
//	ICC_DATE_CLASSES |			// date and time picker
//	ICC_HOTKEY_CLASS |			// hot key
//	ICC_INTERNET_CLASSES |		// IP address entry field
//	ICC_LINK_CLASS |				// hyperlink
//	ICC_LISTVIEW_CLASSES |			// list-view, header
//	ICC_NATIVEFNTCTL_CLASS |		// native font
//	ICC_PAGESCROLLER_CLASS |		// pager
//	ICC_PROGRESS_CLASS |			// progress bar
//	ICC_STANDARD_CLASSES |		// "one of the intrinsic User32 control classes"
	ICC_TAB_CLASSES |				// tab, tooltip
//	ICC_TREEVIEW_CLASSES |		// tree-view, tooltip
//	ICC_UPDOWN_CLASS |			// up-down
//	ICC_USEREX_CLASSES |			// ComboBoxEx
//	ICC_WIN95_CLASSES |			// some of the above
	0;

void init(void)
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
	icc.dwICC = iccFlags;
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
	if (vasprintf(&msg, fmt, arg) == -1) {
		fprintf(stderr, "critical error: vasprintf() failed in panic() preparing panic message; fmt = \"%s\"\n", fmt);
		abort();
	}
	// according to http://msdn.microsoft.com/en-us/library/windows/desktop/ms680582%28v=vs.85%29.aspx
	lerrsuccess = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, lasterr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lerrmsg, 0, NULL);
	if (lerrsuccess == 0) {
		fprintf(stderr, "critical error: FormatMessage() failed in panic() preparing GetLastError() string; panic message = \"%s\", last error in panic(): %ld, last error from FormatMessage(): %ld\n", msg, lasterr, GetLastError());
		abort();
	}
	// note to self: use %ws instead of %S (thanks jon_y in irc.oftc.net/#mingw-w64)
	if (asprintf(&fullmsg, "panic: %s\nlast error: %ws\n", msg, lerrmsg) == -1) {
		fprintf(stderr, "critical error: asprintf() failed in panic() preparing full report; panic message = \"%s\", last error message: \"%ws\"\n", msg, lerrmsg);
		abort();
	}
	fprintf(stderr, "%s\n", fullmsg);
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
