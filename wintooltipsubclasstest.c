// 31 march 2015
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
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

void die(char *why)
{
	fprintf(stderr, "error %s: %I32u\n", why, GetLastError());
	abort();
}

HWND tooltip = NULL;

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CREATESTRUCT *cs = (CREATESTRUCT *) lParam;
	TOOLINFOW ti;

	switch (uMsg) {
	case WM_CREATE:
		tooltip = CreateWindowExW(WS_EX_TOOLWINDOW,
			TOOLTIPS_CLASSW, L"",
			WS_POPUP | TTS_NOPREFIX,
			0, 0,
			0, 0,
			hwnd, NULL, cs->hInstance, NULL);
		if (tooltip == NULL)
			die("creating tooltip");


		ZeroMemory(&ti, sizeof (TOOLINFOW));
		ti.cbSize = sizeof (TOOLINFOW);
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRANSPARENT;
		ti.hwnd = hwnd;
		ti.uId = (UINT_PTR) hwnd;
		ti.hinst = cs->hInstance;
		ti.lpszText = L"this is a tooltip! wow!";
		if (SendMessageW(tooltip, TTM_ADDTOOL, 0, (LPARAM) (&ti)) == FALSE)
			die("setting up tooltip");

		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_DESTROY:
		if (DestroyWindow(tooltip) == 0)
			die("deleting tooltips");
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void initCommonControls(BOOL);

int main(int argc, char *argv[])
{
	BOOL comctl5;
	WNDCLASSW wc;
	HWND mainwin;
	MSG msg;

	comctl5 = FALSE;
	if (argc > 1)
		comctl5 = strcmp(argv[1], "comctl5") == 0;
	initCommonControls(comctl5);

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.hInstance = GetModuleHandle(NULL);
	if (RegisterClassW(&wc) == 0)
		die("registering main window class");

	mainwin = CreateWindowExW(0,
		L"mainwin", L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, GetModuleHandle(NULL), NULL);
	if (mainwin == NULL)
		die("creating main window");

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

// MSDN doesn't list a constant that only includes tooltips but says this and a few others do
#define wantedICCClasses (ICC_BAR_CLASSES)

static ULONG_PTR comctlManifestCookie;
static HMODULE comctl32;

// note that this is an 8-bit character string we're writing; see the encoding clause
static const char manifest[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">\n<assemblyIdentity\n    version=\"1.0.0.0\"\n    processorArchitecture=\"*\"\n    name=\"CompanyName.ProductName.YourApplication\"\n    type=\"win32\"\n/>\n<description>Your application description here.</description>\n<dependency>\n    <dependentAssembly>\n        <assemblyIdentity\n            type=\"win32\"\n            name=\"Microsoft.Windows.Common-Controls\"\n            version=\"6.0.0.0\"\n            processorArchitecture=\"*\"\n            publicKeyToken=\"6595b64144ccf1df\"\n            language=\"*\"\n        />\n    </dependentAssembly>\n</dependency>\n</assembly>\n";

void initCommonControls(BOOL comctl5)
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

	if (!comctl5) {
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
