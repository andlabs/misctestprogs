// 9 june 2014
#define UNICODE
#define _UNICDOE
#define STRICT
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

void panic(char *, ...);

uint8_t *buf;
size_t nbuf = 0;
size_t pbuf = 0;

void expand(size_t by)
{
	if ((pbuf + by) < nbuf)
		return;
	buf = (uint8_t *) realloc(buf, (nbuf * 2) + sizeof (uint8_t));
	if (buf == NULL)
		panic("error expanding dialog template buffer");
	nbuf *= 2;
}

void writebyte(uint8_t b)
{
	expand(1);
	buf[pbuf++] = b;
}

void writeword(uint16_t w)
{
	expand(2);
	// little endian
	buf[pbuf++] = (uint8_t) (w & 0xFF);
	buf[pbuf++] = (uint8_t) ((w >> 8) & 0xFF);
}

void writedword(uint32_t w)
{
	expand(4);
	// little endian
	buf[pbuf++] = (uint8_t) (w & 0xFF);
	buf[pbuf++] = (uint8_t) ((w >> 8) & 0xFF);
	buf[pbuf++] = (uint8_t) ((w >> 16) & 0xFF);
	buf[pbuf++] = (uint8_t) ((w >> 24) & 0xFF);
}

void writestring(WCHAR *str)
{
	size_t i, len;
	uint8_t *raw = (uint8_t *) str;

	len = (wcslen(str) + 1) * sizeof (uint8_t);		// include terminating null
	for (i = 0; i <= len; i++)
		writebyte(str[i]);
}

void aligndword(void)
{
	while ((pbuf % 4) != 0)
		writebyte(0);
}

LOGFONT getMessageFont(void)
{
	NONCLIENTMETRICS ncm;

	ncm.cbSize = sizeof (NONCLIENTMETRICS);
	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, sizeof (NONCLIENTMETRICS)) == 0)
		panic("error getting message font");
	return ncm.lfMessageFont;
}

void genDialog(void)
{
	HDC hdc;
	LOGFONT msgfont;

	hdc = GetDC(NULL);
	if (hdc == NULL)
		panic("error getting device context for the current screen");
	msgfont = getMessageFont();
	nbuf = 1024;
	buf = (uint8_t *) malloc(nbuf * sizeof (uint8_t));
	if (buf == NULL)
		panic("error allocating dialog template buffer");

	writeword(1);			// version
	writeword(0xFFFF);		// signature
	writedword(0);			// help ID
	writedword(0);			// extended window style
// via http://blogs.msdn.com/b/oldnewthing/archive/2005/04/29/412577.aspx
#define STYLE (WS_CAPTION | WS_SYSMENU | DS_SETFONT | DS_MODALFRAME)
	writedword(STYLE);		// window style
	writeword(0);			// number of controls
	writeword(0);			// x
	writeword(0);			// y
	writeword(100);		// width
	writeword(100);		// height
	writestring(L"");		// menu
	writestring(L"");		// window class
	writestring(L"");		// window title

	// the next needs to be a point size
	WORD pointsize;

	if (msgfont.lfHeight < 0)		// pixels; convert to points
		// via http://blogs.msdn.com/b/oldnewthing/archive/2005/04/29/412577.aspx
		pointsize = (WORD) (-MulDiv(msgfont.lfHeight, 72,
			GetDeviceCaps(hdc, LOGPIXELSY)));
	else						// already points
		pointsize = (WORD) msgfont.lfHeight;

	writeword(pointsize);		// font point size
	writeword((uint16_t) msgfont.lfWeight);		// font weight
	writebyte((uint8_t) msgfont.lfItalic);			// font italic
	writebyte((uint8_t) msgfont.lfCharSet);		// font character set
	writestring(msgfont.lfFaceName);			// font face name
	wprintf(L"%s %d weight:%d italic:%d charset:%d\n", msgfont.lfFaceName,
		pointsize, msgfont.lfWeight, msgfont.lfItalic, msgfont.lfCharSet);

	aligndword();

	if (ReleaseDC(NULL, hdc) == 0)
		panic("error releasing screen device context");
}

INT_PTR CALLBACK dlgproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;

	if (uMsg != WM_INITDIALOG)
		return FALSE;
	r.left = 0;
	r.top = 0;
	r.right = 4;		// via http://blogs.msdn.com/b/oldnewthing/archive/2005/03/30/403711.aspx
	r.bottom = 8;
	if (MapDialogRect(hwnd, &r) == 0)
		panic("error getting dialog base units");
	printf("[%d %d %d %d]\n", r.left, r.top, r.right, r.bottom);
	if (EndDialog(hwnd, 1) == 0)
		panic("error ending dialog");
	return FALSE;
}

int main(void)
{
	genDialog();
	if (DialogBoxIndirect(GetModuleHandle(NULL), (LPCDLGTEMPLATE) buf, NULL, dlgproc) <= 0)
		panic("error running dialog box");
	return 0;
}

void panic(char *fmt, ...)
{
	va_list arg;
	int xerrno;
	DWORD xlasterr;

	xerrno = errno;
	xlasterr = GetLastError();
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	fprintf(stderr, "\nerrno: %d\nWindows last error: %d\n", xerrno, xlasterr);
	va_end(arg);
	exit(EXIT_FAILURE);
}
