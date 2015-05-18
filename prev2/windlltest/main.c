// 5 march 2015
#include "dll.h"

void die(char *why)
{
	fprintf(stderr, "error %s: %I32d\n", why, GetLastError());
	abort();
}

int main(void)
{
	HWND hwnd;
	MSG msg;

	if (registerClass() == 0)
		die("registering class");
	fprintf(stderr, "class registered\n");
	hwnd = CreateWindowExW(0,
		windowClass, L"test",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, GetModuleHandle(NULL), NULL);
	if (hwnd == NULL)
		die("creating window");
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	if (UpdateWindow(hwnd) == 0)
		die("updating window");
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
