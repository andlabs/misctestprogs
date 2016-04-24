// 23 april 2016
#include <stdio.h>
#include <wchar.h>

typedef wchar_t WCHAR;

const WCHAR *L__FILE__ = L"L__FILE__";
const WCHAR *L__func__ = L"L__func__";

void pr(const WCHAR *what, int line)
{
	wprintf(L"%s %d\n", what, line);
}

#define A() pr(L ## __FILE__, __LINE__); pr(L ## __func__, __LINE__)

#define ws(s) L ## s

#define B() pr(ws(__FILE__), __LINE__); pr(ws(__func__), __LINE__)

#define ws2(s) ws(s)

#define C() pr(ws2(__FILE__), __LINE__); pr(ws2(__FUNCTION__), __LINE__)

int main(void)
{
	printf("%d\n", __LINE__);
	A();
	B();
	C();
	return 0;
}
