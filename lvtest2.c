// 6 january 2015
// based on lvtest 14 december 2014
// package ui comctl32.dll code 17 july 2014

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

/*
Windows requires a manifest file to enable Common Controls version 6.
The only way to not require an external manifest is to synthesize the manifest ourselves.
We can use the activation context API to load it at runtime.
References:
- http://stackoverflow.com/questions/4308503/how-to-enable-visual-styles-without-a-manifest
- http://support.microsoft.com/kb/830033
Because neither Go nor MinGW have ways to compile in resources like this (as far as I know), we have to do the work ourselves.
*/
DWORD initCommonControls(char **errmsg)
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

	if (GetTempPathW(MAX_PATH + 1, temppath) == 0) {
		*errmsg = "error getting temporary path for writing manifest file";
		return GetLastError();
	}
	if (GetTempFileNameW(temppath, L"manifest", 0, filename) == 0) {
		*errmsg = "error getting temporary filename for writing manifest file";
		return GetLastError();
	}
	file = CreateFileW(filename, GENERIC_WRITE,
		0,			// don't share while writing
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == NULL) {
		*errmsg = "error creating manifest file";
		return GetLastError();
	}
	nExpected = (sizeof manifest / sizeof manifest[0]) - 1;		// - 1 to omit the terminating null character)
	SetLastError(0);		// catch errorless short writes
	if (WriteFile(file, manifest, nExpected, &nGot, NULL) == 0) {
		*errmsg = "error writing manifest file";
		return GetLastError();
	}
	if (nGot != nExpected) {
		DWORD lasterr;

		lasterr = GetLastError();
		*errmsg = "short write to manifest file";
		if (lasterr == 0)
			*errmsg = "short write to manifest file without error code";
		return lasterr;
	}
	if (CloseHandle(file) == 0) {
		*errmsg = "error closing manifest file (this IS an error here because not doing so will prevent Windows from being able to use the manifest file in an activation context)";
		return GetLastError();
	}

	ZeroMemory(&actctx, sizeof (ACTCTX));
	actctx.cbSize = sizeof (ACTCTX);
	actctx.dwFlags = ACTCTX_FLAG_SET_PROCESS_DEFAULT;
	actctx.lpSource = filename;
	ac = CreateActCtx(&actctx);
	if (ac == INVALID_HANDLE_VALUE) {
		*errmsg = "error creating activation context for synthesized manifest file";
		return GetLastError();
	}
	if (ActivateActCtx(ac, &comctlManifestCookie) == FALSE) {
		*errmsg = "error activating activation context for synthesized manifest file";
		return GetLastError();
	}

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = wantedICCClasses;

	comctl32 = LoadLibraryW(L"comctl32.dll");
	if (comctl32 == NULL) {
		*errmsg = "error loading comctl32.dll";
		return GetLastError();
	}

	// GetProcAddress() only takes a multibyte string
#define LOAD(fn) f = GetProcAddress(comctl32, fn); \
	if (f == NULL) { \
		*errmsg = "error loading " fn "()"; \
		return GetLastError(); \
	}

	LOAD("InitCommonControlsEx");
	ficc = (BOOL (*WINAPI)(const LPINITCOMMONCONTROLSEX)) f;
	if ((*ficc)(&icc) == FALSE) {
		*errmsg = "error initializing Common Controls (comctl32.dll)";
		return GetLastError();
	}

	return 0;
}

HWND lv = NULL;

BOOL create(HWND hwnd, LPCREATESTRUCT lpcs)
{
	LVCOLUMNW col;
	LVITEMW item;
	int i, j;
	WCHAR text[100];

	lv = CreateWindowExW(0,
		WC_LISTVIEWW, L"Main Window",
		LVS_REPORT | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS | LVS_SINGLESEL | WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		300, 300,
		hwnd, NULL, GetModuleHandle(NULL), NULL);
	SendMessageW(lv, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

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
			wsprintf(text, L"Item (%d,%d)", i, j);
			item.pszText = text;
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

void resize(HWND hwnd, UINT state, int cx, int cy)
{
	if (lv != NULL)
		MoveWindow(lv, 0, 0, cx, cy, TRUE);
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	HANDLE_MSG(hwnd, WM_CREATE, create);
	HANDLE_MSG(hwnd, WM_SIZE, resize);
	HANDLE_MSG(hwnd, WM_DESTROY, destroy);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	WNDCLASSW wc;
	HWND hwnd;
	int nCmdShow;
	STARTUPINFOW si;
	MSG msg;
	char *m;

	initCommonControls(&m);

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.hInstance = GetModuleHandle(NULL);
	RegisterClassW(&wc);

	hwnd = CreateWindowExW(0,
		L"mainwin", L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		300, 300,
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
