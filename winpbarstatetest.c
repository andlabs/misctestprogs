// 3 december 2015
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
// for the manifest
#define ISOLATION_AWARE_ENABLED 1
// get Windows version right; right now Windows Vista
#define WINVER 0x0600				/* according to Microsoft's winnls.h */
#define _WIN32_WINNT 0x0600		/* according to Microsoft's sdkddkver.h */
#define _WIN32_WINDOWS 0x0600		/* according to Microsoft's pdh.h */
#define _WIN32_IE 0x0700			/* according to Microsoft's sdkddkver.h */
#define NTDDI_VERSION 0x06000000	/* according to Microsoft's sdkddkver.h */
#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
	name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
	processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

HWND pbar;
HWND slider;
HWND state;

void setValue(void)
{
	LRESULT value;

	value = SendMessageW(slider, TBM_GETPOS, 0, 0);
	// avoid animation
	if (value == 100) {			// because we can't 101
		SendMessageW(pbar, PBM_SETRANGE32, 0, 101);
		SendMessageW(pbar, PBM_SETPOS, 101, 0);
		SendMessageW(pbar, PBM_SETPOS, 100, 0);
		SendMessageW(pbar, PBM_SETRANGE32, 0, 100);
		return;
	}
	SendMessageW(pbar, PBM_SETPOS, (WPARAM) (value + 1), 0);
	SendMessageW(pbar, PBM_SETPOS, (WPARAM) value, 0);
}

void addItem(WCHAR *item)
{
	COMBOBOXEXITEMW i;

	ZeroMemory(&i, sizeof (COMBOBOXEXITEMW));
	i.mask = CBEIF_TEXT;
	i.iItem = -1;
	i.pszText = item;
	if (SendMessageW(state, CBEM_INSERTITEMW, 0, (LPARAM) (&i)) == (LRESULT) (-1))
		abort();
}

void setState(void)
{
	WPARAM s;

	switch (SendMessageW(state, CB_GETCURSEL, 0, 0)) {
	case 0:
		s = PBST_NORMAL;
		break;
	case 1:
		s = PBST_ERROR;
		break;
	case 2:
		s = PBST_PAUSED;
		break;
	case 3:
		s = 4;
		break;
	case 4:
		s = 0;
		break;
	}
	SendMessageW(pbar, PBM_SETSTATE, s, 0);
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_HSCROLL:
		if (lParam == ((LPARAM) slider))
			setValue();
		break;
	case WM_COMMAND:
		if (lParam == ((LPARAM) state) && HIWORD(wParam) == CBN_SELCHANGE)
			setState();
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	WNDCLASSW wc;
	HWND mainwin;
	RECT r;
	INITCOMMONCONTROLSEX icc;
	MSG msg;

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_PROGRESS_CLASS |
		ICC_PROGRESS_CLASS |
		ICC_USEREX_CLASSES;
	if (InitCommonControlsEx(&icc) == 0)
		abort();

	ZeroMemory(&wc, sizeof (WNDCLASSEX));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClassW(&wc) == 0)
		abort();

	r.left = 0;
	r.top = 0;
	r.right = 10 + 400 + 10;
	r.bottom = 10 + 20 + 5 + 20 + 5 + 20 + 10;
	if (AdjustWindowRectEx(&r, WS_OVERLAPPEDWINDOW, FALSE, 0) == 0)
		abort();
	mainwin = CreateWindowExW(0,
		L"mainwin", L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		r.right - r.left, r.bottom - r.top,
		NULL, NULL, NULL, NULL);
	if (mainwin == NULL)
		abort();

	pbar = CreateWindowExW(0,
		PROGRESS_CLASSW, L"",
		PBS_SMOOTH | WS_CHILD | WS_VISIBLE,
		10, 10,
		400, 20,
		mainwin, NULL, NULL, NULL);
	if (pbar == NULL)
		abort();
	SendMessageW(pbar, PBM_SETRANGE32, 0, 100);
	SendMessageW(pbar, PBM_SETPOS, 0, 0);
	SendMessageW(pbar, PBM_SETSTATE, (WPARAM) PBST_NORMAL, 0);

	slider = CreateWindowExW(0,
		TRACKBAR_CLASSW, L"",
		TBS_HORZ | TBS_TOOLTIPS | TBS_TRANSPARENTBKGND | WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		10, 10 + 20 + 5,
		400, 20,
		mainwin, NULL, NULL, NULL);
	if (slider == NULL)
		abort();
	SendMessageW(slider, TBM_SETRANGEMIN, (WPARAM) TRUE, 0);
	SendMessageW(slider, TBM_SETRANGEMAX, (WPARAM) TRUE, 100);
	SendMessageW(slider, TBM_SETPOS, (WPARAM) TRUE, 0);

	state = CreateWindowExW(0,
		WC_COMBOBOXEXW, L"",
		CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
		10, 10 + 20 + 5 + 20 + 5,
		400, 20,
		mainwin, NULL, NULL, NULL);
	if (state == NULL)
		abort();
	addItem(L"Normal");
	addItem(L"Error");
	addItem(L"Paused");
	addItem(L"4 (PBFS_PARTIAL)");
	addItem(L"0 (PBFS_PARTIAL)");
	if (SendMessageW(state, CB_SETCURSEL, 0, 0) == (LRESULT) (CB_ERR))
		abort();

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return 0;
}
