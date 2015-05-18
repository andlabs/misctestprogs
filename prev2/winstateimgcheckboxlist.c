// 27 august 2014
// modified from winstateimglist.c 17 august 2014
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
#include <windows.h>
#include <commctrl.h>		// needed for InitCommonControlsEx() (thanks Xeek in irc.freenode.net/#winapi for confirming)
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>

#ifdef  _MSC_VER
#error sorry! the scratch windows program relies on mingw-only functionality! (specifically: asprintf())
#endif

HMODULE hInstance;
HICON hDefaultIcon;
HCURSOR hDefaultCursor;
HFONT controlfont;

void panic(char *fmt, ...);
TCHAR *toWideString(char *what);
void init(void);

LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	NMHDR *nmhdr = (NMHDR *) lparam;
	NMLVDISPINFOW *fill = (NMLVDISPINFO *) lparam;

	switch (msg) {
	case WM_NOTIFY:
		if (nmhdr->code == LVN_GETDISPINFO) {
			if (fill->item.iSubItem == 0) {
				fill->item.state = INDEXTOSTATEIMAGEMASK(fill->item.iItem + 1);
				fill->item.stateMask = LVIS_STATEIMAGEMASK;
			} else
				fill->item.pszText = L"No State Image Here";
			return 0;
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
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

void buildUI(HWND mainwin)
{
#define CSTYLE (WS_CHILD | WS_VISIBLE)
#define CXSTYLE (0)
#define SETFONT(hwnd) SendMessage(hwnd, WM_SETFONT, (WPARAM) controlfont, (LPARAM) TRUE);

	HWND lv;
	LVCOLUMN column;
	HIMAGELIST imglist;

	lv = CreateWindowEx(WS_EX_CLIENTEDGE | CXSTYLE,
		WC_LISTVIEW, L"",
		LVS_REPORT | LVS_OWNERDATA | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS | WS_HSCROLL | WS_VSCROLL | WS_TABSTOP | CSTYLE,
		10, 10, 250, 250,
		mainwin, (HMENU) 100, hInstance, NULL);
	if (lv == NULL)
		panic("error making list view");
	SETFONT(lv);
	SendMessageW(lv, LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES,
		LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);

	// error checking elided from this point to where otherwise noted

	HTHEME theme = NULL;
	HIMAGELIST makeCheckboxImageList(HWND hwnddc, HTHEME *theme);

	SendMessageW(lv, LVM_SETIMAGELIST, LVSIL_STATE, (LPARAM) makeCheckboxImageList(lv, &theme));

	ZeroMemory(&column, sizeof (LVCOLUMN));
	column.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER;
	column.fmt = LVCFMT_LEFT;
	column.pszText = L"State Image";
	column.iSubItem = 0;
	column.iOrder = 0;
	SendMessageW(lv, LVM_INSERTCOLUMN, 0, (LPARAM) (&column));
	ZeroMemory(&column, sizeof (LVCOLUMN));
	column.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER;
	column.fmt = LVCFMT_LEFT;
	column.pszText = L"No State Image";
	column.iSubItem = 1;
	column.iOrder = 1;
	SendMessageW(lv, LVM_INSERTCOLUMN, 1, (LPARAM) (&column));

	// end of error eliding

        if (SendMessageW(lv, LVM_SETCALLBACKMASK, LVIS_STATEIMAGEMASK, 0) == FALSE)
		panic("error marking state image list as application-managed");
	if (SendMessageW(lv, LVM_SETITEMCOUNT, 8, 0) == 0)
		panic("error setting number of items in list view");
}

void firstShowWindow(HWND hwnd);

int main(void)
{
	HWND mainwin;
	MSG msg;

	init();

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
	ICC_LISTVIEW_CLASSES |			// list-view, header
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

#define checkboxnStates 8

static int themestates[checkboxnStates] = {
	CBS_UNCHECKEDNORMAL,			// 0
	CBS_CHECKEDNORMAL,				// checked
	CBS_UNCHECKEDHOT,				// hot
	CBS_CHECKEDHOT,					// checked | hot
	CBS_UNCHECKEDPRESSED,			// pushed
	CBS_CHECKEDPRESSED,				// checked | pushed
	CBS_UNCHECKEDPRESSED,			// hot | pushed
	CBS_CHECKEDPRESSED,				// checked | hot | pushed
};

static SIZE getStateSize(HDC dc, int cbState, HTHEME theme)
{
	SIZE s;
	HRESULT res;

	res = GetThemePartSize(theme, dc, BP_CHECKBOX, themestates[cbState], NULL, TS_DRAW, &s);
	if (res != S_OK)
		panic("error getting theme part size; HRESULT: %x", res);
	return s;
}

static void themeImage(HDC dc, RECT *r, int cbState, HTHEME theme)
{
	HRESULT res;

	res = DrawThemeBackground(theme, dc, BP_CHECKBOX, themestates[cbState], r, NULL);
	if (res != S_OK)
		panic("error drawing checkbox image; HRESULT: %x", res);
}

static void themeSize(HDC dc, int *width, int *height, HTHEME theme)
{
	SIZE size;
	int cbState;

	size = getStateSize(dc, 0, theme);
	for (cbState = 1; cbState < checkboxnStates; cbState++) {
		SIZE against;

		against = getStateSize(dc, cbState, theme);
		if (size.cx != against.cx || size.cy != against.cy)
			panic("size mismatch in checkbox states");
	}
	*width = (int) size.cx;
	*height = (int) size.cy;
}

static HBITMAP makeCheckboxImageListEntry(HDC dc, int width, int height, int cbState, void (*drawfunc)(HDC, RECT *, int, HTHEME), HTHEME theme)
{
	BITMAPINFO bi;
	VOID *ppvBits;
	HBITMAP bitmap;
	RECT r;
	HDC drawDC;
	HBITMAP prevbitmap;

	r.left = 0;
	r.top = 0;
	r.right = width;
	r.bottom = height;
	ZeroMemory(&bi, sizeof (BITMAPINFO));
	bi.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = (LONG) width;
	bi.bmiHeader.biHeight = -((LONG) height);			// negative height to force top-down drawing;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = (DWORD) (width * height * 4);
	bitmap = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &ppvBits, 0, 0);
	if (bitmap == NULL)
		panic("error creating HBITMAP for unscaled ImageList image copy");

	drawDC = CreateCompatibleDC(dc);
	if (drawDC == NULL)
		panic("error getting DC for checkbox image list bitmap");
	prevbitmap = SelectObject(drawDC, bitmap);
	if (prevbitmap == NULL)
		panic("error selecting checkbox image list bitmap into DC");
	(*drawfunc)(drawDC, &r, cbState, theme);
	if (SelectObject(drawDC, prevbitmap) != bitmap)
		panic("error selecting previous bitmap into checkbox image's DC");
	if (DeleteDC(drawDC) == 0)
		panic("error deleting checkbox image's DC");

	return bitmap;
}

static HIMAGELIST newCheckboxImageList(HWND hwnddc, void (*sizefunc)(HDC, int *, int *, HTHEME), void (*drawfunc)(HDC, RECT *, int, HTHEME), HTHEME theme)
{
	int width, height;
	int cbState;
	HDC dc;
	HIMAGELIST il;

	dc = GetDC(hwnddc);
	if (dc == NULL)
		panic("error getting DC for making the checkbox image list");
	(*sizefunc)(dc, &width, &height, theme);
	il = ImageList_Create(width, height, ILC_COLOR32, 20, 20);		// should be reasonable
	if (il == NULL)
		panic("error creating checkbox image list");
	for (cbState = 0; cbState < checkboxnStates; cbState++) {
		HBITMAP bitmap;

		bitmap = makeCheckboxImageListEntry(dc, width, height, cbState, drawfunc, theme);
		if (ImageList_Add(il, bitmap, NULL) == -1)
			panic("error adding checkbox image to image list");
		if (DeleteObject(bitmap) == 0)
			panic("error deleting checkbox bitmap");
	}
	if (ReleaseDC(hwnddc, dc) == 0)
		panic("error deleting checkbox image list DC");
	return il;
}

HIMAGELIST makeCheckboxImageList(HWND hwnddc, HTHEME *theme)
{
	if (*theme != NULL) {
		HRESULT res;

		res = CloseThemeData(*theme);
		if (res != S_OK)
			panic("error closing theme; HRESULT: %x", res);
		*theme = NULL;
	}
	// ignore error; if it can't be done, we can fall back to DrawFrameControl()
	if (*theme == NULL)		// try to open the theme
		*theme = OpenThemeData(hwnddc, L"button");
	if (*theme != NULL)		// use the theme
		return newCheckboxImageList(hwnddc, themeSize, themeImage, *theme);
	panic("error loading theme");
	return NULL;		// unreached
}
