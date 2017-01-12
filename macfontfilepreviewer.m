// 11 january 2017
#import <Cocoa/Cocoa.h>

NSError *walk(NSURL *dirURL, NSError *(^f)(NSURL *url))
{
	NSFileManager *fm;
	BOOL (^errorHandler)(NSURL *url, NSError *error);
	NSDirectoryEnumerator<NSURL *> *walk;
	NSURL *file;
	__block NSError *ret;

	ret = nil;

	fm = [NSFileManager new];
	errorHandler = ^(NSURL *url, NSError *err) {
		// TODO is the retain correct?
		ret = [err retain];
		return NO;
	};
	walk = [fm enumeratorAtURL:dirURL
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

	[walk release];
	[fm release];
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

@interface previewView : NSView<NSTextFieldDelegate> {
	NSArray *fonts;
	NSString *curString;
	CGFloat size;
}
- (IBAction)sizeSliderChanged:(id)sender;
@end

@implementation previewView

- (id)initWithFonts:(NSArray *)fonts
{
	self = [super initWithFrame:NSZeroRect];
	if (self) {
		self->fonts = fonts;
		self->curString = [@"" copy];
		self->size = 12;
	}
	return self;
}

- (void)dealloc
{
	NSValue *v;
	CGFontRef cgfont;

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

- (void)drawRect:(NSRect)r
{
}

- (void)resizeWithOldSuperviewSize:(NSSize)oldSize
{
	[super resizeWithOldSuperviewSize:oldSize];
	[self recomputeFrameSize:[self frame].size.width];
}

- (void)controlTextDidChange:(NSNotification *)note
{
	NSTextField *textField;

	textField = (NSTextField *) [note object];
	// TODO
}

- (IBAction)sizeSliderChanged:(id)sender
{
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
	NSView *contentView;
	NSTextField *textField;
	NSSlider *sizeSlider;
	NSScrollView *sv;
	previewView *pv;

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

	sizeSlider = [[NSlider alloc] initWithFrame:NSMakeRect(0, 0, 92, 2)];
	[sizeSlider setMinValue:8];
	[sizeSlider setMaxValue:288];
	[sizeSlider setDoubeValue:12];
	[sizeSlider setTarget:previewView];
	[sizeSlider setAction:@selector(sizeSliderChanged:)];
	[sizeSlider setTranslatesAutoresizingMaskIntoConstraints:NO];
	[contentView addSubview:sizeSlider];

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
