// 21 november 2017
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
#include <comcat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// build:
// msvc: cl winolecontrollist.cpp -EHsc -W4 -wd4100 -Zi -link -debug user32.lib kernel32.lib gdi32.lib ole32.lib oleaut32.lib advapi32.lib

// references:
// - https://blogs.msdn.microsoft.com/oldnewthing/20171121-00/?p=97435
// - https://msdn.microsoft.com/en-us/library/windows/desktop/dd542655(v=vs.85).aspx
// - https://msdn.microsoft.com/en-us/library/windows/desktop/ms679677(v=vs.85).aspx
// - https://stackoverflow.com/questions/2755351/how-to-list-all-installed-activex-controls
// - https://msdn.microsoft.com/en-us/library/windows/desktop/dd542669(v=vs.85).aspx
// - https://www.google.com/search?q=correct+usage+of+EnumClassesOfCategories&ie=utf-8&oe=utf-8
// - https://books.google.com/books?id=kfRWvKSePmAC&pg=PA152&lpg=PA152&dq=correct+usage+of+EnumClassesOfCategories&source=bl&ots=o9bRcdaOfv&sig=dGBjjQn8P7EwGSrYw9kqt8WJ5UU&hl=en&sa=X&ved=0ahUKEwjVzcPcktDXAhULMt8KHZZnCIUQ6AEIQDAE#v=onepage&q=correct%20usage%20of%20EnumClassesOfCategories&f=false
// - https://msdn.microsoft.com/en-us/library/windows/desktop/ms682271(v=vs.85).aspx
// - https://msdn.microsoft.com/en-us/library/e73ffa52.aspx
// - https://msdn.microsoft.com/en-us/library/windows/desktop/ms221150%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
// - https://stackoverflow.com/questions/897743/how-to-find-a-dll-given-a-clsid
// - https://msdn.microsoft.com/en-us/library/windows/desktop/ms724475%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
// - https://msdn.microsoft.com/en-us/library/windows/desktop/ms724498(v=vs.85).aspx
// - https://msdn.microsoft.com/en-us/library/windows/desktop/ms691424(v=vs.85).aspx
// - https://msdn.microsoft.com/en-us/library/windows/desktop/ms724911%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
// - https://msdn.microsoft.com/en-us/library/aa443492%28v=vs.60%29.aspx?f=255&MSPPError=-2147217396
// - https://msdn.microsoft.com/en-us/library/windows/desktop/ms686568(v=vs.85).aspx

void die(const char *f, HRESULT hr)
{
	fprintf(stderr, "** %s failed: 0x%08I32X\n", f, hr);
	exit(EXIT_FAILURE);
}

void selfkv(HKEY hk, const WCHAR *value, const WCHAR *name = NULL)
{
	BYTE *data;
	DWORD n;
	LONG err;

	if (name == NULL)
		name = value;

	err = RegQueryValueExW(hk, value, NULL,
		NULL, NULL, &n);
	if (err == ERROR_FILE_NOT_FOUND) {
		printf("%ws none\n", name);
		return;
	}
	if (err != ERROR_SUCCESS)
		die("RegQueryValueExW() n", HRESULT_FROM_WIN32(err));
	data = new BYTE[n + 2];		// make room for terminating null character we might need to add (so we'll always add it)
	err = RegQueryValueExW(hk, value, NULL,
		NULL, data, &n);
	if (err != ERROR_SUCCESS)
		die("RegQueryValueExW() data", HRESULT_FROM_WIN32(err));
	data[n] = 0;			// and add the terminating null character
	data[n + 1] = 0;
	printf("%ws %ws\n", name, (WCHAR *) data);
	delete[] data;
}

void subkv(HKEY hk, const WCHAR *subkey, const WCHAR *value)
{
	HKEY sk;
	LONG err;

	err = RegOpenKeyExW(hk, subkey,
		0, KEY_READ, &sk);
	if (err == ERROR_FILE_NOT_FOUND) {
		printf("%ws none\n", subkey);
		return;
	}
	if (err != ERROR_SUCCESS)
		die("RegOpenKeyExW() sub", HRESULT_FROM_WIN32(err));

	selfkv(sk, value, subkey);

	err = RegCloseKey(sk);
	if (err != ERROR_SUCCESS)
		die("RegCloseKey() sub", HRESULT_FROM_WIN32(err));
}

void each(REFCLSID clsid)
{
	OLECHAR *s;
	WCHAR *key;
	HKEY hk;
	LONG err;
	HRESULT hr;

	hr = StringFromCLSID(clsid, &s);
	if (hr != S_OK)
		die("StringFromCLSID()", hr);
	printf("%ws\n", s);

	key = new WCHAR[wcslen(L"\\CLSID\\") + wcslen(s) + 1];
	wcscpy(key, L"\\CLSID\\");
	wcscat(key, s);

	CoTaskMemFree(s);
	s = NULL;

	err = RegOpenKeyExW(HKEY_CLASSES_ROOT, key,
		0, KEY_READ, &hk);
	if (err != ERROR_SUCCESS)
		die("RegOpenKeyExW() main", HRESULT_FROM_WIN32(err));
	delete[] key;

#define E(x) \
	hr = OleRegGetUserType(clsid, x, &s); \
	if (hr != S_OK) \
		die("OleRegGetUserType() " #x, hr); \
	printf("%s %ws\n", #x, s); \
	CoTaskMemFree(s); /* TODO not sure if this is correct */ \
	s = NULL;

	E(USERCLASSTYPE_FULL);
	E(USERCLASSTYPE_SHORT);
	E(USERCLASSTYPE_APPNAME);

#undef E

	hr = ProgIDFromCLSID(clsid, &s);
	if (hr == S_OK) {
		printf("ProgID %ws\n", s);
		CoTaskMemFree(s);
		s = NULL;
	} else if (hr == REGDB_E_CLASSNOTREG)
		printf("ProgID none (class not registered)\n");
	else
		die("ProgIDFromCLSID()", hr);

	selfkv(hk, L"InprocHandler");
	selfkv(hk, L"InprocHandler32");
	subkv(hk, L"InprocServer", NULL);
	subkv(hk, L"InprocServer32", NULL);
	selfkv(hk, L"LocalServer");
	subkv(hk, L"LocalServer32", NULL);

	err = RegCloseKey(hk);
	if (err != ERROR_SUCCESS)
		die("RegCloseKey() main", HRESULT_FROM_WIN32(err));

	printf("\n");
}

#define N 64

int main(void)
{
	ICatInformation *c;
	CATID ids[1];
	IEnumCLSID *e;
	CLSID clsids[N];
	ULONG i, n;
	HRESULT hr;

	hr = CoInitialize(NULL);
	if (hr != S_OK)
		die("CoInitialize()", hr);

	hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_ALL, IID_PPV_ARGS(&c));
	if (hr != S_OK)
		die("CoCreateInstance()", hr);

	ids[0] = CATID_Control;
	hr = c->EnumClassesOfCategories(1, ids, (ULONG) (-1), NULL, &e);
	if (hr != S_OK)
		die("EnumClassesOfCategories()", hr);

	for (;;) {
		hr = e->Next(N, clsids, &n);
		if (hr != S_OK && hr != S_FALSE)
			die("Next()", hr);
		for (i = 0; i < n; i++)
			each(clsids[i]);
		if (hr == S_FALSE)
			break;
	}

	e->Release();
	c->Release();
	CoUninitialize();
	return 0;
}
