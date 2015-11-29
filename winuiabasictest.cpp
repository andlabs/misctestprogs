// 29 november 2015
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define ISOLATION_AWARE_ENABLED 1
// get Windows version right; right now Windows XP
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_WINDOWS 0x0501		/* according to Microsoft's winperf.h */
#define _WIN32_IE 0x0600			/* according to Microsoft's sdkddkver.h */
#define NTDDI_VERSION 0x05010000	/* according to Microsoft's sdkddkver.h */
#include <windows.h>
#include <uiautomation.h>

class tableAcc : public IRawElementProviderSimple {
	HWND hwnd;
	bool valid;
	ULONG refcount;
public:
	tableAcc(HWND);

	// internal methods
	void Invalidate(void);

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// IRawElementProviderSimple
	STDMETHODIMP GetPatternProvider(PATTERNID patternId, IUnknown **pRetVal);
	STDMETHODIMP GetPropertyValue(PROPERTYID propertyId, VARIANT *pRetVal);
	STDMETHODIMP get_HostRawElementProvider(IRawElementProviderSimple **pRetVal);
	STDMETHODIMP get_ProviderOptions(ProviderOptions *pRetVal);
};

tableAcc::tableAcc(HWND hwnd)
{
	this->hwnd = hwnd;
	this->valid = true;
	this->refcount = 1;		// first instance goes to the table
}

void tableAcc::Invalidate(void)
{
	this->hwnd = NULL;
	this->valid = false;
}

STDMETHODIMP tableAcc::QueryInterface(REFIID riid, void **ppvObject)
{
	if (ppvObject == NULL)
		return E_POINTER;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IRawElementProviderSimple)) {
		this->AddRef();
		*ppvObject = this;
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) tableAcc::AddRef(void)
{
	this->refcount++;
	return this->refcount;
}

STDMETHODIMP_(ULONG) tableAcc::Release(void)
{
	this->refcount--;
	if (this->refcount == 0) {
		delete this;
		return 0;
	}
	return this->refcount;
}

STDMETHODIMP tableAcc::GetPatternProvider(PATTERNID patternId, IUnknown **pRetVal)
{
	if (pRetVal == NULL)
		return E_POINTER;
	// no provider yet; let's just make sure getting properties work
	*pRetVal = NULL;
	return S_OK;
}

STDMETHODIMP tableAcc::GetPropertyValue(PROPERTYID propertyId, VARIANT *pRetVal)
{
	BSTR bstr;

	if (pRetVal == NULL)
		return E_POINTER;
	pRetVal->vt = VT_EMPTY;		// behavior on unknown property is to keep it VT_EMPTY and return S_OK
	switch (propertyId) {
	case UIA_NamePropertyId:
		// TODO this doesn't show up
		bstr = SysAllocString(L"test string");
		if (bstr == NULL)
			return E_OUTOFMEMORY;
		pRetVal->vt = VT_BSTR;
		pRetVal->bstrVal = bstr;
		break;
	}
	return S_OK;
}

STDMETHODIMP tableAcc::get_HostRawElementProvider(IRawElementProviderSimple **pRetVal)
{
	if (!this->valid) {
		if (pRetVal == NULL)
			return E_POINTER;
		// TODO correct?
		*pRetVal = NULL;
		return RPC_E_DISCONNECTED;
	}
	return UiaHostProviderFromHwnd(this->hwnd, pRetVal);
}

STDMETHODIMP tableAcc::get_ProviderOptions(ProviderOptions *pRetVal)
{
	if (pRetVal == NULL)
		return E_POINTER;
	// TODO ProviderOptions_UseClientCoordinates?
	*pRetVal = ProviderOptions_ServerSideProvider;
	return S_OK;
}

tableAcc *acc;

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CREATE:
		acc = new tableAcc(hwnd);
		break;
	case WM_DESTROY:
		acc->Invalidate();
		acc->Release();
		PostQuitMessage(0);
		break;
	case WM_GETOBJECT:
		// TODO are these casts correct? MSDN can't seem to settle on one set
		if (((DWORD) lParam) != ((DWORD) UiaRootObjectId))
			break;
		return UiaReturnRawElementProvider(hwnd, wParam, lParam, acc);
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(void)
{
	WNDCLASSW wc;
	HWND mainwin, control;
	MSG msg;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = DefWindowProcW;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClassW(&wc) == 0)
		return 1;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"control";
	wc.lpfnWndProc = wndproc;
	wc.hbrBackground = (HBRUSH) (COLOR_ACTIVECAPTION + 1);
	if (RegisterClassW(&wc) == 0)
		return 2;

	mainwin = CreateWindowExW(0,
		L"mainwin", L"mainwin",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, GetModuleHandle(NULL), NULL);
	if (mainwin == NULL)
		return 3;

	control = CreateWindowExW(0,
		L"control", L"this should not show up",
		WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,
		50, 50, 150, 150,
		mainwin, NULL, GetModuleHandle(NULL), NULL);
	if (control == NULL)
		return 4;

	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return 0;
}
