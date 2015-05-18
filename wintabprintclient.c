// 18 may 2015
// based on wintabparentwinebug.c 3 may 2015
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

void die(char *s)
{
	// TODO
}

void initCommonControls(BOOL);

HWND mainwin;
HWND tab;

#define BGCOLOR RGB(0x0A, 0x24, 0x6A)
#define PCOLOR RGB(0x6A, 0x24, 0x0A)

LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NMHDR *nm = (NMHDR *) lParam;
	PAINTSTRUCT ps;
	HDC dc;
	POINT prev;
	RECT r;
	HBRUSH b;

	switch (uMsg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		dc = BeginPaint(hwnd, &ps);
		SetWindowOrgEx(dc, -240, -20, &prev);
		SendMessage(tab, WM_PRINTCLIENT, (WPARAM) dc, PRF_CLIENT);
		SetWindowOrgEx(dc, prev.x, prev.y, NULL);
		EndPaint(hwnd, &ps);
{COLORREF r;
r=GetSysColor(COLOR_ACTIVECAPTION);
printf("%I32X\n", r);}
		return 0;
	case WM_PRINTCLIENT:
		// the tab control sends this to draw the background of the area where the tab buttons are
		b = CreateSolidBrush(PCOLOR);
		GetClientRect(hwnd, &r);
		FillRect((HDC) wParam, &r, b);
		DeleteObject(b);
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static void makeWindows(void)
{
	mainwin = CreateWindowExW(0,
		L"mainwin", L"Full Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 500,
		NULL, NULL, GetModuleHandle(NULL), NULL);

	// create the tab as a child of the empty window...
	tab = CreateWindowExW(0,
		WC_TABCONTROLW, L"",
		TCS_TOOLTIPS | WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		20, 20, 200, 440,
		mainwin, (HMENU) 100, GetModuleHandle(NULL), NULL);
}

void addTab(WCHAR *name)
{
	TCITEMW item;
	LRESULT n;

	n = SendMessageW(tab, TCM_GETITEMCOUNT, 0, 0);
	ZeroMemory(&item, sizeof (TCITEMW));
	item.mask = TCIF_TEXT;
	item.pszText = name;
	SendMessageW(tab, TCM_INSERTITEM, (WPARAM) n, (LPARAM) (&item));
}

int main(int argc, char *argv[])
{
	WNDCLASSW wc;
	MSG msg;
	HBRUSH b;

	initCommonControls(argc > 1 && strcmp(argv[1], "comctl6") == 0);

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	// if printing client doesn't print the tab background, this color will bleed through instead
	b = CreateSolidBrush(BGCOLOR);
	wc.hbrBackground = b;
	RegisterClassW(&wc);

	makeWindows();
	addTab(L"Page 1");
	addTab(L"Page 2");

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

static const char manifest[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">\n<assemblyIdentity\n    version=\"1.0.0.0\"\n    processorArchitecture=\"*\"\n    name=\"CompanyName.ProductName.YourApplication\"\n    type=\"win32\"\n/>\n<description>Your application description here.</description>\n<dependency>\n    <dependentAssembly>\n        <assemblyIdentity\n            type=\"win32\"\n            name=\"Microsoft.Windows.Common-Controls\"\n            version=\"6.0.0.0\"\n            processorArchitecture=\"*\"\n            publicKeyToken=\"6595b64144ccf1df\"\n            language=\"*\"\n        />\n    </dependentAssembly>\n</dependency>\n</assembly>\n";

static ULONG_PTR comctlManifestCookie;
static HMODULE comctl32;

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
	icc.dwICC = ICC_TAB_CLASSES;

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
