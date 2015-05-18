// 24 january 2015
#include <windows.h>
#include <stdio.h>
#include <stddef.h>

int main(void)
{
	printf("%d\n", sizeof (BITMAP));
#define O(f) printf("%s %x\n", #f, offsetof(BITMAP, f))
	O(bmType);
	O(bmWidth);
	O(bmHeight);
	O(bmWidthBytes);
	O(bmPlanes);
	O(bmBitsPixel);
	O(bmBits);
	return 0;
}
