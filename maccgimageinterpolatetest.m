// 24 june 2016
// based on code from 22 june 2016
#import <Cocoa/Cocoa.h>

NSLayoutConstraint *mkConstraint(id view1, NSLayoutAttribute attr1, NSLayoutRelation relation, id view2, NSLayoutAttribute attr2, CGFloat multiplier, CGFloat c)
{
	return [NSLayoutConstraint constraintWithItem:view1
		attribute:attr1
		relatedBy:relation
		toItem:view2
		attribute:attr2
		multiplier:multiplier
		constant:c];
}

@interface drawingView : NSView
@end

CGImageRef should, shouldNot;
#define filename "16x16test_24june2016.png"

void loadImages(void)
{
	CGDataProviderRef dp;

	dp = CGDataProviderCreateWithFilename(filename);
	should = CGImageCreateWithPNGDataProvider(dp, NULL, true, kCGRenderingIntentDefault);
	CGDataProviderRelease(dp);
	dp = CGDataProviderCreateWithFilename(filename);
	shouldNot = CGImageCreateWithPNGDataProvider(dp, NULL, false, kCGRenderingIntentDefault);
	CGDataProviderRelease(dp);
}

void drawQuadrant(CGContextRef c, NSString *label, CGImageRef image, CGInterpolationQuality interpolation, double x, double y)
{
	CGContextSaveGState(c);
	CGContextSetInterpolationQuality(c, interpolation);

	[label drawAtPoint:NSMakePoint(x, y + 256 + 5)
		withAttributes:nil];

	CGContextDrawImage(c,
		CGRectMake(x, y + (256 - 16), 16, 16),
		image);

	CGContextDrawImage(c,
		CGRectMake(x + 16 + 5, y, 256, 256),
		image);

	CGContextRestoreGState(c);
}

@implementation drawingView

- (id)initWithFrame:(NSRect)r
{
	self = [super initWithFrame:r];
	if (self)
		loadImages();
	return self;
}

- (void)drawRect:(NSRect)r
{
	CGContextRef c;

	c = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];
#define quadHeight (20 + 5 + 256)
	drawQuadrant(c, @"Should, None",
		should, kCGInterpolationNone,
		5, 5 + quadHeight + 5);
	drawQuadrant(c, @"Should, Medium",
		should, kCGInterpolationMedium,
		5 + quadHeight + 5, 5 + quadHeight + 5);
	drawQuadrant(c, @"Should Not, None",
		shouldNot, kCGInterpolationNone,
		5, 5);
	drawQuadrant(c, @"Should Not, Medium",
		shouldNot, kCGInterpolationMedium,
		5 + quadHeight + 5, 5);
}

@end

@interface appDelegate : NSObject<NSApplicationDelegate>
@end

@implementation appDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
	NSWindow *w;
	NSView *contentView;
	drawingView *dv;

	w = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0,
		20 + 5 + quadHeight + 5 + quadHeight + 5 + 20,
		20 + 5 + quadHeight + 5 + quadHeight + 5 + 20)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	contentView = [w contentView];

	dv = [[drawingView alloc] initWithFrame:NSZeroRect];
	[dv setTranslatesAutoresizingMaskIntoConstraints:NO];
	[contentView addSubview:dv];

	[contentView addConstraint:mkConstraint(contentView, NSLayoutAttributeLeading,
		NSLayoutRelationEqual,
		dv, NSLayoutAttributeLeading,
		1, -20)];
	[contentView addConstraint:mkConstraint(contentView, NSLayoutAttributeTop,
		NSLayoutRelationEqual,
		dv, NSLayoutAttributeTop,
		1, -20)];
	[contentView addConstraint:mkConstraint(contentView, NSLayoutAttributeTrailing,
		NSLayoutRelationEqual,
		dv, NSLayoutAttributeTrailing,
		1, 20)];
	[contentView addConstraint:mkConstraint(contentView, NSLayoutAttributeBottom,
		NSLayoutRelationEqual,
		dv, NSLayoutAttributeBottom,
		1, 20)];

	[w makeKeyAndOrderFront:nil];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
	return YES;
}

@end

int main(void)
{
	NSApplication *app;

	app = [NSApplication sharedApplication];
	[app setActivationPolicy:NSApplicationActivationPolicyRegular];
	[app setDelegate:[appDelegate new]];
	[app run];
	return 0;
}
