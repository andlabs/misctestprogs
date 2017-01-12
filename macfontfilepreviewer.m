// 11 january 2017
#import <Cocoa/Cocoa.h>

NSError *walk(NSURL *dirURL, NSError *(^f)(NSURL *url))
{
	BOOL (^errorHandler)(NSURL *url, NSError *error);
	NSDirectoryEnumerator<NSURL *> *walk;
	NSURL *file;
	__block NSError *ret;

	ret = nil;

	errorHandler = ^(NSURL *url, NSError *err) {
		// TODO is the retain correct?
		ret = [err retain];
		return NO;
	};
	walk = [[NSFileManager defaultManager] enumeratorAtURL:dirURL
		includingPropertiesForKeys:@[NSURLNameKey, NSURLIsRegularFileKey, NSURLIsReadableKey]
		options:0
		errorHandler:errorHandler];

	for (file = [walk nextObject]; file != nil; file = [walk nextObject]) {
		NSNumber *num;
		NSError *err;

		if ([file getResourceValue:&num forKey:NSURLIsRegularFileKey error:&err] == NO) {
			// TODO retain err?
			ret = err;
			break;
		}
		if ([num boolValue] == NO)
			continue;
		// TODO release num?
		if ([file getResourceValue:&num forKey:NSURLIsReadableKey error:&err] == NO) {
			// TODO retain err?
			ret = err;
			break;
		}
		if ([num boolValue] == NO)
			continue;
		// TODO release num?
		err = f(file);
		if (err != nil) {
			ret = err;
			break;
		}
	}

	// TODO release walk?
	return ret;
}

NSError *loadFonts(NSURL *dirURL, NSArray **fonts)
{
	__block NSMutableArray *out;
	NSError *err;

	*fonts = nil;
	out = [NSMutableArray new];
	err = walk(dirURL, ^(NSURL *file) {
		CGDataProviderRef dp;
		CGFontRef cgfont;

		dp = CGDataProviderCreateWithURL((CFURLRef) file);
		cgfont = CGFontCreateWithDataProvider(dp);
		if (cgfont != NULL)
			[out addObject:[NSValue valueWithPointer:cgfont]];
		// otherwise the font file is invalid
		CFRelease(dp);
		return (NSError *) nil;		// make compiler happy
	});
	if (err != nil)
		return err;
	*fonts = out;
	return nil;
}

NSApplication *app;

struct ctasset {
	CTFontRef font;
	NSAttributedString *fontName;
	NSRect expectedFontNameRect;
	CTFramesetterRef fs;
	CGSize suggestedSize;
	CTFrameRef frame;
	CGRect expectedFrameRect;
};

@interface previewView : NSView<NSTextFieldDelegate> {
	NSArray *fonts;
	NSString *curString;
	CGFloat size;
	struct ctasset *curAssets;
}
- (IBAction)sizeSliderChanged:(id)sender;
@end

@implementation previewView

- (id)initWithFonts:(NSArray *)fts
{
	self = [super initWithFrame:NSZeroRect];
	if (self) {
		self->fonts = fts;
		self->curString = [@"" copy];
		self->size = 12;
		self->curAssets = NULL;
	}
	return self;
}

- (void)dealloc
{
	NSValue *v;
	CGFontRef cgfont;

	[self freeCurAssets];
	[self->curString release];
	for (v in self->fonts) {
		cgfont = (CGFontRef) [v pointerValue];
		CFRelease(cgfont);
	}
	[self->fonts release];
	[super dealloc];
}

- (BOOL)isFlipped
{
	return YES;
}

- (void)mkCTAsset:(struct ctasset *)c for:(CGFontRef)cgfont width:(CGFloat)width
{
	NSString *fontName;
	NSFont *sysfont;
	CFMutableDictionaryRef dict;
	CFAttributedStringRef cas;
	CFRange range, fitRange;
	CGRect rect;
	CGPathRef path;

	c->font = CTFontCreateWithGraphicsFont(cgfont, self->size, NULL, NULL);
	fontName = [((NSFont *) (c->font)) displayName];
	sysfont = [NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]];
	c->fontName = [[NSAttributedString alloc]
		initWithString:fontName
		attributes:@{
			NSFontAttributeName:		sysfont,
		}];
	dict = CFDictionaryCreateMutable(NULL, 0,
		&kCFCopyStringDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	CFDictionaryAddValue(dict, kCTFontAttributeName, c->font);
	cas = CFAttributedStringCreate(NULL, (CFStringRef) (self->curString), dict);
	CFRelease(dict);
	c->fs = CTFramesetterCreateWithAttributedString(cas);
	range.location = 0;
	range.length = CFAttributedStringGetLength(cas);
	CFRelease(cas);
	c->suggestedSize = CTFramesetterSuggestFrameSizeWithConstraints(c->fs, range,
		NULL,
		CGSizeMake(width, CGFLOAT_MAX),
		&fitRange);
	rect.origin = CGPointZero;
	rect.size = c->suggestedSize;
	path = CGPathCreateWithRect(rect, NULL);
	c->frame = CTFramesetterCreateFrame(c->fs,
		range,
		path,
		NULL);
	CFRelease(path);
}

- (void)freeCTAsset:(struct ctasset *)c
{
	CFRelease(c->frame);
	CFRelease(c->fs);
	[c->fontName release];
	CFRelease(c->font);
}

- (void)freeCurAssets
{
	struct ctasset *c;
	CFIndex i, n;

	if (self->curAssets == NULL)
		return;
	n = [self->fonts count];
	c = self->curAssets;
	for (i = 0; i < n; i++) {
		[self freeCTAsset:c];
		c++;
	}
	free(self->curAssets);
	self->curAssets = NULL;
}

#define XMARGIN 5
#define YMARGIN 5
#define NAMEGAP 5
#define ITEMGAP 10

- (CGFloat)layoutLines
{
	struct ctasset *c;
	CGFloat height;
	CFIndex i, n;

	height = YMARGIN;
	n = [self->fonts count];
	c = self->curAssets;
	for (i = 0; i < n; i++) {
		c->expectedFontNameRect.origin.x = XMARGIN;
		c->expectedFontNameRect.origin.y = height;
		c->expectedFontNameRect.size = [c->fontName size];
		height += c->expectedFontNameRect.size.height;
		height += NAMEGAP;
		c->expectedFrameRect.origin.x = XMARGIN;
		c->expectedFrameRect.origin.y = height;
		c->expectedFrameRect.size = c->suggestedSize;
		height += c->expectedFrameRect.size.height;
		if (i != (n - 1))
			height += ITEMGAP;
		c++;
	}
	height += YMARGIN;
	return height;
}

- (void)recomputeEverything:(CGFloat)width
{
	struct ctasset *c;
	CFIndex i, n;
	CGFloat textwid;
	CGFloat height;

	[self freeCurAssets];
	n = [self->fonts count];
	self->curAssets = (struct ctasset *) malloc(n * sizeof (struct ctasset));
	c = self->curAssets;
	textwid = width - (2 * XMARGIN);
	for (i = 0; i < n; i++) {
		NSValue *v;
		CGFontRef cgfont;

		v = (NSValue *) [self->fonts objectAtIndex:i];
		cgfont = (CGFontRef) [v pointerValue];
		[self mkCTAsset:c for:cgfont width:textwid];
		c++;
	}
	height = [self layoutLines];
	[self setFrameSize:NSMakeSize(width, height)];
	[self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)r
{
	struct ctasset *c;
	CFIndex i, n;
	CGContextRef cc;

	cc = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];
	n = [self->fonts count];
	c = self->curAssets;
	for (i = 0; i < n; i++) {
		[c->fontName drawAtPoint:c->expectedFontNameRect.origin];
		CGContextSaveGState(cc);
		CGContextTranslateCTM(cc, 0, [self frame].size.height);
		CGContextScaleCTM(cc, 1.0, -1.0);
		CGContextTranslateCTM(cc, c->expectedFrameRect.origin.x, [self frame].size.height - c->expectedFrameRect.origin.y - c->expectedFrameRect.size.height);
		CGContextSetTextMatrix(cc, CGAffineTransformIdentity);
		CTFrameDraw(c->frame, cc);
		CGContextRestoreGState(cc);
		c++;
	}
}

- (void)resizeWithOldSuperviewSize:(NSSize)oldSize
{
	[super resizeWithOldSuperviewSize:oldSize];
	[self recomputeEverything:[self frame].size.width];
}

- (void)controlTextDidChange:(NSNotification *)note
{
	NSTextField *textField;

	textField = (NSTextField *) [note object];
	[self->curString release];
	self->curString = [[textField stringValue] copy];
	[self recomputeEverything:[self frame].size.width];
}

- (IBAction)sizeSliderChanged:(id)sender
{
	NSSlider *slider = (NSSlider *) sender;

	self->size = [slider doubleValue];
	[self recomputeEverything:[self frame].size.width];
}

@end

@interface appDelegate : NSObject<NSApplicationDelegate>
- (void)openWindow:(NSArray *)fonts;
@end

@implementation appDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
	NSOpenPanel *open;
	NSArray *fonts;
	NSError *err;

	open = [NSOpenPanel openPanel];
	[open setCanChooseFiles:NO];
	[open setCanChooseDirectories:YES];
	[open setResolvesAliases:YES];
	[open setAllowsMultipleSelection:NO];
	[open setTitle:@"Select Folder"];
	[open setCanCreateDirectories:NO];
	[open setShowsHiddenFiles:YES];
	if ([open runModal] != NSFileHandlingPanelOKButton)
		[app terminate:self];
	// TODO release open? merely hide open? do neither?

	err = loadFonts((NSURL *) [[open URLs] objectAtIndex:0], &fonts);
	if (err != nil) {
		[[NSAlert alertWithError:err] runModal];
		[app terminate:self];
	}

	[self openWindow:fonts];
}

- (void)openWindow:(NSArray *)fonts
{
	NSWindow *mainwin;
	__block NSView *contentView;
	NSTextField *textField;
	NSSlider *sizeSlider;
	NSScrollView *sv;
	previewView *pv;
	__block NSDictionary *views;
	void (^addConstraints)(NSString *s);

	mainwin = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 640, 480)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	contentView = [mainwin contentView];

	pv = [[previewView alloc] initWithFonts:fonts];
	[pv setAutoresizingMask:(NSViewWidthSizable | NSViewMinYMargin)];

	sv = [[NSScrollView alloc] initWithFrame:NSZeroRect];
	[sv setBackgroundColor:[NSColor colorWithCalibratedWhite:1.0 alpha:1.0]];
	[sv setDrawsBackground:YES];
	[sv setBorderType:NSBezelBorder];
	[sv setAutohidesScrollers:YES];
	[sv setHasHorizontalRuler:NO];
	[sv setHasVerticalRuler:NO];
	[sv setRulersVisible:NO];
	[sv setScrollerKnobStyle:NSScrollerKnobStyleDefault];
	[sv setScrollsDynamically:YES];
	[sv setFindBarPosition:NSScrollViewFindBarPositionAboveContent];
	[sv setUsesPredominantAxisScrolling:NO];
	[sv setHorizontalScrollElasticity:NSScrollElasticityAutomatic];
	[sv setVerticalScrollElasticity:NSScrollElasticityAutomatic];
	[sv setAllowsMagnification:NO];
	[sv setHasHorizontalScroller:NO];
	[sv setHasVerticalScroller:YES];
	[sv setDocumentView:pv];
	[sv setTranslatesAutoresizingMaskIntoConstraints:NO];
	[contentView addSubview:sv];

	textField = [[NSTextField alloc] initWithFrame:NSZeroRect];
	[textField setSelectable:YES];
	[textField setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
	[textField setBordered:NO];
	[textField setBezelStyle:NSTextFieldSquareBezel];
	[textField setBezeled:YES];
	[[textField cell] setLineBreakMode:NSLineBreakByClipping];
	[[textField cell] setScrollable:YES];
	[textField setDelegate:pv];
	[textField setTranslatesAutoresizingMaskIntoConstraints:NO];
	[contentView addSubview:textField];

	sizeSlider = [[NSSlider alloc] initWithFrame:NSMakeRect(0, 0, 92, 2)];
	[sizeSlider setMinValue:8];
	[sizeSlider setMaxValue:288];
	[sizeSlider setDoubleValue:12];
	[sizeSlider setTarget:pv];
	[sizeSlider setAction:@selector(sizeSliderChanged:)];
	[sizeSlider setTranslatesAutoresizingMaskIntoConstraints:NO];
	[contentView addSubview:sizeSlider];

	views = NSDictionaryOfVariableBindings(textField, sizeSlider, sv);
	addConstraints = ^(NSString *s) {
		NSArray<NSLayoutConstraint *> *a;

		a = [NSLayoutConstraint constraintsWithVisualFormat:s
			options:0
			metrics:nil
			views:views];
		[contentView addConstraints:a];
	};
	addConstraints(@"H:|-[textField]-[sizeSlider(==92)]-|");
	addConstraints(@"H:|-[sv]-|");
	addConstraints(@"V:|-[textField]-[sv]-|");
	addConstraints(@"V:|-[sizeSlider]-[sv]-|");

	[mainwin center];
	[mainwin makeKeyAndOrderFront:self];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
	return YES;
}

@end

int main(void)
{
	app = [NSApplication sharedApplication];
	[app setActivationPolicy:NSApplicationActivationPolicyRegular];
	[app setDelegate:[appDelegate new]];
	[app run];
	return 0;
}
