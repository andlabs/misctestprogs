// 6 december 2015
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
// get Windows version right; right now Windows Vista
#define WINVER 0x0600				/* according to Microsoft's winnls.h */
#define _WIN32_WINNT 0x0600		/* according to Microsoft's sdkddkver.h */
#define _WIN32_WINDOWS 0x0600		/* according to Microsoft's pdh.h */
#define _WIN32_IE 0x0700			/* according to Microsoft's sdkddkver.h */
#define NTDDI_VERSION 0x06000000	/* according to Microsoft's sdkddkver.h */
#include <windows.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <string.h>
#include <stdlib.h>
#include <typeinfo>
#include <time.h>

HWND mainwin;
std::condition_variable cv;
std::mutex m;
std::unique_lock<std::mutex> ourlock(m);
std::thread *timeThread;

bool wait(void)
try {
	return cv.wait_for(ourlock, std::chrono::seconds(1)) == std::cv_status::timeout;
} catch (const std::exception &e) {
	fprintf(stderr, "%s caught: %s\n", typeid (e).name(), e.what());
	return false;		// kill the thread
}

void threadproc(void)
{
	while (wait())
		PostMessageW(mainwin, WM_APP, 0, 0);
}

HWND edit;

void appendline(const WCHAR *wc)
{
	LRESULT n;

	n = SendMessageW(edit, WM_GETTEXTLENGTH, 0, 0);
	SendMessageW(edit, EM_SETSEL, n, n);
	SendMessageW(edit, EM_REPLACESEL, FALSE, (LPARAM) wc);
}

HWND button;

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		if (lParam == (LPARAM) button)
			appendline(L"Saying something\n");
		break;
	case WM_APP:
		appendline(L"One second passed\n");
		break;
	case WM_CLOSE:
		cv.notify_all();
		timeThread->join();
		PostQuitMessage(0);
		break;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	WNDCLASSW wc;
	RECT r;
	MSG msg;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	r.left = 0;
	r.top = 0;
	r.right = 10 + 300 + 10;
	r.bottom = 10 + 20 + 5 + 195 + 10;
	AdjustWindowRectEx(&r, WS_OVERLAPPEDWINDOW, FALSE, 0);
	mainwin = CreateWindowExW(0,
		L"mainwin", L"mainwin",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		r.right - r.left, r.bottom - r.top,
		NULL, NULL, NULL, NULL);

	button = CreateWindowExW(0,
		L"button", L"Say Something",
		BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
		10, 10,
		300, 20,
		mainwin, NULL, NULL, NULL);

	edit = CreateWindowExW(WS_EX_CLIENTEDGE,
		L"edit", L"",
		ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE | ES_READONLY | ES_WANTRETURN | WS_CHILD | WS_VISIBLE | WS_VSCROLL,
		10, 10 + 20 + 5,
		300, 195,
		mainwin, NULL, NULL, NULL);

	timeThread = new std::thread(threadproc);

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return 0;
}
