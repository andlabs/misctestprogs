// 22 january 2017

struct rect {
	double x;
	double y;
	double width;
	double height;
};

const struct rect macrects_short[] = {
	{ 0, 0, 614.537, 17 },
	{ 0, 17, 614.011, 17 },
	{ 0, 34, 605, 23 },
	{ 0, 57, 268.194, 17 },
	{ 100, 0, 0, 0 },
};

const struct rect macrects[] = {
	{ 0, 0, 618.7, 17 },
	{ 0, 17, 610.155, 17 },
	{ 0, 34, 608.139, 17 },
	{ 0, 51, 600.776, 17 },
	{ 0, 68, 602.861, 17 },
	{ 0, 85, 613.508, 23 },
	{ 0, 108, 461.23, 23 },
	{ 100, 0, 0, 0 },
};

const struct rect unixrects_short[] = {
	{ 0, 0, 542, 28 },
	{ 0, 28, 613, 28 },
	{ 0, 56, 582, 28 },
	{ 0, 84, 605, 28 },
	{ 0, 112, 347, 28 },
	{ 100, 0, 0, 0 },
};

const struct rect unixrects[] = {
	{ 0, 0, 591, 28 },
	{ 0, 28, 600, 28 },
	{ 0, 56, 584, 28 },
	{ 0, 84, 568, 28 },
	{ 0, 112, 582, 28 },
	{ 0, 140, 578, 28 },
	{ 0, 168, 546, 28 },
	{ 0, 196, 576, 28 },
	{ 0, 224, 546, 28 },
	{ 0, 252, 84, 28 },
	{ 100, 0, 0, 0 },
};

const struct rect winrects_short[] = {
	{ 0, 0, 557.712, 24.8281 },
	{ 0, 24.8281, 613.457, 24.8281 },
	{ 0, 49.6563, 557.22, 24.8281 },
	{ 0, 74.4844, 601.253, 24.8281 },
	{ 0, 99.3125, 445.53, 24.8281 },
	{ 100, 0, 0, 0 },
};

const struct rect winrects[] = {
	{ 0, 0, 609.283, 24.8281 },
	{ 0, 24.8281, 595.283, 24.8281 },
	{ 0, 49.6563, 588.41, 24.8281 },
	{ 0, 74.4844, 546.018, 24.8281 },
	{ 0, 99.3125, 602.191, 24.8281 },
	{ 0, 124.141, 625.06, 24.8281 },
	{ 0, 148.969, 617.868, 24.8281 },
	{ 0, 173.797, 623.656, 24.8281 },
	{ 0, 198.625, 558.268, 24.8281 },
	{ 0, 223.453, 85.8503, 24.8281 },
	{ 100, 0, 0, 0 },
};

struct rectEntry {
	const CHARTYPE *name;
	const struct rect *rects;
};

const struct rectEntry rectList[] = {
	{ TOCHAR("Mac Rects (Short)"), macrects_short, },
	{ TOCHAR("Mac Rects"), macrects, },
	{ TOCHAR("Pango Rects (Short)"), unixrects_short, },
	{ TOCHAR("Pango Rects"), unixrects, },
	{ TOCHAR("Windows Rects (Short)"), winrects_short, },
	{ TOCHAR("Windows Rects"), winrects, },
	{ NULL, NULL },
};

struct color {
	double r;
	double g;
	double b;
	double a;
};

const struct color fillcolors[4] = {
	{ 1.0, 0.0, 0.0, 0.5 },
	{ 0.0, 1.0, 0.0, 0.5 },
	{ 0.0, 0.0, 1.0, 0.5 },
	{ 0.0, 1.0, 1.0, 0.5 },
};

const struct color singlefill = { 0.502, 0, 0, 1.0 };
