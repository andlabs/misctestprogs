// 5 march 2015
#include "dll.h"

static HINSTANCE hInstance;

__declspec(dllexport) ATOM registerClass(void)
{
	WNDCLASSW wc;

	fprintf(stderr, "in registerClass()\n");
	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = windowClass;
	wc.lpfnWndProc = DefWindowProcW;
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);		// TODO correct? (check list view behavior on COLOR_WINDOW change)
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInstance;
	return RegisterClassW(&wc);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		hInstance = hinstDLL;
	return TRUE;
}
