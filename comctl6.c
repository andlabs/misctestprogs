// 8 may 2014
#define _UNICODE
#define UNICODE
#define STRICT
#include <windows.h>
#include <string.h>

static const char *manifest = "xxxxxxx";		// not WCHAR!
static ACTCTX context;
static HANDLE contextHandle;
static ULONG_PTR contextCookie;
static const WCHAR manifestfile[MAX_PATH + 1] = { L'\0' };

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

BOOL comctl6Enable(void)
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

BOOL comctl6Disable(void)
{
	return DeactivateActCtx(0, contextCookie);
}
