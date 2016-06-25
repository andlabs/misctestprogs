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

CGImageRef i16, i32;
NSImage *image;
#define filename "16x16test_24june2016.png"
#define filename2 "32x32test_24june2016.png"

void loadImages(void)
{
	CGDataProviderRef dp;
	NSBitmapImageRep *rep;

	dp = CGDataProviderCreateWithFilename(filename);
	i16 = CGImageCreateWithPNGDataProvider(dp, NULL, true, kCGRenderingIntentDefault);
	CGDataProviderRelease(dp);
	dp = CGDataProviderCreateWithFilename(filename2);
	i32 = CGImageCreateWithPNGDataProvider(dp, NULL, true, kCGRenderingIntentDefault);
	CGDataProviderRelease(dp);

	image = [[NSImage alloc] initWithSize:NSMakeSize(16, 16)];
	rep = [[NSBitmapImageRep alloc] initWithCGImage:i16];
	[image addRepresentation:rep];
	[rep setSize:NSMakeSize(16, 16)];
	[rep release];
	rep = [[NSBitmapImageRep alloc] initWithCGImage:i32];
	[image addRepresentation:rep];
	[rep setSize:NSMakeSize(16, 16)];
	[rep release];
}

void drawit(CGContextRef c)
{
	CGImageRef cgi;

	CGContextSaveGState(c);
	CGContextSetInterpolationQuality(c, kCGInterpolationNone);

	CGContextDrawImage(c,
		CGRectMake(0, 16, 16, 16),
		i16);

	CGContextDrawImage(c,
		CGRectMake(16 + 5, 0, 32, 32),
		i32);

	[image drawInRect:NSMakeRect(16 + 5 + 32 + 5, 16, 16, 16)];

	cgi = [image CGImageForProposedRect:NULL
		context:[NSGraphicsContext currentContext]
		hints:nil];
	CGContextDrawImage(c,
		CGRectMake(16 + 5 + 32 + 5 + 16 + 5, 16, 16, 16),
		cgi);
	CGImageRelease(cgi);

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
	drawit(c);
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
		20 + 16 + 5 + 32 + 5 + 16 + 5 + 16 + 20,
		20 + 32 + 20)
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
