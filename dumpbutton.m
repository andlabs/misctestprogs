// 4 april 2014

#include <Cocoa/Cocoa.h>
#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>

void logbool(char *name, BOOL b)
{
	if (b) {
		NSLog(@"%s: true", name);
		return;
	}
	NSLog(@"%s: false", name);
}

void loguint(char *name, NSUInteger i)
{
	NSLog(@"%s: %lu", name, (unsigned long) i);
}

void logint(char *name, NSInteger i)
{
	NSLog(@"%s: %ld", name, (long) i);
}

void logstr(char *name, NSString *s)
{
	NSLog(@"%s: %@", name, s);
}

void logrect(char *name, NSRect r)
{
	logstr(name, NSStringFromRect(r));
}

void logsize(char *name, NSSize s)
{
	logstr(name, NSStringFromSize(s));
}

#define SUPER(super, sel) objc_msgSendSuper(&super, @selector(sel))

void dumpButton(NSButton *b)
{
	logbool("isBordered", [b isBordered]);
	logbool("isTransparent", [b isTransparent]);
	loguint("bezelStyle", [b bezelStyle]);
	logbool("showsBorderOnlyWhileMouseInside", [b showsBorderOnlyWhileMouseInside]);
	logint("tag", [b tag]);
	loguint("alignment", (NSUInteger) [b alignment]);
	logrect("frame", [b frame]);
	logrect("bounds", [b bounds]);

	NSButtonCell *bc = (NSButtonCell *) [b cell];
	struct objc_super c;

	c.receiver = bc;
	c.super_class = (Class) objc_getClass("NSCell");

	NSLog(@" ");
	NSLog(@"cell:");
	loguint("bezelStyle", [bc bezelStyle]);
	loguint("gradientType", [bc gradientType]);
	logbool("isOpaque", [bc isOpaque]);
	logbool("isTransparent", [bc isTransparent]);
	logbool("showsBorderOnlyWhileMouseInside", [bc showsBorderOnlyWhileMouseInside]);
	logint("highlightsBy", [bc highlightsBy]);
	logint("showsStateBy", [bc showsStateBy]);
	logint("tag", [bc tag]);
	logint("cellAttribute:NSCellDisabled", [bc cellAttribute:NSCellDisabled]);
	logint("cellAttribute:NSCellState", [bc cellAttribute:NSCellState]);
	logint("cellAttribute:NSPushInCell", [bc cellAttribute:NSPushInCell]);
	logint("cellAttribute:NSCellEditable", [bc cellAttribute:NSCellEditable]);
	logint("cellAttribute:NSChangeGrayCell", [bc cellAttribute:NSChangeGrayCell]);
	logint("cellAttribute:NSCellHighlighted", [bc cellAttribute:NSCellHighlighted]);
	logint("cellAttribute:NSCellLightsByContents", [bc cellAttribute:NSCellLightsByContents]);
	logint("cellAttribute:NSCellLightsByGray", [bc cellAttribute:NSCellLightsByGray]);
	logint("cellAttribute:NSChangeBackgroundCell", [bc cellAttribute:NSChangeBackgroundCell]);
	logint("cellAttribute:NSCellLightsByBackground", [bc cellAttribute:NSCellLightsByBackground]);
	logint("cellAttribute:NSCellIsBordered", [bc cellAttribute:NSCellIsBordered]);
	logint("cellAttribute:NSCellHasOverlappingImage", [bc cellAttribute:NSCellHasOverlappingImage]);
	logint("cellAttribute:NSCellHasImageHorizontal", [bc cellAttribute:NSCellHasImageHorizontal]);
	logint("cellAttribute:NSCellHasImageOnLeftOrBottom", [bc cellAttribute:NSCellHasImageOnLeftOrBottom]);
	logint("cellAttribute:NSCellChangesContents", [bc cellAttribute:NSCellChangesContents]);
	logint("cellAttribute:NSCellIsInsetButton", [bc cellAttribute:NSCellIsInsetButton]);
	logint("cellAttribute:NSCellAllowsMixedState", [bc cellAttribute:NSCellAllowsMixedState]);
	loguint("type", [bc type]);
	logbool("isBezeled", [bc isBezeled]);
	logbool("isBordered", [bc isBordered]);
	logbool("isOpaque", SUPER(c, isOpaque));
	loguint("controlTint", [bc controlTint]);
	loguint("backgroundStyle", [bc backgroundStyle]);
	loguint("interiorBackgroundStyle", [bc interiorBackgroundStyle]);
	loguint("alignment", (NSUInteger) [bc alignment]);
	loguint("lineBreakMode", [bc lineBreakMode]);
	logint("tag", SUPER(c, tag));
	loguint("focusRingType", (NSUInteger) [bc focusRingType]);
	logsize("cellSize", [bc cellSize]);
	loguint("controlSize", [bc controlSize]);
	logbool("isHighlighted", [bc isHighlighted]);
	logbool("usesSingleLineMode", [bc usesSingleLineMode]);
}
