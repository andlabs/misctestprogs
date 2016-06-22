// 22 june 2016
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

@interface tableModel : NSObject<NSTableViewDataSource, NSTableViewDelegate>
@end

@implementation tableModel

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tv
{
	return 15;
}

 - (NSView *)tableView:(NSTableView *)table viewForTableColumn:(NSTableColumn *)c row:(NSInteger)row
{
	NSTableCellView *tv;
	NSTextField *tf;

	tv = [[NSTableCellView alloc] initWithFrame:NSZeroRect];

	tf = [[NSTextField alloc] initWithFrame:NSZeroRect];
	[tf setStringValue:[NSString stringWithFormat:@"Row %d", ((int) (row + 1))]];
	[tf setEditable:NO];
	[tf setSelectable:NO];
	[tf setDrawsBackground:NO];
	[tf setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
	[tf setBordered:NO];
	[tf setBezelStyle:NSTextFieldSquareBezel];
	[tf setBezeled:NO];
	[[tf cell] setLineBreakMode:NSLineBreakByClipping];
	[[tf cell] setScrollable:YES];

	[tf setTranslatesAutoresizingMaskIntoConstraints:NO];
	[tv addSubview:tf];
	[tv addConstraint:mkConstraint(tv, NSLayoutAttributeLeading,
		NSLayoutRelationEqual,
		tf, NSLayoutAttributeLeading,
		1, -2)];
	[tv addConstraint:mkConstraint(tv, NSLayoutAttributeTrailing,
		NSLayoutRelationEqual,
		tf, NSLayoutAttributeTrailing,
		1, 3)];
	[tv addConstraint:mkConstraint(tv, NSLayoutAttributeCenterY,
		NSLayoutRelationEqual,
		tf, NSLayoutAttributeCenterY,
		1, 0)];

	[tv setTranslatesAutoresizingMaskIntoConstraints:NO];
	return tv;
}

@end

NSTableColumn *mkColumn(NSString *title)
{
	NSTableColumn *c;

	c = [[NSTableColumn alloc] initWithIdentifier:@""];
	// via Interface Builder
	[c setResizingMask:(NSTableColumnAutoresizingMask | NSTableColumnUserResizingMask)];
	[c setTitle:title];
	[[c headerCell] setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSSmallControlSize]]];
	return c;
}

NSTableView *mkTableView(void)
{
	NSTableView *tv;
	tableModel *model;

	tv = [[NSTableView alloc] initWithFrame:NSZeroRect];

	model = [tableModel new];
	[tv setDataSource:model];
	[tv setDelegate:model];
	[tv reloadData];

	[tv setAllowsColumnReordering:NO];
	[tv setAllowsColumnResizing:YES];
	[tv setAllowsMultipleSelection:NO];
	[tv setAllowsEmptySelection:YES];
	[tv setAllowsColumnSelection:NO];
	[tv setUsesAlternatingRowBackgroundColors:YES];
	[tv setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleRegular];
	[tv setGridStyleMask:NSTableViewGridNone];
	[tv setAllowsTypeSelect:YES];

	// needed to allow the table to grow vertically beyond showing all rows
	[tv setContentHuggingPriority:NSLayoutPriorityDefaultLow forOrientation:NSLayoutConstraintOrientationVertical];

	return tv;
}

NSScrollView *mkScrollView(NSView *dv)
{
	NSScrollView *sv;
	NSView *cv;

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

	[dv setTranslatesAutoresizingMaskIntoConstraints:NO];
	[sv setDocumentView:dv];

	cv = [sv contentView];
	[sv addConstraint:mkConstraint(dv, NSLayoutAttributeLeading,
		NSLayoutRelationEqual,
		cv, NSLayoutAttributeLeading,
		1, 0)];
	[sv addConstraint:mkConstraint(dv, NSLayoutAttributeTop,
		NSLayoutRelationEqual,
		cv, NSLayoutAttributeTop,
		1, 0)];
	[sv addConstraint:mkConstraint(dv, NSLayoutAttributeTrailing,
		NSLayoutRelationGreaterThanOrEqual,
		cv, NSLayoutAttributeTrailing,
		1, 0)];
	[sv addConstraint:mkConstraint(dv, NSLayoutAttributeBottom,
		NSLayoutRelationGreaterThanOrEqual,
		cv, NSLayoutAttributeBottom,
		1, 0)];

	return sv;
}

@interface appDelegate : NSObject<NSApplicationDelegate>
@end

@implementation appDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
	NSWindow *w;
	NSView *contentView;
	NSTableView *tv;
	NSScrollView *sv;

	w = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 320, 240)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	contentView = [w contentView];

	tv = mkTableView();
	[tv addTableColumn:mkColumn(@"Column")];
	sv = mkScrollView(tv);
	[sv setTranslatesAutoresizingMaskIntoConstraints:NO];
	[contentView addSubview:sv];

	[contentView addConstraint:mkConstraint(contentView, NSLayoutAttributeLeading,
		NSLayoutRelationEqual,
		sv, NSLayoutAttributeLeading,
		1, -20)];
	[contentView addConstraint:mkConstraint(contentView, NSLayoutAttributeTop,
		NSLayoutRelationEqual,
		sv, NSLayoutAttributeTop,
		1, -20)];
	[contentView addConstraint:mkConstraint(contentView, NSLayoutAttributeTrailing,
		NSLayoutRelationEqual,
		sv, NSLayoutAttributeTrailing,
		1, 20)];
	[contentView addConstraint:mkConstraint(contentView, NSLayoutAttributeBottom,
		NSLayoutRelationEqual,
		sv, NSLayoutAttributeBottom,
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
