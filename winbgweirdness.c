// pietro gagliardi - 11-12 april 2014
#define _UNICODE
#define UNICODE
#include <stdio.h>
#include <windows.h>

void paintwin(HWND hwnd)
{	
	PAINTSTRUCT ps;

	HDC hdc = BeginPaint(hwnd, &ps);
	RECT updaterect = ps.rcPaint;

	// this is the DC that will be drawn to hdc
	HDC rdc = CreateCompatibleDC(hdc);
	HBITMAP rbitmap = CreateCompatibleBitmap(rdc,
			updaterect.right - updaterect.left,
			updaterect.bottom - updaterect.top);
	HBITMAP prevrbitmap = SelectObject(rdc, rbitmap);
	RECT rrect = { 0, 0, updaterect.right - updaterect.left, updaterect.bottom - updaterect.top };
	FillRect(rdc, &rrect, (HBRUSH) (COLOR_BTNFACE + 1));

#if 0
	// this is the alpha-premultiplied RGBA image
	// taken from elsewhere
	int imagewid, imageht;
	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof (bi));
	bi.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = imagewid;
	bi.bmiHeader.biHeight = -imageht;		// negative height to force top-down drawing
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = imagewid * iamgeht * 4;
	uint8_t *ppvBits;
	ibitmap = CreateDIBSection(
		NULL,
		&bi,
		DIB_RGB_COLORS,
		&ppvBits,
		NULL, NULL);

	// fill ppvBits here

	HDC idc = CreateCompatibleDC(hdc)
	HBITMAP previbitmap = SelectObject(idc, ibitmap);

	BLENDFUNCTION blendfunc;
	blendfunc.BlendOp = AC_SRC_OVER;
	blendfunc.BlendFlags = 0;
	blendfunc.SourceConstantAlpha = 255;
	blendfunc.AlphaFormat = AC_SRC_ALPHA;
	AlphaBlend(
		rdc, 0, 0, imagewid, imageht,
		idc, 0, 0, imagewid, imageht,
		blendfunc);
#endif

	BitBlt(hdc, updaterect.left, updaterect.top,
		updaterect.right - updaterect.left,
		updaterect.bottom - updaterect.top,
		rdc, 0, 0, SRCCOPY);

//	SelectObject(idc, previbitmap);
	SelectObject(rdc, prevrbitmap);
//	DeleteObject(ibitmap);
	DeleteObject(rbitmap);
//	DeleteDC(idc);
	DeleteDC(rdc);

	EndPaint(hwnd, &ps);
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_PAINT:
		paintwin(hwnd);
		return 0;
	case  WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int getnCmdShow()
{
	STARTUPINFO si;

	GetStartupInfo(&si);
	if ((si.dwFlags & STARTF_USESHOWWINDOW) != 0)
		return si.wShowWindow;
	return SW_SHOWDEFAULT;
}

int main(int argc, char *argv[])
{
	WNDCLASS cls;
	MSG msg;
	HWND mainwin;

	HINSTANCE hInstance = GetModuleHandle(NULL);
	int nCmdShow = getnCmdShow();

	ZeroMemory(&cls, sizeof (WNDCLASS));
	cls.lpszClassName = L"mainwin";
	cls.lpfnWndProc = wndproc;
	cls.hInstance = hInstance;
	cls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	cls.hCursor = LoadCursor(NULL, IDC_ARROW);
	cls.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClass(&cls) == 0) {
		fprintf(stderr, "registering window class failed: %lu\n", GetLastError());
		return 1;
	}

	mainwin = CreateWindowEx(0,
		L"mainwin", L"mainwin",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);
	if (mainwin == NULL) {
		fprintf(stderr, "opening main window failed: %lu", GetLastError());
		return 1;
	}
	ShowWindow(mainwin, nCmdShow);
	UpdateWindow(mainwin);

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
