// 8-9 may 2014
#define _UNICODE
#define UNICODE
#define STRICT
#include <windows.h>
#include <string.h>

// specifically designed to import everything in such a way that the compiler checks all the autogenerated functions, not just the ones that are able to run on the system being targeted
#undef ISOLATION_AWARE_ENABLED
#ifndef _WIN64
#define _WIN64
#endif
#undef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_VISTA
#undef _WIN32_IE
#define _WIN32_IE 0x0600
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#undef WINVER
#define WINVER 0x0600
#include <commctrl.h>

// and for MinGW
#define xDSAInsertItemvoid const void*
#if defined(__MINGW32__) || defined(__MINGW64__)
// if MinGW ever fixes its DPA/DSA types or adds Vista-specific comctl32 features, this will need to be changed

// actually in these cases it looks like Visual Studio 2010 changed the headers but MSDN didn't... and MinGW is using what previous versions of VS/Platform SDK and MSDN says... TODO check VS2008 and VS2005 (earliest version with __VA_ARGS__)
#undef xDSAInsertItemvoid
#define xDSAInsertItemvoid void*
typedef PFNDPAENUMCALLBACK PFNDAENUMCALLBACK;
typedef PFNDPACOMPARE PFNDACOMPARE;

typedef int TASKDIALOG_COMMON_BUTTON_FLAGS;		// there's even a friendly comment in Microsoft's headers informing us that this is, indeed, an int
typedef struct TASKDIALOGCONFIG TASKDIALOGCONFIG;
#endif

static const char *manifest = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">\n<assemblyIdentity\n    version=\"1.0.0.0\"\n    processorArchitecture=\"*\"\n    name=\"CompanyName.ProductName.YourApplication\"\n    type=\"win32\"\n/>\n<description>Your application description here.</description>\n<dependency>\n    <dependentAssembly>\n        <assemblyIdentity\n            type=\"win32\"\n            name=\"Microsoft.Windows.Common-Controls\"\n            version=\"6.0.0.0\"\n            processorArchitecture=\"*\"\n            publicKeyToken=\"6595b64144ccf1df\"\n            language=\"*\"\n        />\n    </dependentAssembly>\n</dependency>\n</assembly>\n\n";		// not WCHAR!
static ACTCTX context;
static HANDLE contextHandle;
static ULONG_PTR contextCookie;
static WCHAR manifestfile[MAX_PATH + 1] = { L'\0' };

static BOOL writeManifest(void)
{
	WCHAR tmppath[MAX_PATH + 1];
	HANDLE tmpfile;
	DWORD nWanted, nGot;

	if (GetTempPath(MAX_PATH + 1, tmppath) == 0)
		goto error;
	if (GetTempFileName(tmppath, L"manifest", 0, manifestfile) == 0)
		goto error;
	tmpfile = CreateFile(manifestfile, GENERIC_WRITE,
		0,		// no sharing; especially important here because the activation context functions will fail if the file is opened elsewhere
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (tmpfile == INVALID_HANDLE_VALUE)
		goto error;
	nWanted = strlen(manifest);
	SetLastError(0);		// to trap short writes that don't return errors
	if (WriteFile(tmpfile, manifest, nWanted, &nGot, NULL) == 0)
		goto error;
	if (nWanted != nGot) {
		DWORD err;

		err = GetLastError();
		if (err == 0)
			err = ERROR_WRITE_FAULT;	// best I can think of for now
		SetLastError(err);
		goto error;
	}
	if (CloseHandle(tmpfile) == 0)
		goto error;
	return TRUE;		// all good!
error:
	// reset manifestfile so we can try again
	manifestfile[0] = L'\0';
	return FALSE;
}

static BOOL comctl6Enable(void)
{
	if (manifestfile[0] == L'\0') {
		if (writeManifest() == FALSE)
			return FALSE;
		ZeroMemory(&context, sizeof (ACTCTX));
		context.cbSize = sizeof (ACTCTX);
		context.lpSource = manifestfile;
		contextHandle = CreateActCtx(&context);
		if (contextHandle == INVALID_HANDLE_VALUE) {
			// reset manifestfile so we can try again
			manifestfile[0] = L'\0';
			return FALSE;
		}
	}
	return ActivateActCtx(contextHandle, &contextCookie);
}

static BOOL comctl6Disable(void)
{
	return DeactivateActCtx(0, contextCookie);
}

#define indexCreateMappedBitmap 0
#define indexCreateStatusWindowA 1
#define indexCreateStatusWindowW 2
#define indexCreateToolbarEx 3
#define indexCreateUpDownControl 4
#define indexDPA_Clone 5
#define indexDPA_Create 6
#define indexDPA_CreateEx 7
#define indexDPA_DeleteAllPtrs 8
#define indexDPA_DeletePtr 9
#define indexDPA_Destroy 10
#define indexDPA_DestroyCallback 11
#define indexDPA_EnumCallback 12
#define indexDPA_GetPtr 13
#define indexDPA_GetPtrIndex 14
#define indexDPA_GetSize 15
#define indexDPA_Grow 16
#define indexDPA_InsertPtr 17
#define indexDPA_LoadStream 18
#define indexDPA_Merge 19
#define indexDPA_SaveStream 20
#define indexDPA_Search 21
#define indexDPA_SetPtr 22
#define indexDPA_Sort 23
#define indexDSA_Clone 24
#define indexDSA_Create 25
#define indexDSA_DeleteAllItems 26
#define indexDSA_DeleteItem 27
#define indexDSA_Destroy 28
#define indexDSA_DestroyCallback 29
#define indexDSA_EnumCallback 30
#define indexDSA_GetItem 31
#define indexDSA_GetItemPtr 32
#define indexDSA_GetSize 33
#define indexDSA_InsertItem 34
#define indexDSA_SetItem 35
#define indexDSA_Sort 36
#define indexDefSubclassProc 37
#define indexDrawInsert 38
#define indexDrawShadowText 39
#define indexDrawStatusTextA 40
#define indexDrawStatusTextW 41
#define indexFlatSB_EnableScrollBar 42
#define indexFlatSB_GetScrollInfo 43
#define indexFlatSB_GetScrollPos 44
#define indexFlatSB_GetScrollProp 45
#define indexFlatSB_GetScrollPropPtr 46
#define indexFlatSB_GetScrollRange 47
#define indexFlatSB_SetScrollInfo 48
#define indexFlatSB_SetScrollPos 49
#define indexFlatSB_SetScrollProp 50
#define indexFlatSB_SetScrollRange 51
#define indexFlatSB_ShowScrollBar 52
#define indexGetEffectiveClientRect 53
#define indexGetMUILanguage 54
#define indexGetWindowSubclass 55
#define indexHIMAGELIST_QueryInterface 56
#define indexImageList_Add 57
#define indexImageList_AddMasked 58
#define indexImageList_BeginDrag 59
#define indexImageList_Copy 60
#define indexImageList_Create 61
#define indexImageList_Destroy 62
#define indexImageList_DragEnter 63
#define indexImageList_DragLeave 64
#define indexImageList_DragMove 65
#define indexImageList_DragShowNolock 66
#define indexImageList_Draw 67
#define indexImageList_DrawEx 68
#define indexImageList_DrawIndirect 69
#define indexImageList_Duplicate 70
#define indexImageList_EndDrag 71
#define indexImageList_GetBkColor 72
#define indexImageList_GetDragImage 73
#define indexImageList_GetIcon 74
#define indexImageList_GetIconSize 75
#define indexImageList_GetImageCount 76
#define indexImageList_GetImageInfo 77
#define indexImageList_LoadImageA 78
#define indexImageList_LoadImageW 79
#define indexImageList_Merge 80
#define indexImageList_Read 81
#define indexImageList_ReadEx 82
#define indexImageList_Remove 83
#define indexImageList_Replace 84
#define indexImageList_ReplaceIcon 85
#define indexImageList_SetBkColor 86
#define indexImageList_SetDragCursorImage 87
#define indexImageList_SetIconSize 88
#define indexImageList_SetImageCount 89
#define indexImageList_SetOverlayImage 90
#define indexImageList_Write 91
#define indexImageList_WriteEx 92
#define indexInitCommonControls 93
#define indexInitCommonControlsEx 94
#define indexInitMUILanguage 95
#define indexInitializeFlatSB 96
#define indexLBItemFromPt 97
#define indexLoadIconMetric 98
#define indexLoadIconWithScaleDown 99
#define indexMakeDragList 100
#define indexMenuHelp 101
#define indexRemoveWindowSubclass 102
#define indexSetWindowSubclass 103
#define indexShowHideMenuCtl 104
#define indexStr_SetPtrW 105
#define indexTaskDialog 106
#define indexTaskDialogIndirect 107
#define indexUninitializeFlatSB 108
#define index_TrackMouseEvent 109
#define nIndices 110

static FARPROC comctlFuncs[nIndices][2];
static char *comctlFuncNames[nIndices] = {
	"CreateMappedBitmap",
	"CreateStatusWindowA",
	"CreateStatusWindowW",
	"CreateToolbarEx",
	"CreateUpDownControl",
	"DPA_Clone",
	"DPA_Create",
	"DPA_CreateEx",
	"DPA_DeleteAllPtrs",
	"DPA_DeletePtr",
	"DPA_Destroy",
	"DPA_DestroyCallback",
	"DPA_EnumCallback",
	"DPA_GetPtr",
	"DPA_GetPtrIndex",
	"DPA_GetSize",
	"DPA_Grow",
	"DPA_InsertPtr",
	"DPA_LoadStream",
	"DPA_Merge",
	"DPA_SaveStream",
	"DPA_Search",
	"DPA_SetPtr",
	"DPA_Sort",
	"DSA_Clone",
	"DSA_Create",
	"DSA_DeleteAllItems",
	"DSA_DeleteItem",
	"DSA_Destroy",
	"DSA_DestroyCallback",
	"DSA_EnumCallback",
	"DSA_GetItem",
	"DSA_GetItemPtr",
	"DSA_GetSize",
	"DSA_InsertItem",
	"DSA_SetItem",
	"DSA_Sort",
	"DefSubclassProc",
	"DrawInsert",
	"DrawShadowText",
	"DrawStatusTextA",
	"DrawStatusTextW",
	"FlatSB_EnableScrollBar",
	"FlatSB_GetScrollInfo",
	"FlatSB_GetScrollPos",
	"FlatSB_GetScrollProp",
	"FlatSB_GetScrollPropPtr",
	"FlatSB_GetScrollRange",
	"FlatSB_SetScrollInfo",
	"FlatSB_SetScrollPos",
	"FlatSB_SetScrollProp",
	"FlatSB_SetScrollRange",
	"FlatSB_ShowScrollBar",
	"GetEffectiveClientRect",
	"GetMUILanguage",
	"GetWindowSubclass",
	"HIMAGELIST_QueryInterface",
	"ImageList_Add",
	"ImageList_AddMasked",
	"ImageList_BeginDrag",
	"ImageList_Copy",
	"ImageList_Create",
	"ImageList_Destroy",
	"ImageList_DragEnter",
	"ImageList_DragLeave",
	"ImageList_DragMove",
	"ImageList_DragShowNolock",
	"ImageList_Draw",
	"ImageList_DrawEx",
	"ImageList_DrawIndirect",
	"ImageList_Duplicate",
	"ImageList_EndDrag",
	"ImageList_GetBkColor",
	"ImageList_GetDragImage",
	"ImageList_GetIcon",
	"ImageList_GetIconSize",
	"ImageList_GetImageCount",
	"ImageList_GetImageInfo",
	"ImageList_LoadImageA",
	"ImageList_LoadImageW",
	"ImageList_Merge",
	"ImageList_Read",
	"ImageList_ReadEx",
	"ImageList_Remove",
	"ImageList_Replace",
	"ImageList_ReplaceIcon",
	"ImageList_SetBkColor",
	"ImageList_SetDragCursorImage",
	"ImageList_SetIconSize",
	"ImageList_SetImageCount",
	"ImageList_SetOverlayImage",
	"ImageList_Write",
	"ImageList_WriteEx",
	"InitCommonControls",
	"InitCommonControlsEx",
	"InitMUILanguage",
	"InitializeFlatSB",
	"LBItemFromPt",
	"LoadIconMetric",
	"LoadIconWithScaleDown",
	"MakeDragList",
	"MenuHelp",
	"RemoveWindowSubclass",
	"SetWindowSubclass",
	"ShowHideMenuCtl",
	"Str_SetPtrW",
	"TaskDialog",
	"TaskDialogIndirect",
	"UninitializeFlatSB",
	"_TrackMouseEvent",
};

static HMODULE comctl5 = NULL, comctl6 = NULL;

static void loadAll(HANDLE h, int n)
{
	int i;

	for (i = 0; i < nIndices; i++)
		comctlFuncs[i][n] = GetProcAddress(h, comctlFuncNames[i]);
}

BOOL loadcomctl(void)
{
	ZeroMemory(comctlFuncs, sizeof (FARPROC) * nIndices * 2);		// just to be safe
	comctl5 = LoadLibraryW(L"comctl32.dll");
	if (comctl5 == NULL)
		return FALSE;
	loadAll(comctl5, 0);
	if (comctl6Enable() == FALSE)
		return FALSE;
	comctl6 = LoadLibraryW(L"comctl32.dll");
	if (comctl6 == NULL) {
		DWORD err;

		err = GetLastError();
		comctl6Disable();	// disregard error here; the above function is what failed
		SetLastError(err);
		return FALSE;
	}
	loadAll(comctl6, 1);
	return comctl6Disable();
}

// TODO make thread safe
static int which = 0;

BOOL switchcomctl5(void)
{
	BOOL err = TRUE;

	if (which == 1)
		err = comctl6Disable();
	if (err == TRUE)
		which = 0;
	return err;
}

BOOL switchcomctl6(void)
{
	BOOL err = TRUE;

	if (which == 0)
		err = comctl6Enable();
	if (err == TRUE)
		which = 1;
	return err;
}

BOOL comctlSupports(const char *func)
{
	int i;

	for (i = 0; i < nIndices; i++)
		if (strcmp(comctlFuncNames[i], func) == 0)
			return comctlFuncs[i][which] != NULL;
	return FALSE;
}

#define IMPL(n, ret, ...) \
	ret WINAPI n(__VA_ARGS__) \
	{ \
		ret (WINAPI *fn)(__VA_ARGS__) = \
			(ret (WINAPI *)(__VA_ARGS__)) comctlFuncs[index ## n][which];
#define ENDIMPL(...) \
		(*fn)(__VA_ARGS__); \
	}

// note in the below that I use this* instead of this * due to the awk script used to generate these macro invocations; see below that for details

WINCOMMCTRLAPI IMPL(CreateMappedBitmap, HBITMAP, HINSTANCE hInstance, INT_PTR idBitmap, UINT wFlags, LPCOLORMAP lpColorMap, int iNumMaps) return ENDIMPL(hInstance, idBitmap, wFlags, lpColorMap, iNumMaps)
WINCOMMCTRLAPI IMPL(CreateStatusWindowA, HWND, LONG style, LPCSTR lpszText, HWND hwndParent, UINT wID) return ENDIMPL(style, lpszText, hwndParent, wID)
WINCOMMCTRLAPI IMPL(CreateStatusWindowW, HWND, LONG style, LPCWSTR lpszText, HWND hwndParent, UINT wID) return ENDIMPL(style, lpszText, hwndParent, wID)
WINCOMMCTRLAPI IMPL(CreateToolbarEx, HWND, HWND hwnd, DWORD ws, UINT wID, int nBitmaps, HINSTANCE hBMInst, UINT_PTR wBMID, LPCTBBUTTON lpButtons, int iNumButtons, int dxButton, int dyButton, int dxBitmap, int dyBitmap, UINT uStructSize) return ENDIMPL(hwnd, ws, wID, nBitmaps, hBMInst, wBMID, lpButtons, iNumButtons, dxButton, dyButton, dxBitmap, dyBitmap, uStructSize)
WINCOMMCTRLAPI IMPL(CreateUpDownControl, HWND, DWORD dwStyle, int x, int y, int cx, int cy, HWND hParent, int nID, HINSTANCE hInst, HWND hBuddy, int nUpper, int nLower, int nPos) return ENDIMPL(dwStyle, x, y, cx, cy, hParent, nID, hInst, hBuddy, nUpper, nLower, nPos)
WINCOMMCTRLAPI IMPL(DPA_Clone, HDPA, const HDPA hdpa, HDPA hdpaNew) return ENDIMPL(hdpa, hdpaNew)
WINCOMMCTRLAPI IMPL(DPA_Create, HDPA, int cItemGrow) return ENDIMPL(cItemGrow)
WINCOMMCTRLAPI IMPL(DPA_CreateEx, HDPA, int cpGrow, HANDLE hheap) return ENDIMPL(cpGrow, hheap)
WINCOMMCTRLAPI IMPL(DPA_DeleteAllPtrs, BOOL, HDPA hdpa) return ENDIMPL(hdpa)
WINCOMMCTRLAPI IMPL(DPA_DeletePtr, PVOID, HDPA hdpa, int i) return ENDIMPL(hdpa, i)
WINCOMMCTRLAPI IMPL(DPA_Destroy, BOOL, HDPA hdpa) return ENDIMPL(hdpa)
WINCOMMCTRLAPI IMPL(DPA_DestroyCallback, void, HDPA hdpa, PFNDAENUMCALLBACK pfnCB, void* pData) ENDIMPL(hdpa, pfnCB, pData)
WINCOMMCTRLAPI IMPL(DPA_EnumCallback, void, HDPA hdpa, PFNDAENUMCALLBACK pfnCB, void* pData) ENDIMPL(hdpa, pfnCB, pData)
WINCOMMCTRLAPI IMPL(DPA_GetPtr, PVOID, HDPA hdpa, INT_PTR i) return ENDIMPL(hdpa, i)
WINCOMMCTRLAPI IMPL(DPA_GetPtrIndex, int, HDPA hdpa, const void* p) return ENDIMPL(hdpa, p)
WINCOMMCTRLAPI IMPL(DPA_GetSize, ULONGLONG, HDPA hdpa) return ENDIMPL(hdpa)
WINCOMMCTRLAPI IMPL(DPA_Grow, BOOL, HDPA pdpa, int cp) return ENDIMPL(pdpa, cp)
WINCOMMCTRLAPI IMPL(DPA_InsertPtr, int, HDPA hdpa, int i, void* p) return ENDIMPL(hdpa, i, p)
WINCOMMCTRLAPI IMPL(DPA_LoadStream, HRESULT, HDPA* phdpa, PFNDPASTREAM pfn, struct IStream* pstream, void* pvInstData) return ENDIMPL(phdpa, pfn, pstream, pvInstData)
WINCOMMCTRLAPI IMPL(DPA_Merge, BOOL, HDPA hdpaDest, HDPA hdpaSrc, DWORD dwFlags, PFNDACOMPARE pfnCompare, PFNDPAMERGE pfnMerge, LPARAM lParam) return ENDIMPL(hdpaDest, hdpaSrc, dwFlags, pfnCompare, pfnMerge, lParam)
WINCOMMCTRLAPI IMPL(DPA_SaveStream, HRESULT, HDPA hdpa, PFNDPASTREAM pfn, struct IStream* pstream, void* pvInstData) return ENDIMPL(hdpa, pfn, pstream, pvInstData)
WINCOMMCTRLAPI IMPL(DPA_Search, int, HDPA hdpa, void* pFind, int iStart, PFNDACOMPARE pfnCompare, LPARAM lParam, UINT options) return ENDIMPL(hdpa, pFind, iStart, pfnCompare, lParam, options)
WINCOMMCTRLAPI IMPL(DPA_SetPtr, BOOL, HDPA hdpa, int i, void* p) return ENDIMPL(hdpa, i, p)
WINCOMMCTRLAPI IMPL(DPA_Sort, BOOL, HDPA hdpa, PFNDACOMPARE pfnCompare, LPARAM lParam) return ENDIMPL(hdpa, pfnCompare, lParam)
WINCOMMCTRLAPI IMPL(DSA_Clone, HDSA, HDSA hdsa) return ENDIMPL(hdsa)
WINCOMMCTRLAPI IMPL(DSA_Create, HDSA, int cbItem, int cItemGrow) return ENDIMPL(cbItem, cItemGrow)
WINCOMMCTRLAPI IMPL(DSA_DeleteAllItems, BOOL, HDSA hdsa) return ENDIMPL(hdsa)
WINCOMMCTRLAPI IMPL(DSA_DeleteItem, BOOL, HDSA hdsa, int i) return ENDIMPL(hdsa, i)
WINCOMMCTRLAPI IMPL(DSA_Destroy, BOOL, HDSA hdsa) return ENDIMPL(hdsa)
WINCOMMCTRLAPI IMPL(DSA_DestroyCallback, void, HDSA hdsa, PFNDAENUMCALLBACK pfnCB, void* pData) ENDIMPL(hdsa, pfnCB, pData)
WINCOMMCTRLAPI IMPL(DSA_EnumCallback, void, HDSA hdsa, PFNDAENUMCALLBACK pfnCB, void* pData) ENDIMPL(hdsa, pfnCB, pData)
WINCOMMCTRLAPI IMPL(DSA_GetItem, BOOL, HDSA hdsa, int i, void* pitem) return ENDIMPL(hdsa, i, pitem)
WINCOMMCTRLAPI IMPL(DSA_GetItemPtr, PVOID, HDSA hdsa, int i) return ENDIMPL(hdsa, i)
WINCOMMCTRLAPI IMPL(DSA_GetSize, ULONGLONG, HDSA hdsa) return ENDIMPL(hdsa)
WINCOMMCTRLAPI IMPL(DSA_InsertItem, int, HDSA hdsa, int i, xDSAInsertItemvoid pitem) return ENDIMPL(hdsa, i, pitem)
WINCOMMCTRLAPI IMPL(DSA_SetItem, BOOL, HDSA hdsa, int i, const void* pitem) return ENDIMPL(hdsa, i, pitem)
WINCOMMCTRLAPI IMPL(DSA_Sort, BOOL, HDSA pdsa, PFNDACOMPARE pfnCompare, LPARAM lParam) return ENDIMPL(pdsa, pfnCompare, lParam)
IMPL(DefSubclassProc, LRESULT, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) return ENDIMPL(hWnd, uMsg, wParam, lParam)
WINCOMMCTRLAPI IMPL(DrawInsert, void, HWND handParent, HWND hLB, int nItem) ENDIMPL(handParent, hLB, nItem)
IMPL(DrawShadowText, int, HDC hdc, LPCWSTR pszText, UINT cch, RECT* prc, DWORD dwFlags, COLORREF crText, COLORREF crShadow, int ixOffset, int iyOffset) return ENDIMPL(hdc, pszText, cch, prc, dwFlags, crText, crShadow, ixOffset, iyOffset)
WINCOMMCTRLAPI IMPL(DrawStatusTextA, void, HDC hDC, LPCRECT lprc, LPCSTR pszText, UINT uFlags) ENDIMPL(hDC, lprc, pszText, uFlags)
WINCOMMCTRLAPI IMPL(DrawStatusTextW, void, HDC hDC, LPCRECT lprc, LPCWSTR pszText, UINT uFlags) ENDIMPL(hDC, lprc, pszText, uFlags)
WINCOMMCTRLAPI IMPL(FlatSB_EnableScrollBar, BOOL, HWND h, int i, UINT u) return ENDIMPL(h, i, u)
WINCOMMCTRLAPI IMPL(FlatSB_GetScrollInfo, BOOL, HWND h, int code, LPSCROLLINFO si) return ENDIMPL(h, code, si)
WINCOMMCTRLAPI IMPL(FlatSB_GetScrollPos, int, HWND h, int code) return ENDIMPL(h, code)
WINCOMMCTRLAPI IMPL(FlatSB_GetScrollProp, BOOL, HWND h, int propIndex, LPINT p) return ENDIMPL(h, propIndex, p)
WINCOMMCTRLAPI IMPL(FlatSB_GetScrollPropPtr, BOOL, HWND h, int propIndex, PINT_PTR p) return ENDIMPL(h, propIndex, p)
WINCOMMCTRLAPI IMPL(FlatSB_GetScrollRange, BOOL, HWND h, int code, LPINT a, LPINT b) return ENDIMPL(h, code, a, b)
WINCOMMCTRLAPI IMPL(FlatSB_SetScrollInfo, int, HWND h, int code, LPSCROLLINFO psi, BOOL fRedraw) return ENDIMPL(h, code, psi, fRedraw)
WINCOMMCTRLAPI IMPL(FlatSB_SetScrollPos, int, HWND h, int code, int pos, BOOL fRedraw) return ENDIMPL(h, code, pos, fRedraw)
WINCOMMCTRLAPI IMPL(FlatSB_SetScrollProp, BOOL, HWND h, UINT index, INT_PTR newValue, BOOL b) return ENDIMPL(h, index, newValue, b)
WINCOMMCTRLAPI IMPL(FlatSB_SetScrollRange, int, HWND h, int code, int min, int max, BOOL fRedraw) return ENDIMPL(h, code, min, max, fRedraw)
WINCOMMCTRLAPI IMPL(FlatSB_ShowScrollBar, BOOL, HWND h, int code, BOOL b) return ENDIMPL(h, code, b)
WINCOMMCTRLAPI IMPL(GetEffectiveClientRect, void, HWND hWnd, LPRECT lprc, const INT* lpInfo) ENDIMPL(hWnd, lprc, lpInfo)
IMPL(GetMUILanguage, LANGID, void) return ENDIMPL()
IMPL(GetWindowSubclass, BOOL, HWND hWnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass, DWORD_PTR* pdwRefData) return ENDIMPL(hWnd, pfnSubclass, uIdSubclass, pdwRefData)
WINCOMMCTRLAPI IMPL(HIMAGELIST_QueryInterface, HRESULT, HIMAGELIST himl, REFIID riid, void** ppv) return ENDIMPL(himl, riid, ppv)
WINCOMMCTRLAPI IMPL(ImageList_Add, int, HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask) return ENDIMPL(himl, hbmImage, hbmMask)
WINCOMMCTRLAPI IMPL(ImageList_AddMasked, int, HIMAGELIST himl, HBITMAP hbmImage, COLORREF crMask) return ENDIMPL(himl, hbmImage, crMask)
WINCOMMCTRLAPI IMPL(ImageList_BeginDrag, BOOL, HIMAGELIST himlTrack, int iTrack, int dxHotspot, int dyHotspot) return ENDIMPL(himlTrack, iTrack, dxHotspot, dyHotspot)
WINCOMMCTRLAPI IMPL(ImageList_Copy, BOOL, HIMAGELIST himlDst, int iDst, HIMAGELIST himlSrc, int iSrc, UINT uFlags) return ENDIMPL(himlDst, iDst, himlSrc, iSrc, uFlags)
WINCOMMCTRLAPI IMPL(ImageList_Create, HIMAGELIST, int cx, int cy, UINT flags, int cInitial, int cGrow) return ENDIMPL(cx, cy, flags, cInitial, cGrow)
WINCOMMCTRLAPI IMPL(ImageList_Destroy, BOOL, HIMAGELIST himl) return ENDIMPL(himl)
WINCOMMCTRLAPI IMPL(ImageList_DragEnter, BOOL, HWND hwndLock, int x, int y) return ENDIMPL(hwndLock, x, y)
WINCOMMCTRLAPI IMPL(ImageList_DragLeave, BOOL, HWND hwndLock) return ENDIMPL(hwndLock)
WINCOMMCTRLAPI IMPL(ImageList_DragMove, BOOL, int x, int y) return ENDIMPL(x, y)
WINCOMMCTRLAPI IMPL(ImageList_DragShowNolock, BOOL, BOOL fShow) return ENDIMPL(fShow)
WINCOMMCTRLAPI IMPL(ImageList_Draw, BOOL, HIMAGELIST himl, int i, HDC hdcDst, int x, int y, UINT fStyle) return ENDIMPL(himl, i, hdcDst, x, y, fStyle)
WINCOMMCTRLAPI IMPL(ImageList_DrawEx, BOOL, HIMAGELIST himl, int i, HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle) return ENDIMPL(himl, i, hdcDst, x, y, dx, dy, rgbBk, rgbFg, fStyle)
WINCOMMCTRLAPI IMPL(ImageList_DrawIndirect, BOOL, IMAGELISTDRAWPARAMS* pimldp) return ENDIMPL(pimldp)
WINCOMMCTRLAPI IMPL(ImageList_Duplicate, HIMAGELIST, HIMAGELIST himl) return ENDIMPL(himl)
WINCOMMCTRLAPI IMPL(ImageList_EndDrag, void, void) ENDIMPL()
WINCOMMCTRLAPI IMPL(ImageList_GetBkColor, COLORREF, HIMAGELIST himl) return ENDIMPL(himl)
WINCOMMCTRLAPI IMPL(ImageList_GetDragImage, HIMAGELIST, POINT* ppt, POINT* pptHotspot) return ENDIMPL(ppt, pptHotspot)
WINCOMMCTRLAPI IMPL(ImageList_GetIcon, HICON, HIMAGELIST himl, int i, UINT flags) return ENDIMPL(himl, i, flags)
WINCOMMCTRLAPI IMPL(ImageList_GetIconSize, BOOL, HIMAGELIST himl, int* cx, int* cy) return ENDIMPL(himl, cx, cy)
WINCOMMCTRLAPI IMPL(ImageList_GetImageCount, int, HIMAGELIST himl) return ENDIMPL(himl)
WINCOMMCTRLAPI IMPL(ImageList_GetImageInfo, BOOL, HIMAGELIST himl, int i, IMAGEINFO* pImageInfo) return ENDIMPL(himl, i, pImageInfo)
WINCOMMCTRLAPI IMPL(ImageList_LoadImageA, HIMAGELIST, HINSTANCE hi, LPCSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags) return ENDIMPL(hi, lpbmp, cx, cGrow, crMask, uType, uFlags)
WINCOMMCTRLAPI IMPL(ImageList_LoadImageW, HIMAGELIST, HINSTANCE hi, LPCWSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags) return ENDIMPL(hi, lpbmp, cx, cGrow, crMask, uType, uFlags)
WINCOMMCTRLAPI IMPL(ImageList_Merge, HIMAGELIST, HIMAGELIST himl1, int i1, HIMAGELIST himl2, int i2, int dx, int dy) return ENDIMPL(himl1, i1, himl2, i2, dx, dy)
WINCOMMCTRLAPI IMPL(ImageList_Read, HIMAGELIST, struct IStream* pstm) return ENDIMPL(pstm)
WINCOMMCTRLAPI IMPL(ImageList_ReadEx, HRESULT, DWORD dwFlags, struct IStream* pstm, REFIID riid, PVOID* ppv) return ENDIMPL(dwFlags, pstm, riid, ppv)
WINCOMMCTRLAPI IMPL(ImageList_Remove, BOOL, HIMAGELIST himl, int i) return ENDIMPL(himl, i)
WINCOMMCTRLAPI IMPL(ImageList_Replace, BOOL, HIMAGELIST himl, int i, HBITMAP hbmImage, HBITMAP hbmMask) return ENDIMPL(himl, i, hbmImage, hbmMask)
WINCOMMCTRLAPI IMPL(ImageList_ReplaceIcon, int, HIMAGELIST himl, int i, HICON hicon) return ENDIMPL(himl, i, hicon)
WINCOMMCTRLAPI IMPL(ImageList_SetBkColor, COLORREF, HIMAGELIST himl, COLORREF clrBk) return ENDIMPL(himl, clrBk)
WINCOMMCTRLAPI IMPL(ImageList_SetDragCursorImage, BOOL, HIMAGELIST himlDrag, int iDrag, int dxHotspot, int dyHotspot) return ENDIMPL(himlDrag, iDrag, dxHotspot, dyHotspot)
WINCOMMCTRLAPI IMPL(ImageList_SetIconSize, BOOL, HIMAGELIST himl, int cx, int cy) return ENDIMPL(himl, cx, cy)
WINCOMMCTRLAPI IMPL(ImageList_SetImageCount, BOOL, HIMAGELIST himl, UINT uNewCount) return ENDIMPL(himl, uNewCount)
WINCOMMCTRLAPI IMPL(ImageList_SetOverlayImage, BOOL, HIMAGELIST himl, int iImage, int iOverlay) return ENDIMPL(himl, iImage, iOverlay)
WINCOMMCTRLAPI IMPL(ImageList_Write, BOOL, HIMAGELIST himl, struct IStream* pstm) return ENDIMPL(himl, pstm)
WINCOMMCTRLAPI IMPL(ImageList_WriteEx, HRESULT, HIMAGELIST himl, DWORD dwFlags, struct IStream* pstm) return ENDIMPL(himl, dwFlags, pstm)
WINCOMMCTRLAPI IMPL(InitCommonControls, void, void) ENDIMPL()
WINCOMMCTRLAPI IMPL(InitCommonControlsEx, BOOL, const INITCOMMONCONTROLSEX* picce) return ENDIMPL(picce)
IMPL(InitMUILanguage, void, LANGID uiLang) ENDIMPL(uiLang)
WINCOMMCTRLAPI IMPL(InitializeFlatSB, BOOL, HWND h) return ENDIMPL(h)
WINCOMMCTRLAPI IMPL(LBItemFromPt, int, HWND hLB, POINT pt, BOOL bAutoScroll) return ENDIMPL(hLB, pt, bAutoScroll)
WINCOMMCTRLAPI IMPL(LoadIconMetric, HRESULT, HINSTANCE hInst, PCWSTR pszName, int lims, HICON* phico) return ENDIMPL(hInst, pszName, lims, phico)
WINCOMMCTRLAPI IMPL(LoadIconWithScaleDown, HRESULT, HINSTANCE hInst, PCWSTR pszName, int cx, int cy, HICON* phico) return ENDIMPL(hInst, pszName, cx, cy, phico)
WINCOMMCTRLAPI IMPL(MakeDragList, BOOL, HWND hLB) return ENDIMPL(hLB)
WINCOMMCTRLAPI IMPL(MenuHelp, void, UINT uMsg, WPARAM wParam, LPARAM lParam, HMENU hMainMenu, HINSTANCE hInst, HWND hwndStatus, UINT* lpwIDs) ENDIMPL(uMsg, wParam, lParam, hMainMenu, hInst, hwndStatus, lpwIDs)
IMPL(RemoveWindowSubclass, BOOL, HWND hWnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass) return ENDIMPL(hWnd, pfnSubclass, uIdSubclass)
IMPL(SetWindowSubclass, BOOL, HWND hWnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) return ENDIMPL(hWnd, pfnSubclass, uIdSubclass, dwRefData)
WINCOMMCTRLAPI IMPL(ShowHideMenuCtl, BOOL, HWND hWnd, UINT_PTR uFlags, LPINT lpInfo) return ENDIMPL(hWnd, uFlags, lpInfo)
WINCOMMCTRLAPI IMPL(Str_SetPtrW, BOOL, LPWSTR* ppsz, LPCWSTR psz) return ENDIMPL(ppsz, psz)
WINCOMMCTRLAPI IMPL(TaskDialog, HRESULT, HWND hwndParent, HINSTANCE hInstance, PCWSTR pszWindowTitle, PCWSTR pszMainInstruction, PCWSTR pszContent, TASKDIALOG_COMMON_BUTTON_FLAGS dwCommonButtons, PCWSTR pszIcon, int* pnButton) return ENDIMPL(hwndParent, hInstance, pszWindowTitle, pszMainInstruction, pszContent, dwCommonButtons, pszIcon, pnButton)
WINCOMMCTRLAPI IMPL(TaskDialogIndirect, HRESULT, const TASKDIALOGCONFIG* pTaskConfig, int* pnButton, int* pnRadioButton, BOOL* pfVerificationFlagChecked) return ENDIMPL(pTaskConfig, pnButton, pnRadioButton, pfVerificationFlagChecked)
WINCOMMCTRLAPI IMPL(UninitializeFlatSB, HRESULT, HWND h) return ENDIMPL(h)
WINCOMMCTRLAPI IMPL(_TrackMouseEvent, BOOL, LPTRACKMOUSEEVENT lpEventTrack) return ENDIMPL(lpEventTrack)

/*
the above is generated by running

	|awk '{
		prefix = "WINCOMMCTRLAPI "
		i = 1
		if ($1 == "NODLL") {
			prefix = ""
			i = 2
		}
		funcname = $i
		i++
		ret = $i
		i++
		retstmt = "return "
		if (ret == "void")
			retstmt = ""
		if ($i == "void") {
			args = $i
			argnames = ""
		} else {
			args = ""
			argnames = ""
			for (; i <= NF; i++) {
				xa = $i " "
				if ($i == "const" || $i == "struct") {
					i++
					xa = xa $i " "
				}
				i++
				args = args xa $i ", "
				argnames = argnames $i ", "
			}
			sub(/, $/, "", args)
			sub(/, $/, "", argnames)
		}
		printf "%sIMPL(%s, %s, %s) %sENDIMPL(%s)\n", \
			prefix, funcname, ret, args, retstmt, argnames
	}'

(the | at the beginning is for acme, my text editor) on the following:

CreateMappedBitmap HBITMAP HINSTANCE hInstance INT_PTR idBitmap UINT wFlags LPCOLORMAP lpColorMap int iNumMaps
CreateStatusWindowA HWND LONG style LPCSTR lpszText HWND hwndParent UINT wID
CreateStatusWindowW HWND LONG style LPCWSTR lpszText HWND hwndParent UINT wID
CreateToolbarEx HWND HWND hwnd DWORD ws UINT wID int nBitmaps HINSTANCE hBMInst UINT_PTR wBMID LPCTBBUTTON lpButtons int iNumButtons int dxButton int dyButton int dxBitmap int dyBitmap UINT uStructSize
CreateUpDownControl HWND DWORD dwStyle int x int y int cx int cy HWND hParent int nID HINSTANCE hInst HWND hBuddy int nUpper int nLower int nPos
DPA_Clone HDPA const HDPA hdpa HDPA hdpaNew
DPA_Create HDPA int cItemGrow
DPA_CreateEx HDPA int cpGrow HANDLE hheap
DPA_DeleteAllPtrs BOOL HDPA hdpa
DPA_DeletePtr PVOID HDPA hdpa int i
DPA_Destroy BOOL HDPA hdpa
DPA_DestroyCallback void HDPA hdpa PFNDAENUMCALLBACK pfnCB void* pData
DPA_EnumCallback void HDPA hdpa PFNDAENUMCALLBACK pfnCB void* pData
DPA_GetPtr PVOID HDPA hdpa INT_PTR i
DPA_GetPtrIndex int HDPA hdpa const void* p
DPA_GetSize ULONGLONG HDPA hdpa
DPA_Grow BOOL HDPA pdpa int cp
DPA_InsertPtr int HDPA hdpa int i void* p
DPA_LoadStream HRESULT HDPA* phdpa PFNDPASTREAM pfn struct IStream* pstream void* pvInstData
DPA_Merge BOOL HDPA hdpaDest HDPA hdpaSrc DWORD dwFlags PFNDACOMPARE pfnCompare PFNDPAMERGE pfnMerge LPARAM lParam
DPA_SaveStream HRESULT HDPA hdpa PFNDPASTREAM pfn struct IStream* pstream void* pvInstData
DPA_Search int HDPA hdpa void* pFind int iStart PFNDACOMPARE pfnCompare LPARAM lParam UINT options
DPA_SetPtr BOOL HDPA hdpa int i void* p
DPA_Sort BOOL HDPA hdpa PFNDACOMPARE pfnCompare LPARAM lParam
DSA_Clone HDSA HDSA hdsa
DSA_Create HDSA int cbItem int cItemGrow
DSA_DeleteAllItems BOOL HDSA hdsa
DSA_DeleteItem BOOL HDSA hdsa int i
DSA_Destroy BOOL HDSA hdsa
DSA_DestroyCallback void HDSA hdsa PFNDAENUMCALLBACK pfnCB void* pData
DSA_EnumCallback void HDSA hdsa PFNDAENUMCALLBACK pfnCB void* pData
DSA_GetItem BOOL HDSA hdsa int i void* pitem
DSA_GetItemPtr PVOID HDSA hdsa int i
DSA_GetSize ULONGLONG HDSA hdsa
DSA_InsertItem int HDSA hdsa int i xDSAInsertItemvoid pitem
DSA_SetItem BOOL HDSA hdsa int i const void* pitem
DSA_Sort BOOL HDSA pdsa PFNDACOMPARE pfnCompare LPARAM lParam
NODLL DefSubclassProc LRESULT HWND hWnd UINT uMsg WPARAM wParam LPARAM lParam
DrawInsert void HWND handParent HWND hLB int nItem
NODLL DrawShadowText int HDC hdc LPCWSTR pszText UINT cch RECT* prc DWORD dwFlags COLORREF crText COLORREF crShadow int ixOffset int iyOffset
DrawStatusTextA void HDC hDC LPCRECT lprc LPCSTR pszText UINT uFlags
DrawStatusTextW void HDC hDC LPCRECT lprc LPCWSTR pszText UINT uFlags
FlatSB_EnableScrollBar BOOL HWND h int i UINT u
FlatSB_GetScrollInfo BOOL HWND h int code LPSCROLLINFO si
FlatSB_GetScrollPos int HWND h int code
FlatSB_GetScrollProp BOOL HWND h int propIndex LPINT p
FlatSB_GetScrollPropPtr BOOL HWND h int propIndex PINT_PTR p
FlatSB_GetScrollRange BOOL HWND h int code LPINT a LPINT b
FlatSB_SetScrollInfo int HWND h int code LPSCROLLINFO psi BOOL fRedraw
FlatSB_SetScrollPos int HWND h int code int pos BOOL fRedraw
FlatSB_SetScrollProp BOOL HWND h UINT index INT_PTR newValue BOOL b
FlatSB_SetScrollRange int HWND h int code int min int max BOOL fRedraw
FlatSB_ShowScrollBar BOOL HWND h int code BOOL b
GetEffectiveClientRect void HWND hWnd LPRECT lprc const INT* lpInfo
NODLL GetMUILanguage LANGID void
NODLL GetWindowSubclass BOOL HWND hWnd SUBCLASSPROC pfnSubclass UINT_PTR uIdSubclass DWORD_PTR* pdwRefData
HIMAGELIST_QueryInterface HRESULT HIMAGELIST himl REFIID riid void** ppv
ImageList_Add int HIMAGELIST himl HBITMAP hbmImage HBITMAP hbmMask
ImageList_AddMasked int HIMAGELIST himl HBITMAP hbmImage COLORREF crMask
ImageList_BeginDrag BOOL HIMAGELIST himlTrack int iTrack int dxHotspot int dyHotspot
ImageList_Copy BOOL HIMAGELIST himlDst int iDst HIMAGELIST himlSrc int iSrc UINT uFlags
ImageList_Create HIMAGELIST int cx int cy UINT flags int cInitial int cGrow
ImageList_Destroy BOOL HIMAGELIST himl
ImageList_DragEnter BOOL HWND hwndLock int x int y
ImageList_DragLeave BOOL HWND hwndLock
ImageList_DragMove BOOL int x int y
ImageList_DragShowNolock BOOL BOOL fShow
ImageList_Draw BOOL HIMAGELIST himl int i HDC hdcDst int x int y UINT fStyle
ImageList_DrawEx BOOL HIMAGELIST himl int i HDC hdcDst int x int y int dx int dy COLORREF rgbBk COLORREF rgbFg UINT fStyle
ImageList_DrawIndirect BOOL IMAGELISTDRAWPARAMS* pimldp
ImageList_Duplicate HIMAGELIST HIMAGELIST himl
ImageList_EndDrag void void
ImageList_GetBkColor COLORREF HIMAGELIST himl
ImageList_GetDragImage HIMAGELIST POINT* ppt POINT* pptHotspot
ImageList_GetIcon HICON HIMAGELIST himl int i UINT flags
ImageList_GetIconSize BOOL HIMAGELIST himl int* cx int* cy
ImageList_GetImageCount int HIMAGELIST himl
ImageList_GetImageInfo BOOL HIMAGELIST himl int i IMAGEINFO* pImageInfo
ImageList_LoadImageA HIMAGELIST HINSTANCE hi LPCSTR lpbmp int cx int cGrow COLORREF crMask UINT uType UINT uFlags
ImageList_LoadImageW HIMAGELIST HINSTANCE hi LPCWSTR lpbmp int cx int cGrow COLORREF crMask UINT uType UINT uFlags
ImageList_Merge HIMAGELIST HIMAGELIST himl1 int i1 HIMAGELIST himl2 int i2 int dx int dy
ImageList_Read HIMAGELIST struct IStream* pstm
ImageList_ReadEx HRESULT DWORD dwFlags struct IStream* pstm REFIID riid PVOID* ppv
ImageList_Remove BOOL HIMAGELIST himl int i
ImageList_Replace BOOL HIMAGELIST himl int i HBITMAP hbmImage HBITMAP hbmMask
ImageList_ReplaceIcon int HIMAGELIST himl int i HICON hicon
ImageList_SetBkColor COLORREF HIMAGELIST himl COLORREF clrBk
ImageList_SetDragCursorImage BOOL HIMAGELIST himlDrag int iDrag int dxHotspot int dyHotspot
ImageList_SetIconSize BOOL HIMAGELIST himl int cx int cy
ImageList_SetImageCount BOOL HIMAGELIST himl UINT uNewCount
ImageList_SetOverlayImage BOOL HIMAGELIST himl int iImage int iOverlay
ImageList_Write BOOL HIMAGELIST himl struct IStream* pstm
ImageList_WriteEx HRESULT HIMAGELIST himl DWORD dwFlags struct IStream* pstm
InitCommonControls void void
InitCommonControlsEx BOOL const INITCOMMONCONTROLSEX* picce
NODLL InitMUILanguage void LANGID uiLang
InitializeFlatSB BOOL HWND h
LBItemFromPt int HWND hLB POINT pt BOOL bAutoScroll
LoadIconMetric HRESULT HINSTANCE hInst PCWSTR pszName int lims HICON* phico
LoadIconWithScaleDown HRESULT HINSTANCE hInst PCWSTR pszName int cx int cy HICON* phico
MakeDragList BOOL HWND hLB
MenuHelp void UINT uMsg WPARAM wParam LPARAM lParam HMENU hMainMenu HINSTANCE hInst HWND hwndStatus UINT* lpwIDs
NODLL RemoveWindowSubclass BOOL HWND hWnd SUBCLASSPROC pfnSubclass UINT_PTR uIdSubclass
NODLL SetWindowSubclass BOOL HWND hWnd SUBCLASSPROC pfnSubclass UINT_PTR uIdSubclass DWORD_PTR dwRefData
ShowHideMenuCtl BOOL HWND hWnd UINT_PTR uFlags LPINT lpInfo
Str_SetPtrW BOOL LPWSTR* ppsz LPCWSTR psz
TaskDialog HRESULT HWND hwndParent HINSTANCE hInstance PCWSTR pszWindowTitle PCWSTR pszMainInstruction PCWSTR pszContent TASKDIALOG_COMMON_BUTTON_FLAGS dwCommonButtons PCWSTR pszIcon int* pnButton
TaskDialogIndirect HRESULT const TASKDIALOGCONFIG* pTaskConfig int* pnButton int* pnRadioButton BOOL* pfVerificationFlagChecked
UninitializeFlatSB HRESULT HWND h
_TrackMouseEvent BOOL LPTRACKMOUSEEVENT lpEventTrack

*/
