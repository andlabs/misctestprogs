// 20 june 2016
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define WINVER			0x0600
#define _WIN32_WINNT		0x0600
#define _WIN32_WINDOWS	0x0600
#define _WIN32_IE			0x0700
#define NTDDI_VERSION		0x06000000
#include <windows.h>
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>

HWND mainwin;
HWND edit;

void redraw(void)
{
	HDC dc;
	PAINTSTRUCT ps;
	WCHAR text[1024];
	HTHEME theme;
	LOGFONTW lf;
	HFONT font, oldfont;
	COLORREF color;
	SIZE extent;
	RECT r;

	dc = BeginPaint(mainwin, &ps);

	theme = OpenThemeData(mainwin, VSCLASS_AEROWIZARD);
	GetThemeFont(theme, dc, AW_HEADERAREA, 0, TMT_FONT, &lf);
	font = CreateFontIndirectW(&lf);
	GetThemeColor(theme, AW_HEADERAREA, 0, TMT_TEXTCOLOR, &color);

	GetWindowTextW(edit, text, 1024);
	GetThemeTextExtent(theme, dc, AW_HEADERAREA, 0,
		text, -1, 0, NULL, &r);
	extent.cx = r.right - r.left;
	extent.cy = r.bottom - r.top;

	SetWindowPos(edit, NULL,
		10, 10, 500, 20,
		SWP_NOOWNERZORDER | SWP_NOZORDER);

	r.left = 10;
	r.top = 40;
	r.right = 10 + extent.cx;
	r.bottom = r.top + extent.cy;
	DrawThemeText(theme, dc, AW_HEADERAREA, 0,
		text, -1, 0, 0, &r);

	oldfont = (HFONT) SelectObject(dc, font);
	color = SetTextColor(dc, color);
	r.top = r.bottom + 10;
	r.bottom = r.top + extent.cy;
	DrawTextW(dc, text, -1, &r, 0);
	SetTextColor(dc, color);
	SelectObject(dc, oldfont);
	DeleteObject(font);

	CloseThemeData(theme);
	EndPaint(mainwin, &ps);
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_PAINT:
		redraw();
		return 0;
	case WM_COMMAND:
		InvalidateRect(hwnd, NULL, TRUE);
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
	MSG msg;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	mainwin = CreateWindowExW(0,
		L"mainwin", L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		640, 200,
		NULL, NULL, NULL, NULL);

	edit = CreateWindowExW(WS_EX_CLIENTEDGE,
		L"EDIT", L"Top is DrawThemeText(), bottom is manual",
		WS_CHILD | WS_VISIBLE,
		10, 10, 100, 100,
		mainwin, (HMENU) 100, NULL, NULL);

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return 0;
}
