// 11 december 2014
#define UNICODE
#define _UNICODE
#define STRICT
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_WINDOWS 0x0501		/* according to Microsoft's winperf.h */
#define _WIN32_IE 0x0600			/* according to Microsoft's sdkddkver.h */
#define NTDDI_VERSION 0x05010000	/* according to Microsoft's sdkddkver.h */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

void printrgncode(int rgncode)
{
	switch (rgncode) {
	case ERROR:
		printf("ERROR\n");
		return;
	case NULLREGION:
		printf("NULLREGION\n");
		return;
	case SIMPLEREGION:
		printf("SIMPLEREGION\n");
		return;
	case COMPLEXREGION:
		printf("COMPLEXREGION\n");
		return;
	}
	printf("unknown region code %d\n", rgncode);
}

HINSTANCE hInstance;
int nCmdShow;

const char *testnames[] = {
	"ScrollWindowEx() before ShowWindow(nCmdShow)",
	"ScrollWindowEx() after ShowWindow(nCmdShow) and ShowWindow(SW_HIDE)",
	"ScrollWindowEx() after only ShowWindow(nCmdShow)",
};

void run(int i)
{
	HWND hwnd;

	hwnd = CreateWindowExW(0,
		L"mainwin", L"test",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		300, 300,
		NULL, NULL, hInstance, NULL);
	if (hwnd == NULL)
		abort();
	switch (i) {
	case 0:
		// do nothing
		break;
	case 1:
		ShowWindow(hwnd, nCmdShow);
		ShowWindow(hwnd, SW_HIDE);
		break;
	case 2:
		ShowWindow(hwnd, nCmdShow);
		break;
	}
	printf("%s: ", testnames[i]);
	printrgncode(ScrollWindowEx(hwnd, 20, 20,
		NULL, NULL, NULL, NULL,
		SW_ERASE | SW_INVALIDATE));
	if (DestroyWindow(hwnd) == 0)
		abort();
}

int main(void)
{
	STARTUPINFOW si;
	WNDCLASSW wc;

	hInstance = GetModuleHandle(NULL);
	if (hInstance == NULL)
		abort();
	nCmdShow = SW_SHOWDEFAULT;
	GetStartupInfoW(&si);
	if ((si.dwFlags & STARTF_USESHOWWINDOW) != 0)
		nCmdShow = si.wShowWindow;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = DefWindowProcW;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	if (RegisterClassW(&wc) == 0)
		abort();

	run(0);
	run(1);
	run(2);

	return 0;
}
