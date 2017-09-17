// 17 september 2017
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
#include <dwrite.h>
#include <shlwapi.h>
#include <sstream>
#include <iomanip>
#include <time.h>

std::wostringstream messages;
bool done = false;

void addmsg(const WCHAR *name, WPARAM wParam, LPARAM lParam)
{
	time_t t;
	bool mods = false;

	if (done)
		return;
	time(&t);
	messages << ctime(&t);
	messages << L" " << name;
	messages << L" wParam " << std::hex << wParam;
	messages << L" lParam " << std::hex << lParam;
	messages << L" modifiers";
	if ((GetKeyState(VK_CONTROL) & 0x80) != 0) {
		mods = true;
		messages << L" Ctrl";
	}
	if ((GetKeyState(VK_MENU) & 0x80) != 0) {
		mods = true;
		messages << L" Alt";
	}
	if ((GetKeyState(VK_SHIFT) & 0x80) != 0) {
		mods = true;
		messages << L" Shift";
	}
	if (((GetKeyState(VK_LWIN) & 0x80) != 0) || ((GetKeyState(VK_RWIN) & 0x80) != 0)) {
		mods = true;
		messages << L" Win";
	}
	if (!mods)
		messages << L" (none)";
	messages << L"\r\n";
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC dc;
	PAINTSTRUCT ps;

	switch (uMsg) {
#define DO2(m, n) case m: addmsg(n, wParam, lParam); break;
#define DO(m) DO2(m, L ## #m)
	DO(WM_KEYDOWN)
	DO(WM_SYSKEYDOWN)
	DO2(WM_CHAR, L"WM_CHAR (key down)")
	DO2(WM_SYSCHAR, L"WM_SYSCHAR (key down)")
	DO(WM_KEYUP)
	DO(WM_SYSKEYUP)
	DO(WM_COMMAND)
	DO(WM_APPCOMMAND)
	DO(WM_SYSCOMMAND)
	case WM_PAINT:
		if (done)
			break;
		dc = BeginPaint(hwnd, &ps);
#define msg L"Click the window to stop logging."
		TextOutW(dc, 20, 20, msg, wcslen(msg));
#undef msg
		EndPaint(hwnd, &ps);
		return 0;
	case WM_LBUTTONDOWN:
		if (!done) {
			std::wstring text;

			done = true;
			text = messages.str();
			CreateWindowExW(WS_EX_CLIENTEDGE,
				L"edit", text.c_str(),
				ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | ES_WANTRETURN | ES_READONLY | WS_TABSTOP | WS_CHILD | WS_VSCROLL | WS_VISIBLE,
#define boxwid 500
#define boxht 500
				10, 10, boxwid, boxht,
				hwnd, (HMENU) 100, NULL, NULL);
			InvalidateRect(hwnd, NULL, TRUE);
		}
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	WNDCLASSW wc;
	HWND hwnd;
	RECT r;
	MSG msg;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	r.left = 0;
	r.top = 0;
	r.right = 10 + boxwid + 10;
	r.bottom = 10 + boxht + 10;
	AdjustWindowRectEx(&r,
		WS_OVERLAPPEDWINDOW,
		FALSE,
		0);
	hwnd = CreateWindowExW(0,
		L"mainwin", L"mainwin",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		r.right - r.left, r.bottom - r.top,
		NULL, NULL, NULL, NULL);
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);
	SetFocus(hwnd);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return 0;
}
