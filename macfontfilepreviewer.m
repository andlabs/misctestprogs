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

@interface appDelegate : NSObject<NSApplicationDelegate>
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

	NSLog(@"%ld", [fonts count]);
	[app terminate:self];
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
