// 14 december 2015
#define _UNICODE
#define UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
// get Windows version right; right now Windows XP
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_WINDOWS 0x0501		/* according to Microsoft's winperf.h */
#define _WIN32_IE 0x0600			/* according to Microsoft's sdkddkver.h */
#define NTDDI_VERSION 0x05010000	/* according to Microsoft's sdkddkver.h */
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>

void chk(HRESULT hr) { if (hr != S_OK) DebugBreak(); }

void doWork(bool pumpMessages)
{
	UINT_PTR timer;
	MSG msg;

	if (!pumpMessages) {
		Sleep(2000);
		return;
	}
	timer = SetTimer(NULL, 20, 2000, NULL);
	while (GetMessageW(&msg, NULL, 0, 0)) {
		if (msg.message == WM_TIMER && msg.hwnd == NULL)
			break;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	KillTimer(NULL, timer);
}

HWINEVENTHOOK hook;
HWND dialogWindow;
HANDLE dialogEvent;
void CALLBACK onDialogClosed(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (hwnd == dialogWindow)
		SetEvent(dialogEvent);
}
void waitEvent(void)
{
	MSG msg;
	DWORD ret;

	for (;;) {
		ret = MsgWaitForMultipleObjectsEx(1, &dialogEvent,
			INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
		if (ret == WAIT_OBJECT_0)	// event
			break;
		if (GetMessage(&msg, NULL, 0, 0) == 0)
			break;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

void rundialogs(HWND parent, bool pumpMessages, bool tryHide, int tryHideWhat, bool abort)
{
	IProgressDialog *pd;
	IOleWindow *olewin;
	HWND hwnd;
	DWORD process;
	DWORD thread;

	chk(CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pd)));
	chk(pd->SetTitle(L"Test"));
	chk(pd->StartProgressDialog(parent, NULL,
		PROGDLG_NORMAL | PROGDLG_MODAL | PROGDLG_AUTOTIME | PROGDLG_NOMINIMIZE,
		NULL));

	doWork(pumpMessages);

	chk(pd->Timer(PDTIMER_RESET, NULL));
	for (ULONGLONG i = 0; i < 10; i++) {
		if (pd->HasUserCancelled())
			break;

		doWork(pumpMessages);
		if (i == 5 && abort)
			break;

		chk(pd->SetProgress64(i + 1, 10));
	}

	chk(pd->QueryInterface(IID_PPV_ARGS(&olewin)));
	chk(olewin->GetWindow(&hwnd));
	olewin->Release();

	// set up event hoook before stopping the progress dialog
	// this way it won't get sdestroyed before the hook is set up
	if (tryHide && tryHideWhat == 3) {
		thread = GetWindowThreadProcessId(hwnd, &process);
		dialogWindow = hwnd;
		dialogEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		ResetEvent(dialogEvent);
		hook = SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_DESTROY,
			NULL, onDialogClosed,
			process, thread,
			WINEVENT_OUTOFCONTEXT);
	}

	chk(pd->StopProgressDialog());
	if (tryHide)
		switch (tryHideWhat) {
		case 0:		// hide
			ShowWindow(hwnd, SW_HIDE);
			break;
		case 1:		// send WM_NULL
			SendMessageW(hwnd, WM_NULL, 0, 0);
			break;
		case 2:		// post WM_NULL
			PostMessageW(hwnd, WM_NULL, 0, 0);
			break;
		case 3:		// winevents
			waitEvent();
			UnhookWinEvent(hook);
			break;
		}
	pd->Release();

	MessageBoxW(parent,
		L"This should be MODAL to the main window!\n"
		L"But you should see that in reality the main window\n"
		L"is still enabled!",
		L"mainwin",
		MB_OK | MB_ICONINFORMATION);
}

HWND button;
HWND checkbox;
HWND checkbox2;
HWND combobox;
HWND checkbox3;

bool ischecked(HWND hwnd) { return SendMessageW(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED; }

static LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_COMMAND && lParam == (LPARAM) button)
		rundialogs(hwnd,
			ischecked(checkbox),
			ischecked(checkbox2),
			(int) SendMessageW(combobox, CB_GETCURSEL, 0, 0),
			ischecked(checkbox3));
	if (uMsg == WM_CLOSE)
		PostQuitMessage(0);
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSW wc;
	HWND mainwin;
	MSG msg;

	CoInitialize(NULL);

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	mainwin = CreateWindowExW(0,
		L"mainwin", L"mainwin",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		200, 220,
		NULL, NULL, hInstance, NULL);

	button = CreateWindowExW(0,
		L"button", L"Click Me",
		BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
		10, 10, 150, 100,
		mainwin, (HMENU) 100, hInstance, NULL);

	checkbox = CreateWindowExW(0,
		L"button", L"Pump Messages",
		BS_CHECKBOX | BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE,
		10, 110, 150, 20,
		mainwin, (HMENU) 101, hInstance, NULL);
	checkbox2 = CreateWindowExW(0,
		L"button", L"Try",
		BS_CHECKBOX | BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE,
		10, 130, 50, 20,
		mainwin, (HMENU) 101, hInstance, NULL);
	combobox = CreateWindowExW(0,
		L"combobox", L"",
		CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
		60, 130, 100, 100,
		mainwin, (HMENU) 102, hInstance, NULL);
	SendMessageW(combobox, CB_ADDSTRING, 0, (LPARAM) L"Hide");
	SendMessageW(combobox, CB_ADDSTRING, 0, (LPARAM) L"Send");
	SendMessageW(combobox, CB_ADDSTRING, 0, (LPARAM) L"Post");
	SendMessageW(combobox, CB_ADDSTRING, 0, (LPARAM) L"WinEvents");
	SendMessageW(combobox, CB_SETCURSEL, 0, 0);
	checkbox3 = CreateWindowExW(0,
		L"button", L"Abort Early",
		BS_CHECKBOX | BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE,
		10, 150, 150, 20,
		mainwin, (HMENU) 103, hInstance, NULL);

	ShowWindow(mainwin, nCmdShow);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	CoUninitialize();
	return 0;
}
