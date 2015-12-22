// 22 december 2015
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
#include <dwrite.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void panic(HRESULT hr, const char *fmt, ...);

WCHAR userLocale[LOCALE_NAME_MAX_LENGTH];
int userLocaleSuccess;

void printFamilyName(IDWriteFontFamily *family, UINT32 i)
{
	IDWriteLocalizedStrings *names;
	HRESULT hr;
	UINT32 index;
	BOOL exists;
	UINT32 length;
	WCHAR *name;

	hr = family->GetFamilyNames(&names);
	if (hr != S_OK)
		panic(hr, "Error getting font %I32u names", i);

	// assume failure in case userLocaleSuccess == 0
	// this is complex, but we ignore failure conditions to allow fallbacks
	// 1) If the user locale name was successfully retrieved, try it
	// 2) If the user locale name was not successfully retrieved, or that locale's string does not exist, or an error occurred, try L"en-us", the US English locale
	// 3) And if that fails, assume the first one
	// This algorithm is straight from MSDN: https://msdn.microsoft.com/en-us/library/windows/desktop/dd368214%28v=vs.85%29.aspx
	// For step 2 to work, start by setting hr to S_OK and exists to FALSE.
	hr = S_OK;
	exists = FALSE;
	if (userLocaleSuccess != 0)
		hr = names->FindLocaleName(userLocale, &index, &exists);
	if (hr == S_OK && !exists)
		hr = names->FindLocaleName(L"en-us", &index, &exists);
	if (!exists)
		index = 0;

	hr = names->GetStringLength(index, &length);
	if (hr != S_OK)
		panic(hr, "Error getting size of font %I32u name", i);

	// GetStringLength() does not include the null terminator, but GetString() does
	name = new WCHAR[length + 1];
	hr = names->GetString(index, name, length + 1);
	if (hr != S_OK)
		panic(hr, "Error getting font %I32u name", i);

	printf("%ws\n", name);

	delete[] name;
	names->Release();
}

int main(void)
{
	IDWriteFactory *dwfactory;
	IDWriteFontCollection *fonts;
	IDWriteFontFamily *family;
	UINT32 i, n;
	HRESULT hr;

	hr = CoInitialize(NULL);
	if (hr != S_OK)
		panic(hr, "Error initializing COM");

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof (IDWriteFactory),
		(IUnknown **) (&dwfactory));
	if (hr != S_OK)
		panic(hr, "Error initializing DirectWrite");

	userLocaleSuccess = GetUserDefaultLocaleName(userLocale, LOCALE_NAME_MAX_LENGTH);

	hr = dwfactory->GetSystemFontCollection(&fonts, TRUE);
	if (hr != S_OK)
		panic(hr, "Error getting system font list");

	n = fonts->GetFontFamilyCount();
	printf("Found %I32u fonts.\n", n);

	for (i = 0; i < n; i++) {
		hr = fonts->GetFontFamily(i, &family);
		if (hr != S_OK)
			panic(hr, "Error getting font %I32u", i);
		printFamilyName(family, i);
		family->Release();
	}

	fonts->Release();
	dwfactory->Release();
	CoUninitialize();
	return 0;
}

void panic(HRESULT hr, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, " (HRESULT 0x%08I32X)\n", hr);
	va_end(ap);
	exit(EXIT_FAILURE);
}
