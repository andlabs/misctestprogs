// 4 june 2014
// scratch Windows program by pietro gagliardi 17 april 2014
// fixed typos and added toWideString() 1 may 2014
// borrows code from the scratch GTK+ program (16-17 april 2014) and from code written 31 march 2014 and 11-12 april 2014
#define _UNICODE
#define UNICODE
#define STRICT
#define _GNU_SOURCE		// needed to declare asprintf()/vasprintf()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include <windows.h>
#include <commctrl.h>		// needed for InitCommonControlsEx() (thanks Xeek in irc.freenode.net/#winapi for confirming)

#ifdef  _MSC_VER
#error sorry! the scratch windows program relies on mingw-only functionality! (specifically: asprintf(), getopt_long_only())
#endif

// cheating: we store the help string in the flag argument, collect them, then overwrite them with NULL in init() so getopt_long_only() will return val and not overwrite a string (apparently I'm not the first to think of this: GerbilSoft says GNU tools do this too)
#define flagBool(name, help, short) { name, no_argument, (int *) help, short }
#define flagString(name, help, short) { name, required_argument, (int *) help, short }
static struct option flags[] = {
	// place other options here
	flagBool("help", "show help and quit", 'h'),
	{ 0, 0, 0, 0 },
};

HMODULE hInstance;
HICON hDefaultIcon;
HCURSOR hDefaultCursor;
HFONT controlfont;

void panic(char *fmt, ...);
TCHAR *toWideString(char *what);

void init(int argc, char *argv[]);

char *args = "";		// other command-line arguments here, if any

BOOL parseArgs(int argc, char *argv[])
{
	// parse arguments here, if any; use optind
	// return TRUE if parsed successfully; FALSE otherwise
	// (returning FALSE will show usage and quit)
	return TRUE;
}

LOGFONT clf;

void printfont(HDC dc, LOGFONT clf, TEXTMETRIC tm)
{
	LONG h;

	printf("%S ", clf.lfFaceName);
	h = clf.lfHeight;
	if (h < 0) {
		// solve height = -MulDiv(points, base, 72dpi) for points to get this
		h = -h;
		h = MulDiv(h, 72, GetDeviceCaps(dc, LOGPIXELSY));
	} else if (h > 0) {
		// solve the first formula in http://support.microsoft.com/kb/74299 for point size
		h = ((h - tm.tmInternalLeading) * 72) / GetDeviceCaps(dc, LOGPIXELSY);
	} else
		panic("not sure how to convert %d from height to points", h);
	printf("%d", h);
}

typedef void (*dlgfn)(int, int, va_list, int *, int *);

void printdlgunits(char *what, HDC dc, TEXTMETRIC cltm, dlgfn f, ...)
{
	int res1, res2;
	va_list arg;

// button
#define BUTTON_SIZE_X 50
#define BUTTON_SIZE_Y 14
	printf(what);
	printfont(dc, clf, cltm);
	printf(" | ");
	va_start(arg, f);
	(*f)(BUTTON_SIZE_X, BUTTON_SIZE_Y, arg, &res1, &res2);
	va_end(arg);
	printf(" | button %d x %d -> %d x %d\n",
		BUTTON_SIZE_X, BUTTON_SIZE_Y,
		res1, res2);
}

void dlg_nosys(int w, int h, va_list arg, int *resX, int *resY)
{
	int baseX, baseY;

	baseX = va_arg(arg, int);
	baseY = va_arg(arg, int);
	*resX = MulDiv(w, baseX, 4);
	*resY = MulDiv(h, baseY, 8);
	printf("baseX %d baseY %d", baseX, baseY);
}

void dlg_sys(int w, int h, va_list arg, int *resX, int *resY)
{
	int cX, cY, sysX, sysY;

	cX = va_arg(arg, int);
	sysX = va_arg(arg, int);
	cY = va_arg(arg, int);
	sysY = va_arg(arg, int);
	*resX = 2 * w * (((double) (cX)) / ((double) (sysX)));
	*resY = 2 * h * (((double) (cY)) / ((double) (sysY)));
	printf("cX %d cY %d sysX %d sysY %d", cX, cY, sysX, sysY);
}

TEXTMETRIC getAverages(HDC dc, int *xGTEP32, int *xTM, int *y)
{
	SIZE extents;
	TEXTMETRIC tm;

#define GTEP32STR L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
	if (GetTextExtentPoint32(dc, GTEP32STR, 52, &extents) == 0)
		panic("error calling GetTextExtents32()");
	*xGTEP32 = (extents.cx / 26 + 1) / 2;
	if (GetTextMetrics(dc, &tm) == 0)
		panic("error calling GetTextMetrics()");
	*xTM = tm.tmAveCharWidth;
	*y = tm.tmHeight;
	return tm;
}

void paint(HDC dc)
{
	HFONT prev;		// holds the System font
	int cXG, cXT, cY;
	int sysXG, sysXT, sysY;
	LOGFONT syslf;
	TEXTMETRIC ctm, systm;

	systm = getAverages(dc, &sysXG, &sysXT, &sysY);
	prev = SelectObject(dc, controlfont);
	if (prev == NULL)
		panic("error calling SelectObject()");
	if (GetObject(prev, sizeof (LOGFONT), &syslf) == 0)
		panic("error getting LOGFONT for the System font");
	ctm = getAverages(dc, &cXG, &cXT, &cY);

	printdlgunits("GetTextExtentPoint32: ", dc, ctm, dlg_nosys, cXG, cY);
	printdlgunits("tm.tmAveCharWidth:    ", dc, ctm, dlg_nosys, cXT, cY);
	printf("with system font: ");
	printfont(dc, syslf, systm);
	printf("\n");
	printdlgunits("GetTextExtentPoint32: ", dc, ctm, dlg_sys, cXG, sysXG, cY, sysY);
	printdlgunits("tm.tmAveCharWidth:    ", dc, ctm, dlg_sys, cXT, sysXT, cY, sysY);

	if (SelectObject(dc, prev) == NULL)
		panic("error returning font to normal");
	exit(0);		// no need to do anything special anymore
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	PAINTSTRUCT ps;
	HDC dc;

	switch (msg) {
	case WM_PAINT:
		dc = BeginPaint(hwnd, &ps);
		if (dc == NULL)
			panic("error calling BeginPaint()");
		paint(dc);
		EndPaint(hwnd, &ps);
		return 0;
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
		CW_USEDEFAULT, CW_USEDEFAULT,
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
	// build GUI here; use CSTYLE and CXSTYLE in CreateWindowEx() and call SETFONT() on each new widget
}

void firstShowWindow(HWND hwnd);

int main(int argc, char *argv[])
{
	HWND mainwin;
	MSG msg;

	init(argc, argv);

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
//	ICC_TAB_CLASSES |				// tab, tooltip
//	ICC_TREEVIEW_CLASSES |		// tree-view, tooltip
//	ICC_UPDOWN_CLASS |			// up-down
//	ICC_USEREX_CLASSES |			// ComboBoxEx
//	ICC_WIN95_CLASSES |			// some of the above
	0;

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
	clf = ncm.lfMessageFont;
}

void init(int argc, char *argv[])
{
	int usageExit = 1;
	char *opthelp[512];		// more than enough
	int i;

	for (i = 0; flags[i].name != 0; i++) {
		opthelp[i] = (char *) flags[i].flag;
		flags[i].flag = NULL;
	}

	for (;;) {
		int c;

		c = getopt_long_only(argc, argv, "", flags, NULL);
		if (c == -1)
			break;
		switch (c) {
		case 'h':		// -help
			usageExit = 0;
			goto usage;
		case '?':
			// getopt_long_only() should have printed something since we did not set opterr to 0
			goto usage;
		default:
			fprintf(stderr, "internal error: getopt_long_only() returned %d\n", c);
			exit(1);
		}
	}

	if (parseArgs(argc, argv) != TRUE)
		goto usage;

	initwin();
	return;

usage:
	fprintf(stderr, "usage: %s [options]", argv[0]);
	if (args != NULL && *args != '\0')
		fprintf(stderr, " %s", args);
	fprintf(stderr, "\n");
	for (i = 0; flags[i].name != 0; i++)
		fprintf(stderr, "\t-%s%s - %s\n",
			flags[i].name,
			(flags[i].has_arg == required_argument) ? " string" : "",
			opthelp[i]);
	exit(usageExit);
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

TCHAR *toWideString(char *what)
{
	TCHAR *buf;
	int n;
	size_t len;

	len = strlen(what);
	if (len == 0) {
		buf = (TCHAR *) malloc(sizeof (TCHAR));
		if (buf == NULL)
			goto mallocfail;
		buf[0] = L'\0';
	} else {
		n = MultiByteToWideChar(CP_UTF8, 0, what, -1, NULL, 0);
		if (n == 0)
			panic("error getting number of bytes to convert \"%s\" to UTF-16", what);
		buf = (TCHAR *) malloc((n + 1) * sizeof (TCHAR));
		if (buf == NULL)
			goto mallocfail;
		if (MultiByteToWideChar(CP_UTF8, 0, what, -1, buf, n) == 0)
			panic("erorr converting \"%s\" to UTF-16", what);
	}
	return buf;
mallocfail:
	panic("error allocating memory for UTF-16 version of \"%s\"", what);
}
