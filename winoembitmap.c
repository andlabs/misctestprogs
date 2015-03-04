// 3 march 2015
#define UNICODE
#define _UNICODE
#define STRICT
#define OEMRESOURCE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// #qo LIBS: user32 kernel32 gdi32

void panic(char *why)
{
	fprintf(stderr, "error %s: %I32u\n", why, GetLastError());
	abort();
}

struct {
	WCHAR *label;
	LPCWSTR resname;
} oemids[] = {
#define entry(nn) { L ## #nn, MAKEINTRESOURCE(nn) }
	entry(OBM_BTNCORNERS),
	entry(OBM_BTSIZE),
	entry(OBM_CHECK),
	entry(OBM_CHECKBOXES),
	entry(OBM_CLOSE),
	entry(OBM_COMBO),
	entry(OBM_DNARROW),
	entry(OBM_DNARROWD),
	entry(OBM_DNARROWI),
	entry(OBM_LFARROW),
	entry(OBM_LFARROWD),
	entry(OBM_LFARROWI),
	entry(OBM_MNARROW),
	entry(OBM_OLD_CLOSE),
	entry(OBM_OLD_DNARROW),
	entry(OBM_OLD_LFARROW),
	entry(OBM_OLD_REDUCE),
	entry(OBM_OLD_RESTORE),
	entry(OBM_OLD_RGARROW),
	entry(OBM_OLD_UPARROW),
	entry(OBM_OLD_ZOOM),
	entry(OBM_REDUCE),
	entry(OBM_REDUCED),
	entry(OBM_RESTORE),
	entry(OBM_RESTORED),
	entry(OBM_RGARROW),
	entry(OBM_RGARROWD),
	entry(OBM_RGARROWI),
	entry(OBM_SIZE),
	entry(OBM_UPARROW),
	entry(OBM_UPARROWD),
	entry(OBM_UPARROWI),
	entry(OBM_ZOOM),
	entry(OBM_ZOOMD),
#undef entry
	{ NULL, NULL },
};

HWND mainwin;
HWND combobox;
HINSTANCE hInstance;
HBITMAP current = NULL;
int curwidth, curheight;

#define margin 7
#define padding 4
#define comboHeight 16

void draw(HDC dc)
{
	HDC cdc;
	HBITMAP prev;

	if (current == NULL)
		return;
	cdc = CreateCompatibleDC(dc);
	if (cdc == NULL)
		panic("calling CreateCompatibleDC() in draw()");
	prev = SelectObject(cdc, current);
	if (prev == NULL)
		panic("selecting the current bitmap in draw()");
	// TODO why is just padding not enough?
	if (BitBlt(dc, margin, margin + comboHeight + padding * 3,
		curwidth, curheight,
		cdc, 0, 0, SRCCOPY) == 0)
		panic("calling BitBlt() in draw()");
	if (SelectObject(cdc, prev) != current)
		panic("restoring previous bitmap in draw()");
	if (DeleteDC(cdc) == 0)
		panic("deleting the compatible DC in draw()");
}

void load(LRESULT which)
{
	BITMAP b;

	if (current != NULL)
		if (DeleteObject(current) == 0)
			panic("freeing old bitmap in load()");
	current = LoadBitmap(NULL, oemids[which].resname);
	if (current == NULL)
		panic("loading new bitmap in load()");
	ZeroMemory(&b, sizeof (BITMAP));
	if (GetObject(current, sizeof (BITMAP), &b) == 0)
		panic("getting new bitmap dimensions in load()");
	curwidth = b.bmWidth;
	curheight = b.bmHeight;
	if (InvalidateRect(mainwin, NULL, TRUE) == 0)
		panic("queueing main window for redraw in load()");
}

void makeCombobox(void)
{
	int i;
	LRESULT index;

	combobox = CreateWindowExW(WS_EX_CLIENTEDGE,
		L"combobox", L"",
		CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL,
		margin, margin,
		200, comboHeight,
		mainwin, (HMENU) 100, hInstance, NULL);
	if (combobox == NULL)
		panic("creating combobox in makeCombobox()");
	for (i = 0; oemids[i].label != NULL; i++) {
		index = SendMessage(combobox, CB_ADDSTRING,
			0, (LPARAM) (oemids[i].label));
		if (index == CB_ERR)
			panic("adding item to combobox in makeCombobox()");
		else if (index == CB_ERRSPACE)
			panic("memory exhausted adding item to combobox in makeCombobox()");
	}
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC dc;
	PAINTSTRUCT ps;

	switch (uMsg) {
	case WM_PAINT:
		dc = BeginPaint(hwnd, &ps);
		if (dc == NULL)
			panic("calling BeginPaint() in wndproc()");
		draw(dc);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_COMMAND:
		if (HIWORD(wParam) == CBN_SELCHANGE && lParam == (LPARAM) combobox) {
			load(SendMessageW(combobox, CB_GETCURSEL, 0, 0));
			return 0;
		}
		break;		// fall through to DefWindowProc()
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	WNDCLASSW wc;
	MSG msg;

	hInstance = GetModuleHandle(NULL);
	if (hInstance == NULL)
		panic("calling GetModuleHandle() in main()");

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	if (wc.hCursor == NULL)
		panic("calling LoadCursorW() in main()");
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	if (wc.hIcon == NULL)
		panic("calling LoadIconW() in main()");
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClassW(&wc) == 0)
		panic("registering main window class in main()");

	mainwin = CreateWindowExW(0,
		L"mainwin", L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 600,
		NULL, NULL, hInstance, NULL);
	if (mainwin == NULL)
		panic("creating main window in main()");

	makeCombobox();

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	if (UpdateWindow(mainwin) == 0)
		panic("calling UpdateWindow() in main()");

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
