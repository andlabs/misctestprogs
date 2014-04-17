// scratch Windows program by pietro gagliardi 17 april 2014
// borrows code from the scratch GTK+ program (16-17 april 2014) and from code written 31 march 2014
#define _UNICODE
#define UNICODE
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include <windows.h>

#ifdef  _MSC_VER
#error sorry! the scratch windows program relies on mingw-only functionality! (specifically: asprintf(), getopt_long_only())
#endif

// cheating: we store the help string in the flag argument, collect them, then overwrite them with NULL in init() so getopt_long_only() will return val and not overwrite a string (apparently I'm not the first to think of this: GerbilSoft says GNU tools do this too)
#define flagBool(name, help, short) { name, no_argument, (int *) help, short }
#define flagString(name, help, short) { name, required_argument, (int *) help, short }
static struct option flags[] = {
	// place other options here
	flagBool("help", "show help and quit", 'h'),
	{ 0, 0, 0, 0 },
};

void panic(char *fmt, ...);

void init(int argc, char *argv[]);

char *args = "";		// other command-line arguments here, if any

BOOL parseArgs(int argc, char *argv[])
{
	// parse arguments here, if any; use optind
	// return TRUE if parsed successfully; FALSE otherwise
	// (returning FALSE will show usage and quit)
	return TRUE;
}

int main(int argc, char *argv[])
{
	init(argc, argv);

	SetLastError(4);
	panic("last error test");
	return 0;
}

void init(int argc, char *argv[])
{
	int usageExit = 1;
	char *mode;
	char *opthelp[512];		/* more than enough */
	int i;

	for (i = 0; flags[i].name != 0; i++) {
		opthelp[i] = (char *) flags[i].flag;
		flags[i].flag = NULL;
	}

	for (;;) {
		int c;

		c = getopt_long_only(argc, argv, "", flags, NULL);
		if (c == -1)
			break;
		switch (c) {
		case 'h':		/* -help */
			usageExit = 0;
			goto usage;
		case '?':
			/* getopt_long_only() should have printed something since we did not set opterr to 0 */
			goto usage;
		default:
			fprintf(stderr, "internal error: getopt_long_only() returned %d\n", c);
			exit(1);
		}
	}

	if (parseArgs(argc, argv) == TRUE)
		return;
	// otherwise fall through

usage:
	fprintf(stderr, "usage: %s [options]", argv[0]);
	if (args != NULL && *args != '\0')
		fprintf(stderr, " %s", args);
	fprintf(stderr, "\n");
	for (i = 0; flags[i].name != 0; i++)
		fprintf(stderr, "\t-%s%s - %s\n",
			flags[i].name,
			(flags[i].has_arg == required_argument) ? " string" : "",
			opthelp[i]);
	exit(usageExit);
}

void panic(char *fmt, ...)
{
	char *msg;
	TCHAR *lerrmsg;
	char *fullmsg;
	va_list arg;
	DWORD lasterr;
	DWORD lerrsuccess;

	lasterr = GetLastError();
	va_start(arg, fmt);
	if (vasprintf(&msg, fmt, arg) == -1) {
		fprintf(stderr, "critical error: vasprintf() failed in panic() preparing panic message; fmt = \"%s\"\n", fmt);
		abort();
	}
	// according to http://msdn.microsoft.com/en-us/library/windows/desktop/ms680582%28v=vs.85%29.aspx
	lerrsuccess = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, lasterr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lerrmsg, 0, NULL);
	if (lerrsuccess == 0) {
		fprintf(stderr, "critical error: FormatMessage() failed in panic() preparing GetLastError() string; panic message = \"%s\", last error in panic(): %ld, last error from FormatMessage(): %ld\n", msg, lasterr, GetLastError());
		abort();
	}
	if (asprintf(&fullmsg, "panic: %s\nlast error: %S\n", msg, lerrmsg) == -1) {
		fprintf(stderr, "critical error: asprintf() failed in panic() preparing full report; panic message = \"%s\", last error message: \"%S\"\n", msg, lerrmsg);
		abort();
	}
	fprintf(stderr, "%s\n", fullmsg);
	va_end(arg);
	exit(1);
}
