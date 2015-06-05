// 5 june 2015
// based on windtmgetidealsize.c 3 june 2015
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define CINTERFACE
#define WINVER 0x0600
#define _WIN32_WINNT 0x0600
#define _WIN32_WINDOWS 0x0600
#define _WIN32_IE 0x0700
#define NTDDI_VERSION 0x06000000
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

HWND mainwin;
HWND rb[20];
int cur = 0;
HWND addButton;

HFONT lfMessageFont;

void repositionButton(void)
{
	MoveWindow(addButton, 10, 10 + (cur * 16), 100, 20, TRUE);
}

void append(WCHAR *label)
{
	DWORD style;

	style = 0;
	if (cur == 0)
		style = WS_GROUP | WS_TABSTOP;
	rb[cur] = CreateWindowExW(0,
		L"button", label,
		style | BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE,
		10, 10 + (cur * 16),
		200, 16,
		mainwin, NULL, GetModuleHandle(NULL), NULL);
	SendMessageW(rb[cur], WM_SETFONT, (WPARAM) lfMessageFont, (LPARAM) TRUE);
	if (cur > 0)
		SetWindowPos(rb[cur], rb[cur - 1], 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOSIZE);
	cur++;
	repositionButton();
}

void tryToggle(HWND hwnd)
{
	int i;
	WPARAM check;

	for (i = 0; i < cur; i++)
		if (rb[i] == hwnd)
			goto isrb;
	// not a radio button
	return;
isrb:
	for (i = 0; i < cur; i++) {
		check = BST_UNCHECKED;
		if (rb[i] == hwnd)
			check = BST_CHECKED;
		SendMessage(rb[i], BM_SETCHECK, check, 0);
	}
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int x, y;
	SIZE s;

	switch (uMsg) {
	case WM_COMMAND:
		if (HIWORD(wParam) != BN_CLICKED)
			break;
		if (lParam == (LPARAM) addButton)
			append(L"New Item");
		else
			tryToggle((HWND) lParam);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
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

	addButton = CreateWindowExW(0,
		L"button", L"Add Button",
		BS_PUSHBUTTON | WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		0, 0,
		100, 100,
		mainwin, NULL, GetModuleHandle(NULL), NULL);

	append(L"Item 1");
	append(L"Item 2");
	append(L"Item 3");
}

void initCommonControls(BOOL);

int main(int argc, char *argv[])
{
	WNDCLASSW wc;
	MSG msg;
	NONCLIENTMETRICSW ncm;

	initCommonControls(TRUE);

	ZeroMemory(&ncm, sizeof (NONCLIENTMETRICSW));
	ncm.cbSize = sizeof (NONCLIENTMETRICSW);
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof (NONCLIENTMETRICSW), &ncm, sizeof (NONCLIENTMETRICSW));
	lfMessageFont = CreateFontIndirectW(&(ncm.lfMessageFont));

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	makeWindows();

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0) > 0)
		if (IsDialogMessage(mainwin, &msg) == 0) {
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
	icc.dwICC = ICC_STANDARD_CLASSES | ICC_PROGRESS_CLASS | ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES | ICC_UPDOWN_CLASS | ICC_BAR_CLASSES | ICC_DATE_CLASSES;

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
