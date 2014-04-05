// 4 april 2014
#include <stdio.h>
#include <Cocoa/Cocoa.h>
#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>

int main(void)
{
	printf("@encode: %s\n", @encode(void(id, SEL, NSRect)));
	printf("stored: %s\n",
		method_getTypeEncoding(class_getInstanceMethod(
			[NSView class],
			@selector(drawRect:))));
	return 0;
}
