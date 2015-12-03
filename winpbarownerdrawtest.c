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
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>
#include <stdlib.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
	name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
	processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

HWND pbar;
HWND fake;
HWND slider;
HWND state;

#define fakepbarSetValue WM_USER
#define fakepbarSetState (WM_USER + 1)
struct fakepbar {
	WPARAM value;
	WPARAM state;
	HTHEME theme;
};

void fakepbarPaint(HWND hwnd, HDC dc, struct fakepbar *fp)
{
	RECT client;
	double nper;
	LONG width;
	LONG clientwidth;
	WPARAM ts;

#define dtb(part, state, w) client.right = client.left + w; DrawThemeBackground(fp->theme, dc, part, (int) (state), &client, NULL)
	GetClientRect(hwnd, &client);
	clientwidth = client.right - client.left;
	nper = ((double) clientwidth) / 100;
	width = (LONG) (nper * fp->value);
	ts = PBBS_NORMAL;
	if (fp->state == PBFS_PARTIAL)
		ts = PBBS_PARTIAL;
	dtb(PP_TRANSPARENTBAR, ts, clientwidth);
	dtb(PP_FILL, fp->state, width);
}

LRESULT CALLBACK fakepbarWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct fakepbar *fp;
	HDC dc;
	PAINTSTRUCT ps;

	fp = (struct fakepbar *) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (fp == NULL) {
		if (uMsg == WM_CREATE) {
			fp = (struct fakepbar *) malloc(sizeof (struct fakepbar));
			ZeroMemory(fp, sizeof (struct fakepbar));
			fp->state = PBFS_NORMAL;
			fp->theme = OpenThemeData(hwnd, VSCLASS_PROGRESS);
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) fp);
		}
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
	switch (uMsg) {
	case WM_THEMECHANGED:
		CloseThemeData(fp->theme);
		fp->theme = OpenThemeData(hwnd, VSCLASS_PROGRESS);
		// TODO reset animation timers?
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	case WM_PAINT:
		dc = BeginPaint(hwnd, &ps);
		fakepbarPaint(hwnd, dc, fp);
		EndPaint(hwnd, &ps);
		return 0;
	case fakepbarSetValue:
		fp->value = wParam;
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
	case fakepbarSetState:
		fp->state = wParam;
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void setValue(void)
{
	LRESULT value;

	value = SendMessageW(slider, TBM_GETPOS, 0, 0);
	SendMessageW(fake, fakepbarSetValue, (WPARAM) value, 0);
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
	WPARAM sf;

	switch (SendMessageW(state, CB_GETCURSEL, 0, 0)) {
	case 0:
		s = PBST_NORMAL;
		sf = PBFS_NORMAL;
		break;
	case 1:
		s = PBST_ERROR;
		sf = PBFS_ERROR;
		break;
	case 2:
		s = PBST_PAUSED;
		sf = PBFS_PAUSED;
		break;
	case 3:
		s = 4;
		sf = PBFS_PARTIAL;
		break;
	case 4:
		s = 0;
		sf = 0;
		break;
	}
	SendMessageW(pbar, PBM_SETSTATE, s, 0);
	SendMessageW(fake, fakepbarSetState, sf, 0);
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
	r.bottom = 10 + 20 + 5 + 20 + 5 + 30 + 5 + 20 + 10;
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

	ZeroMemory(&wc, sizeof (WNDCLASSEX));
	wc.lpszClassName = L"fakepbar";
	wc.lpfnWndProc = fakepbarWndProc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	if (RegisterClassW(&wc) == 0)
		abort();
	fake = CreateWindowExW(0,
		L"fakepbar", L"",
		WS_CHILD | WS_VISIBLE,
		10, 10 + 20 + 5,
		400, 20,
		mainwin, NULL, NULL, NULL);
	if (fake == NULL)
		abort();

	slider = CreateWindowExW(0,
		TRACKBAR_CLASSW, L"",
		TBS_HORZ | TBS_TOOLTIPS | TBS_TRANSPARENTBKGND | WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		10, 10 + 20 + 5 + 20,
		400, 30,
		mainwin, NULL, NULL, NULL);
	if (slider == NULL)
		abort();
	SendMessageW(slider, TBM_SETRANGEMIN, (WPARAM) TRUE, 0);
	SendMessageW(slider, TBM_SETRANGEMAX, (WPARAM) TRUE, 100);
	SendMessageW(slider, TBM_SETPOS, (WPARAM) TRUE, 0);

	state = CreateWindowExW(0,
		WC_COMBOBOXEXW, L"",
		CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
		10, 10 + 20 + 5 + 20 + 5 + 30 + 5,
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
