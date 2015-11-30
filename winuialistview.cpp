// 29 november 2015
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
#include <uiautomation.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
	name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
	processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

void log(const WCHAR *, ...);

#define must(x) { HRESULT hr; hr = (x); if (hr != S_OK) { printf("%08I32X\n", hr); abort(); } }

void tryPattern(WCHAR *name, PATTERNID pid, IUIAutomationElement *elem)
{
	IUnknown *unk;
	HRESULT hr;

	hr = elem->GetCurrentPattern(pid, &unk);
	if (hr != S_OK)
		log(L"    %s: no 0x%08I32X\n", name, hr);
	else if (unk == NULL)
		log(L"    %s: no\n", name);
	else {
		log(L"    %s: yes\n", name);
		unk->Release();
	}
}

void getinfo(IUIAutomationElement *elem)
{
	CONTROLTYPEID ctype;

	log(L"provider: %p\n", elem);
	must(elem->get_CurrentControlType(&ctype));
	log(L"control type: %d\n", (int) ctype);
	log(L"patterns:\n");
#define TP(x) tryPattern(L ## #x , x, elem)
	TP(UIA_AnnotationPatternId);
	tryPattern(L"UIA_CustomNavigationPatternId", 10033, elem);
	TP(UIA_DockPatternId);
	TP(UIA_DragPatternId);
	TP(UIA_DropTargetPatternId);
	TP(UIA_ExpandCollapsePatternId);
	TP(UIA_GridItemPatternId);
	TP(UIA_GridPatternId);
	TP(UIA_InvokePatternId);
	TP(UIA_ItemContainerPatternId);
	TP(UIA_LegacyIAccessiblePatternId);
	TP(UIA_MultipleViewPatternId);
	TP(UIA_ObjectModelPatternId);
	TP(UIA_RangeValuePatternId);
	TP(UIA_ScrollItemPatternId);
	TP(UIA_ScrollPatternId);
	TP(UIA_SelectionItemPatternId);
	TP(UIA_SelectionPatternId);
	TP(UIA_SpreadsheetPatternId);
	TP(UIA_SpreadsheetItemPatternId);
	TP(UIA_StylesPatternId);
	TP(UIA_SynchronizedInputPatternId);
	TP(UIA_TableItemPatternId);
	TP(UIA_TablePatternId);
	TP(UIA_TextChildPatternId);
	TP(UIA_TextEditPatternId);
	TP(UIA_TextPatternId);
	TP(UIA_TextPattern2Id);
	TP(UIA_TogglePatternId);
	TP(UIA_TransformPatternId);
	TP(UIA_TransformPattern2Id);
	TP(UIA_ValuePatternId);
	TP(UIA_VirtualizedItemPatternId);
	TP(UIA_WindowPatternId);
#undef TP
}

HWND setupListView(HWND parent, int x, int y, int width, int height)
{
	HWND hwnd;
	LVCOLUMNW col;
	LVITEMW item;
	int i, j;
	WCHAR text[100];

	hwnd = CreateWindowExW(WS_EX_CLIENTEDGE,
		WC_LISTVIEWW, L"Main Window",
		LVS_REPORT | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS | LVS_SINGLESEL | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,
		x, y,
		width, height,
		parent, NULL, GetModuleHandle(NULL), NULL);
	SendMessageW(hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	for (i = 0; i < 5; i++) {
		ZeroMemory(&col, sizeof (LVCOLUMNW));
		col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
		col.fmt = LVCFMT_LEFT;
		col.cx = 200;
		col.pszText = L"Column";
		SendMessage(hwnd, LVM_INSERTCOLUMN, (WPARAM) i, (LPARAM) (&col));
	}

	for (i = 0; i < 100; i++) {
		for (j = 0; j < 5; j++) {
			ZeroMemory(&item, sizeof (LVITEMW));
			item.mask = LVIF_TEXT;
			wsprintf(text, L"Item (%d,%d)", i, j);
			item.pszText = text;
			item.iItem = i;
			item.iSubItem = j;
			if (j == 0)
				SendMessageW(hwnd, LVM_INSERTITEM, 0, (LPARAM) (&item));
			else
				SendMessageW(hwnd, LVM_SETITEM, 0, (LPARAM) (&item));
		}
	}

	return hwnd;
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CLOSE)
		PostQuitMessage(0);
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

HWND edit;

int main(void)
{
	WNDCLASSW wc;
	HWND mainwin;
	HWND lv;
	IUIAutomation *uia;
	IUIAutomationElement *elem;
	INITCOMMONCONTROLSEX icc;
	RECT r;
	MSG msg;
	HRESULT hr;

	hr = CoInitialize(NULL);
	if (hr != S_OK && hr != S_FALSE)
		abort();
	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;
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
	r.right = 400 + 10 + 10;
	r.bottom = 400 + 10 + 10 + 5;
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

	lv = setupListView(mainwin, 10, 10, 400, 200);

	edit = CreateWindowExW(WS_EX_CLIENTEDGE,
		L"edit", L"",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_READONLY,
		10, 10 + 200 + 5, 400, 200,
		mainwin, NULL, NULL, NULL);
	if (edit == NULL)
		abort();

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	hr = CoCreateInstance(CLSID_CUIAutomation, NULL,
		CLSCTX_INPROC_SERVER, IID_IUIAutomation, (LPVOID *) (&uia));
	if (hr != S_OK)
		abort();
	hr = uia->ElementFromHandle(lv, &elem);
	if (hr != S_OK)
		abort();
	getinfo(elem);
	elem->Release();
	uia->Release();

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	CoUninitialize();
	return 0;
}

void log(const WCHAR *msg, ...)
{
	va_list ap;
	WCHAR buf[1024];
	LPARAM n;

	va_start(ap, msg);
	// TODO safe
	wvsprintf(buf, msg, ap);
	va_end(ap);
	n = SendMessageW(edit, WM_GETTEXTLENGTH, 0, 0);
	SendMessageW(edit, EM_SETSEL, n, n);
	SendMessageW(edit, EM_REPLACESEL, FALSE, (LPARAM) buf);
}
