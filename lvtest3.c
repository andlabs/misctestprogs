// 17 march-4 april 2015
// based on lvtest2 6 january 2015
// based on lvtest 14 december 2014
// package ui comctl32.dll code 17 july 2014

// TODO set focus on list view at first?

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

static ULONG_PTR comctlManifestCookie;
static HMODULE comctl32;

#define wantedICCClasses ( \
	ICC_PROGRESS_CLASS |		/* progress bars */		\
	ICC_TAB_CLASSES |			/* tabs */				\
	ICC_LISTVIEW_CLASSES |		/* list views */			\
	ICC_UPDOWN_CLASS |		/* spinboxes */		\
	0)

// note that this is an 8-bit character string we're writing; see the encoding clause
static const char manifest[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">\n<assemblyIdentity\n    version=\"1.0.0.0\"\n    processorArchitecture=\"*\"\n    name=\"CompanyName.ProductName.YourApplication\"\n    type=\"win32\"\n/>\n<description>Your application description here.</description>\n<dependency>\n    <dependentAssembly>\n        <assemblyIdentity\n            type=\"win32\"\n            name=\"Microsoft.Windows.Common-Controls\"\n            version=\"6.0.0.0\"\n            processorArchitecture=\"*\"\n            publicKeyToken=\"6595b64144ccf1df\"\n            language=\"*\"\n        />\n    </dependentAssembly>\n</dependency>\n</assembly>\n";

void die(char *s)
{
	// TODO
}

void initCommonControls(BOOL comctl6)
{
	WCHAR temppath[MAX_PATH + 1];
	WCHAR filename[MAX_PATH + 1];
	HANDLE file;
	DWORD nExpected, nGot;
	ACTCTX actctx;
	HANDLE ac;
	INITCOMMONCONTROLSEX icc;
	FARPROC f;
	// this is listed as WINAPI in both Microsoft's and MinGW's headers, but not on MSDN for some reason
	BOOL (*WINAPI ficc)(const LPINITCOMMONCONTROLSEX);

	if (comctl6) {
		if (GetTempPathW(MAX_PATH + 1, temppath) == 0)
			die("getting temporary path for writing manifest file");
		if (GetTempFileNameW(temppath, L"manifest", 0, filename) == 0)
			die("getting temporary filename for writing manifest file");
		file = CreateFileW(filename, GENERIC_WRITE,
			0,			// don't share while writing
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file == NULL)
			die("creating manifest file");
		nExpected = (sizeof manifest / sizeof manifest[0]) - 1;		// - 1 to omit the terminating null character)
		if (WriteFile(file, manifest, nExpected, &nGot, NULL) == 0)
			die("writing manifest file");
		if (nGot != nExpected)
			die("short write to manifest file");
		if (CloseHandle(file) == 0)
			die("closing manifest file (this IS an error here because not doing so will prevent Windows from being able to use the manifest file in an activation context)");

		ZeroMemory(&actctx, sizeof (ACTCTX));
		actctx.cbSize = sizeof (ACTCTX);
		actctx.dwFlags = ACTCTX_FLAG_SET_PROCESS_DEFAULT;
		actctx.lpSource = filename;
		ac = CreateActCtx(&actctx);
		if (ac == INVALID_HANDLE_VALUE)
			die("creating activation context for synthesized manifest file");
		if (ActivateActCtx(ac, &comctlManifestCookie) == FALSE)
			die("activating activation context for synthesized manifest file");
	}

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = wantedICCClasses;

	comctl32 = LoadLibraryW(L"comctl32.dll");
	if (comctl32 == NULL)
		die("loading comctl32.dll");
	f = GetProcAddress(comctl32, "InitCommonControlsEx");
	if (f == NULL)
		die("loading InitCommonControlsEx()");
	ficc = (BOOL (*WINAPI)(const LPINITCOMMONCONTROLSEX)) f;
	if ((*ficc)(&icc) == FALSE)
		die("initializing Common Controls (comctl32.dll)");
}
HFONT msgfont;

HWND lv = NULL;
HWND bEnable = NULL;
BOOL enabled = TRUE;		// initial state of list view
HWND bDelFirst = NULL;
HWND bDelLast = NULL;
HWND bDelMiddle = NULL;

#define MARGIN 6
#define PADDING 4
#define BTNWID 70
#define BTNHT 20
#define WINWIDTH ((2 * MARGIN) + (4 * BTNWID) + (3 * PADDING))
#define WINHEIGHT 300

void deln(int which)
{
	SendMessageW(lv, LVM_DELETEITEM, (WPARAM) which, 0);
}

BOOL fullrow = TRUE;
BOOL hascheckboxes = FALSE;
BOOL tooltips = FALSE;
BOOL gridlines = FALSE;
BOOL custommargin = FALSE;
BOOL hotlabels = FALSE;
// TODO make an option
#define CUSTOMMARGIN 30
DWORD lvstyle = 0;
DWORD lvexstyle = 0;

BOOL create(HWND hwnd, LPCREATESTRUCT lpcs)
{
	LVCOLUMNW col;
	LVITEMW item;
	int i, j;
	WCHAR text[100];

#define MKBUTTON(var, name, n) var = CreateWindowExW(0, \
		L"button", name, \
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, \
		MARGIN + (n * (BTNWID + PADDING)), MARGIN, \
		BTNWID, BTNHT, \
		hwnd, NULL, GetModuleHandle(NULL), NULL); \
	SendMessage(var, WM_SETFONT, (WPARAM) msgfont, (LPARAM) TRUE);
	MKBUTTON(bEnable, L"Disable", 0);
	MKBUTTON(bDelFirst, L"Delete First", 1);
	MKBUTTON(bDelLast, L"Delete Last", 2);
	// TODO make a macro or editable
	MKBUTTON(bDelMiddle, L"Delete 15", 3);

#define LVTOP (MARGIN + BTNHT + PADDING)
	lv = CreateWindowExW(lvexstyle,
		WC_LISTVIEWW, L"Main Window",
		LVS_REPORT | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS | WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE | lvstyle,
		MARGIN, LVTOP,
		WINWIDTH - (2 * MARGIN), WINHEIGHT - LVTOP - MARGIN,
		hwnd, NULL, GetModuleHandle(NULL), NULL);
	if (fullrow)
		SendMessageW(lv, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	if (hascheckboxes)
		SendMessageW(lv, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);
	if (tooltips)
		SendMessageW(lv, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_LABELTIP, LVS_EX_LABELTIP);
	if (gridlines)
		SendMessageW(lv, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_GRIDLINES, LVS_EX_GRIDLINES);
	if (hotlabels)
		SendMessageW(lv, LVM_SETEXTENDEDLISTVIEWSTYLE, (LVS_EX_UNDERLINEHOT | LVS_EX_ONECLICKACTIVATE), (LVS_EX_UNDERLINEHOT | LVS_EX_ONECLICKACTIVATE));

	if (custommargin) {
		HWND header;

		header = (HWND) SendMessageW(lv, LVM_GETHEADER, 0, 0);
		SendMessageW(header, HDM_SETBITMAPMARGIN, CUSTOMMARGIN, 0);
	}

	for (i = 0; i < 5; i++) {
		ZeroMemory(&col, sizeof (LVCOLUMNW));
		col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
		col.fmt = LVCFMT_LEFT;
		col.cx = 200;
		col.pszText = L"Column";
		SendMessage(lv, LVM_INSERTCOLUMN, (WPARAM) i, (LPARAM) (&col));
	}

	for (i = 0; i < 100; i++) {
		for (j = 0; j < 5; j++) {
			ZeroMemory(&item, sizeof (LVITEMW));
			item.mask = LVIF_TEXT;
			if (i == 5 && j == 0)
				item.pszText = L"0123456789112345678921234567893123456789412345678951234567896123456789712345This label is longer than the supposed maximum tooltip width!";
			else if ((i % 6) == 5 && j == 0)
				item.pszText = L"Very long item text that will be ellipsized";
			else {
				wsprintf(text, L"Item (%d,%d)", i, j);
				item.pszText = text;
			}
			item.iItem = i;
			item.iSubItem = j;
			if (j == 0)
				SendMessageW(lv, LVM_INSERTITEM, 0, (LPARAM) (&item));
			else
				SendMessageW(lv, LVM_SETITEM, 0, (LPARAM) (&item));
		}
	}

	return TRUE;
}

void destroy(HWND hwnd)
{
	DestroyWindow(lv);
	PostQuitMessage(0);
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	HANDLE_MSG(hwnd, WM_CREATE, create);
	HANDLE_MSG(hwnd, WM_DESTROY, destroy);
	case WM_COMMAND:
#define ISCLICKED(x) (lParam == (LPARAM) x && HIWORD(wParam) == BN_CLICKED)
		if (ISCLICKED(bEnable)) {
			enabled = !enabled;
			EnableWindow(lv, enabled);
			if (enabled)
				SetWindowTextW(bEnable, L"Disable");
			else
				SetWindowTextW(bEnable, L"Enable");
			return 0;
		}
		if (ISCLICKED(bDelFirst)) {
			deln(0);
			return 0;
		}
		if (ISCLICKED(bDelLast)) {
			deln(99);
			return 0;
		}
		if (ISCLICKED(bDelMiddle)) {
			deln(15);
			return 0;
		}
		break;		// fall through to DefWindowProc()
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main(int argc, char *argv[])
{
	WNDCLASSW wc;
	HWND hwnd;
	int nCmdShow;
	STARTUPINFOW si;
	MSG msg;
	RECT winr;
	NONCLIENTMETRICSW ncm;
	int i;
	BOOL comctl6 = TRUE;
	DWORD edges = 0;
	BOOL noclientedge = FALSE;
	BOOL multisel = FALSE;

	for (i = 1; i < argc; i++)
		if (strcmp(argv[i], "windowedge") == 0)
			edges |= WS_EX_WINDOWEDGE;
		else if (strcmp(argv[i], "staticedge") == 0)
			edges |= WS_EX_STATICEDGE;
		else if (strcmp(argv[i], "noclientedge") == 0)
			noclientedge = TRUE;
		else if (strcmp(argv[i], "border") == 0)
			lvstyle |= WS_BORDER;
		else if (strcmp(argv[i], "nofullrow") == 0)
			fullrow = FALSE;
		else if (strcmp(argv[i], "comctl5") == 0)
			comctl6 = FALSE;
		else if (strcmp(argv[i], "checkboxes") == 0)
			hascheckboxes = TRUE;
		else if (strcmp(argv[i], "tooltips") == 0)
			tooltips = TRUE;
		else if (strcmp(argv[i], "gridlines") == 0)
			gridlines = TRUE;
		else if (strcmp(argv[i], "editlabels") == 0)
			lvstyle |= LVS_EDITLABELS;
		else if (strcmp(argv[i], "custommargin") == 0)
			custommargin = TRUE;
		else if (strcmp(argv[i], "hotlabels") == 0)
			hotlabels = TRUE;
		else if (strcmp(argv[i], "multisel") == 0)
			multisel = TRUE;
		else {
			fprintf(stderr, "unknown option %s\n", argv[i]);
			return 1;
		}

	if (!noclientedge)
		edges |= WS_EX_CLIENTEDGE;
	lvexstyle |= edges;

	if (!multisel)
		lvstyle |= LVS_SINGLESEL;

	ZeroMemory(&ncm, sizeof (NONCLIENTMETRICSW));
	ncm.cbSize = sizeof (NONCLIENTMETRICSW);
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof (NONCLIENTMETRICSW), &ncm, sizeof (NONCLIENTMETRICSW));
	msgfont = CreateFontIndirectW(&(ncm.lfMessageFont));

	initCommonControls(comctl6);

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.hInstance = GetModuleHandle(NULL);
	RegisterClassW(&wc);

#define STYLE (WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME))
	winr.left = 0;
	winr.top = 0;
	winr.right = WINWIDTH;
	winr.bottom = WINHEIGHT;
	AdjustWindowRect(&winr, STYLE, FALSE);
	hwnd = CreateWindowExW(0,
		L"mainwin", L"Main Window",
		STYLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		winr.right - winr.left, winr.bottom - winr.top,
		NULL, NULL, GetModuleHandle(NULL), NULL);

	GetStartupInfoW(&si);
	nCmdShow = SW_SHOWDEFAULT;
	if ((si.dwFlags & STARTF_USESHOWWINDOW) != 0)
		nCmdShow = si.wShowWindow;
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
