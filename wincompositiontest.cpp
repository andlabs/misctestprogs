// 12 december 2016
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
// get Windows version right; right now Windows Vista
// unless otherwise stated, all values from Microsoft's sdkddkver.h
// TODO is all of this necessary? how is NTDDI_VERSION used?
// TODO plaform update sp2
#define WINVER			0x0600	/* from Microsoft's winnls.h */
#define _WIN32_WINNT		0x0600
#define _WIN32_WINDOWS	0x0600	/* from Microsoft's pdh.h */
#define _WIN32_IE			0x0700
#define NTDDI_VERSION		0x06000000
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <windowsx.h>
#include <shobjidl.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <usp10.h>
#include <msctf.h>
#include <textstor.h>
#include <olectl.h>
#include <shlwapi.h>
#include <dwmapi.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <inttypes.h>
#include <vector>
#include <map>
#include <string>

#pragma comment(linker, \
	"\"/manifestdependency:type='Win32' " \
	"name='Microsoft.Windows.Common-Controls' " \
	"version='6.0.0.0' " \
	"processorArchitecture='*' " \
	"publicKeyToken='6595b64144ccf1df' " \
	"language='*'\"")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dwmapi.lib")

#define HR(call) printf("%s -> 0x%I32X\n", #call, call)

struct metrics {
	RECT windowRect;
	MARGINS resizeFrameInsets;
	MARGINS nonclientInsets;
	MARGINS realNonclientInsets;
	RECT effectiveClientRect;
	RECT relativeClientRect;
};

BOOL defWindowProcFirst = TRUE;

// TODO this is incorrect when maximized
void getMetrics(HWND hwnd, struct metrics *m)
{
	RECT r;

	GetWindowRect(hwnd, &(m->windowRect));

	// get the margins of the resize frame
	ZeroMemory(&r, sizeof (RECT));
	AdjustWindowRectEx(&r,
		GetWindowStyle(hwnd) & ~WS_CAPTION,
		FALSE,
		GetWindowExStyle(hwnd));
	m->resizeFrameInsets.cxLeftWidth = -r.left;
	m->resizeFrameInsets.cyTopHeight = -r.top;
	m->resizeFrameInsets.cxRightWidth = r.right;
	m->resizeFrameInsets.cyBottomHeight = r.bottom;

	// get non-client insets
	ZeroMemory(&r, sizeof (RECT));
	AdjustWindowRectEx(&r,
		GetWindowStyle(hwnd),
		FALSE,
		GetWindowExStyle(hwnd));
	m->nonclientInsets.cxLeftWidth = -r.left;
	m->nonclientInsets.cyTopHeight = -r.top;
	m->nonclientInsets.cxRightWidth = r.right;
	m->nonclientInsets.cyBottomHeight = r.bottom;
	if (defWindowProcFirst) {
		m->nonclientInsets.cxLeftWidth = 0;
		m->nonclientInsets.cxRightWidth = 0;
		m->nonclientInsets.cyBottomHeight = 0;
	}

	// give the top 2.5x the room so we can shove stuff in there
	m->realNonclientInsets = m->nonclientInsets;
	m->realNonclientInsets.cyTopHeight *= 2.5;

	// compute the effective client rect
	m->effectiveClientRect = m->windowRect;
	m->effectiveClientRect.left += m->realNonclientInsets.cxLeftWidth;
	m->effectiveClientRect.top += m->realNonclientInsets.cyTopHeight;
	m->effectiveClientRect.right -= m->realNonclientInsets.cxRightWidth;
	m->effectiveClientRect.bottom -= m->realNonclientInsets.cyBottomHeight;

	// and compute it relative to the window's real client rect
	m->relativeClientRect = m->effectiveClientRect;
	MapWindowRect(NULL, hwnd, &(m->relativeClientRect));

#if 0
// for debugging
printf("***\n");
#define PRINTRECT(r) ((int)((r).left)), ((int)((r).top)), ((int)((r).right)), ((int)((r).bottom))
printf("window rect %d %d %d %d\n", PRINTRECT(m->windowRect));
	ZeroMemory(&r, sizeof (RECT));
	AdjustWindowRectEx(&r,
		GetWindowStyle(hwnd),
		FALSE,
		GetWindowExStyle(hwnd));
r.left=-r.left;r.top=-r.top;
printf("edge insets %d %d %d %d\n", PRINTRECT(r));
HR(DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof (RECT)));
printf("DWMWA_EXTENDED_FRAME_BOUNDS %d %d %d %d\n", PRINTRECT(r));
{HMODULE m;
m=LoadLibraryW(L"kernel32.dll");
if(m == NULL)printf("unknown os\n");
// TODO this doesn't work; apparently the function is really in one of the api-ms-core* DLLs...
else if(GetProcAddress(m,"VirtualAllocFromApp")!=NULL)printf("windows 10\n");
else if(GetProcAddress(m,"GetPackageApplicationIds")!=NULL)printf("windows 8.1\n");
else if(GetProcAddress(m,"GetSystemTimePreciseAsFileTime")!=NULL)printf("windows 8\n");
else printf("windows 7\n");}
printf("\n");
#endif
}

HWND rebarHost;
HWND rebar;

const char *htnames[] = {
	"HTERROR",
	"HTTRANSPARENT",
	"HTNOWHERE",
	"HTCLIENT",
	"HTCAPTION",
	"HTSYSMENU",
	"HTGROWBOX",
	"HTMENU",
	"HTHSCROLL",
	"HTVSCROLL",
	"HTMINBUTTON",
	"HTMAXBUTTON",
	"HTLEFT",
	"HTRIGHT",
	"HTTOP",
	"HTTOPLEFT",
	"HTTOPRIGHT",
	"HTBOTTOM",
	"HTBOTTOMLEFT",
	"HTBOTTOMRIGHT",
	"HTBORDER",
	"HTOBJECT",
	"HTCLOSE",
	"HTHELP",
};

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct metrics m;
	HDC dc;
	PAINTSTRUCT ps;
	BOOL dwmHandled;
	LRESULT lResult;

	dwmHandled = DwmDefWindowProc(hwnd, uMsg, wParam, lParam, &lResult);
	getMetrics(hwnd, &m);
	switch (uMsg) {
	case WM_CREATE:
		SetWindowPos(hwnd, NULL,
			m.windowRect.left, m.windowRect.top,
			m.windowRect.right - m.windowRect.left, m.windowRect.bottom - m.windowRect.top,
			SWP_FRAMECHANGED);
		// TODO if we pass SWP_NOOWNERZORDER || SWP_NOZORDER, the default frame is not correctly inhibited

		rebarHost = CreateWindowExW(0,
			L"rebarHost", L"",
			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			m.realNonclientInsets.cxLeftWidth,
			m.nonclientInsets.cyTopHeight,
			m.windowRect.right - m.windowRect.left -
				m.realNonclientInsets.cxLeftWidth - m.realNonclientInsets.cxRightWidth,
			m.realNonclientInsets.cyTopHeight - m.nonclientInsets.cyTopHeight,
			hwnd, NULL, GetModuleHandle(NULL), NULL);

		rebar = CreateWindowExW(0,
			REBARCLASSNAMEW, L"",
			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | RBS_VARHEIGHT | CCS_NODIVIDER,
			0, 0, 0, 0,
			rebarHost, NULL, GetModuleHandle(NULL), NULL);

		{
			REBARBANDINFOW rb;

			ZeroMemory(&rb, sizeof (REBARBANDINFOW));
			rb.cbSize = sizeof (REBARBANDINFOW);
			rb.fMask = RBBIM_TEXT;
			rb.lpText = L"This is a rebar";
			HR((HRESULT) SendMessageW(rebar, RB_INSERTBANDW, (WPARAM) (-1), (LPARAM) (&rb)));
		}

		SendMessageW(rebar, RB_SETWINDOWTHEME, 0,
			(LPARAM) L"NavbarComposited");

		break;
	case WM_ACTIVATE:
		HR(DwmExtendFrameIntoClientArea(hwnd, &(m.realNonclientInsets)));
		break;
	case WM_NCCALCSIZE:
		if (wParam != (WPARAM) FALSE) {
			NCCALCSIZE_PARAMS *op = (NCCALCSIZE_PARAMS *) lParam;
			NCCALCSIZE_PARAMS np;

			if (!defWindowProcFirst)
				return 0;
			np = *op;
			DefWindowProcW(hwnd, uMsg, wParam, (LPARAM) (&np));
			printf("old %ld %ld %ld %ld\nnew %ld %ld %ld %ld\n",
				op->rgrc[0].left, op->rgrc[0].top, op->rgrc[0].right, op->rgrc[0].bottom,
				np.rgrc[0].left, np.rgrc[0].top, np.rgrc[0].right, np.rgrc[0].bottom);
			op->rgrc[0].left = np.rgrc[0].left;
			op->rgrc[0].right = np.rgrc[0].right;
			op->rgrc[0].bottom = np.rgrc[0].bottom;
			return 0;
		}
		break;
	case WM_NCHITTEST:
		if (dwmHandled)
			return lResult;
		// DWM did not handle it; we have to do it ourselves
		if (defWindowProcFirst) {
			lResult = DefWindowProcW(hwnd, uMsg, wParam, lParam);
			if (lResult != HTCLIENT) {
				printf("them %s\n", htnames[lResult + 2]);
				return lResult;
			}
		}
		{
			POINT p;

			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);

			lResult = HTNOWHERE;
			if (p.y >= m.windowRect.top && p.y < (m.windowRect.top + m.resizeFrameInsets.cyTopHeight))
				lResult = HTTOP;
			else if (p.y >= m.effectiveClientRect.bottom && p.y < m.windowRect.bottom)
				lResult = HTBOTTOM;

			if (p.x >= m.windowRect.left && p.x < m.effectiveClientRect.left)
				switch (lResult) {
				case HTNOWHERE:
					lResult = HTLEFT;
					break;
				case HTTOP:
					lResult = HTTOPLEFT;
					break;
				case HTBOTTOM:
					lResult = HTBOTTOMLEFT;
					break;
				}
			else if (p.x >= m.effectiveClientRect.right && p.x < m.windowRect.right)
				switch (lResult) {
				case HTNOWHERE:
					lResult = HTRIGHT;
					break;
				case HTTOP:
					lResult = HTTOPRIGHT;
					break;
				case HTBOTTOM:
					lResult = HTBOTTOMRIGHT;
					break;
				}

			if (lResult == HTNOWHERE)
				if (p.y >= (m.windowRect.top + m.resizeFrameInsets.cyTopHeight) && p.y < m.effectiveClientRect.top)
					lResult = HTCAPTION;

			if (defWindowProcFirst)
				printf("us %s\n", htnames[lResult + 2]);
			if (lResult != HTNOWHERE)
				return lResult;
		}
		// we can't handle it; give it to DefWindowProcW() and hope for the best
		break;
	case WM_SIZE:
		// TODO this seems to be wrong when shrinking the size on the right or bottom edges
		// TODO without this call, the WM_PAINT never fills new areas
		// we may need to handle WM_WINDOWPOSCHANGED and compute new metrics from there
		InvalidateRect(hwnd, &(m.relativeClientRect), FALSE);
		break;
	case WM_PAINT:
		dc = BeginPaint(hwnd, &ps);
		FillRect(dc, &(m.relativeClientRect), (HBRUSH) (COLOR_BTNFACE + 1));
		EndPaint(hwnd, &ps);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}
	if (dwmHandled)
		return lResult;
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	INITCOMMONCONTROLSEX icc;
	WNDCLASSW wc;
	HWND mainwin;
	MSG msg;

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = (ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_BAR_CLASSES | ICC_TAB_CLASSES | ICC_UPDOWN_CLASS | ICC_PROGRESS_CLASS | ICC_HOTKEY_CLASS | ICC_ANIMATE_CLASS | ICC_WIN95_CLASSES | ICC_DATE_CLASSES | ICC_USEREX_CLASSES | ICC_COOL_CLASSES | ICC_INTERNET_CLASSES | ICC_PAGESCROLLER_CLASS | ICC_NATIVEFNTCTL_CLASS | ICC_STANDARD_CLASSES | ICC_LINK_CLASS);
	InitCommonControlsEx(&icc);

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	RegisterClassW(&wc);

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"rebarHost";
	wc.lpfnWndProc = DefWindowProcW;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	RegisterClassW(&wc);

	mainwin = CreateWindowExW(0,
		L"mainwin", L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		400, 400,
		NULL, NULL, GetModuleHandle(NULL), NULL);

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return 0;
}
