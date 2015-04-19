// 18 april 2015
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

static void paintControlBackground(HWND hwnd, HDC dc)
{
	HWND parent;
	RECT r;
	POINT pOrig;
	int class;
	DWORD le;

	parent = hwnd;
	// get to the container
	parent = GetParent(parent);
	if (parent == NULL)
		die("error getting parent control of control in paintControlBackground()");
	// and again to get to the tab
	parent = GetParent(parent);
	if (parent == NULL)
		die("likewise");
	if (GetWindowRect(hwnd, &r) == 0)
		die("error getting control's window rect in paintControlBackground()");
	// the above is a window rect in screen coordinates; convert to parent coordinates
	SetLastError(0);
	if (MapWindowRect(NULL, parent, &r) == 0) {
		le = GetLastError();
		SetLastError(le);		// just to be safe
		if (le != 0)
			die("error getting client origin of control in paintControlBackground()");
	}
	if (SetWindowOrgEx(dc, r.left, r.top, &pOrig) == 0)
		die("error moving window origin in paintControlBackground()");
	SendMessageW(parent, WM_PRINTCLIENT, (WPARAM) dc, PRF_CLIENT);
	if (SetWindowOrgEx(dc, pOrig.x, pOrig.y, NULL) == 0)
		die("error resetting window origin in paintControlBackground()");
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
		if (SetBkMode((HDC) wParam, TRANSPARENT) == 0)
			die("setting transparent background mode to controls in wndproc()");
		paintControlBackground((HWND) lParam, (HDC) wParam);
		return (LRESULT) GetStockObject(HOLLOW_BRUSH);
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	WNDCLASSW wc;
	HWND mainwin;
	HWND tab;
	HWND container;
	HWND button1, button2;
	MSG msg;

	initCommonControls(TRUE);

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClassW(&wc) == 0)
		die("registering window class");

	mainwin = CreateWindowExW(0,
		L"mainwin", L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 500,
		NULL, NULL, GetModuleHandle(NULL), NULL);
	if (mainwin == NULL)
		die("creating main window");

	tab = CreateWindowExW(0,
		WC_TABCONTROLW, L"",
		TCS_TOOLTIPS | WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		50, 50,
		400, 400,
		mainwin, NULL, GetModuleHandle(NULL), NULL);
	if (tab == NULL)
		die("creating tab");

	container = CreateWindowExW(0,
		L"mainwin", L"",
		WS_CHILD | WS_VISIBLE,
		100, 100,
		200, 200,
		tab, NULL, GetModuleHandle(NULL), NULL);
	if (container == NULL)
		die("creating inner container");

	button1 = CreateWindowExW(0,
		L"button", L"Click Me",
		BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
		50, 50,
		100, 30,
		container, NULL, GetModuleHandle(NULL), NULL);
	if (button1 == NULL)
		die("creating button 1");

	button2 = CreateWindowExW(0,
		L"button", L"Click Me",
		BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
		50, 100,
		100, 30,
		container, NULL, GetModuleHandle(NULL), NULL);
	if (button2 == NULL)
		die("creating button 2");

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
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
