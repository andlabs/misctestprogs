// 14 september 2015
#import <Cocoa/Cocoa.h>

int main(void)
{
#define show(c) printf(#c " %f\n", (double) c)
	show(NSFontWeightUltraLight);
	show(NSFontWeightThin);
	show(NSFontWeightLight);
	show(NSFontWeightRegular);
	show(NSFontWeightMedium);
	show(NSFontWeightSemibold);
	show(NSFontWeightBold);
	show(NSFontWeightHeavy);
	show(NSFontWeightBlack);
	return 0;
}
