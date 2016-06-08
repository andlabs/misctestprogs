// 7 june 2016
#import <Cocoa/Cocoa.h>

NSLayoutConstraint *mkConstraint(id view1, NSLayoutAttribute attr1, NSLayoutRelation relation, id view2, NSLayoutAttribute attr2, CGFloat multiplier, CGFloat c, NSString *desc);
NSTextField *newLabel(NSString *str);

@interface formChild : NSView
@property (strong) NSView *view;
@property (strong) NSTextField *label;
@property BOOL stretchy;
@property (strong) NSLayoutConstraint *baseline;
@property (strong) NSLayoutConstraint *leading;
@property (strong) NSLayoutConstraint *trailing;
@property (strong) NSLayoutConstraint *top;
@property (strong) NSLayoutConstraint *bottom;
- (id)initWithLabel:(NSTextField *)l;
- (void)onDestroy;
@end

@interface formView : NSView {
	NSMutableArray *children;
	BOOL padded;
	uintmax_t nStretchy;

	NSLayoutConstraint *first;
	NSMutableArray *inBetweens;
	NSLayoutConstraint *last;
	NSMutableArray *widths;
	NSMutableArray *leadings;
	NSMutableArray *middles;
	NSMutableArray *trailings;
}
- (id)init;
- (void)onDestroy;
- (void)removeOurConstraints;
- (CGFloat)paddingAmount;
- (void)establishOurConstraints;
- (void)append:(NSString *)label c:(NSView *)c stretchy:(BOOL)stretchy;
- (void)setPadded:(BOOL)p;
@end

@implementation formChild

- (id)initWithLabel:(NSTextField *)l
{
	self = [super initWithFrame:NSZeroRect];
	if (self) {
		self.label = l;
		[self.label setTranslatesAutoresizingMaskIntoConstraints:NO];
		[self.label setContentHuggingPriority:NSLayoutPriorityRequired forOrientation:NSLayoutConstraintOrientationHorizontal];
		[self.label setContentHuggingPriority:NSLayoutPriorityRequired forOrientation:NSLayoutConstraintOrientationVertical];
		[self addSubview:self.label];

		self.leading = mkConstraint(self.label, NSLayoutAttributeLeading,
			NSLayoutRelationGreaterThanOrEqual,
			self, NSLayoutAttributeLeading,
			1, 0,
			@"uiForm label leading");
		[self addConstraint:self.leading];
		self.trailing = mkConstraint(self.label, NSLayoutAttributeTrailing,
			NSLayoutRelationEqual,
			self, NSLayoutAttributeTrailing,
			1, 0,
			@"uiForm label trailing");
		[self addConstraint:self.trailing];
		self.top = mkConstraint(self.label, NSLayoutAttributeTop,
			NSLayoutRelationEqual,
			self, NSLayoutAttributeTop,
			1, 0,
			@"uiForm label top");
		[self addConstraint:self.top];
		self.bottom = mkConstraint(self.label, NSLayoutAttributeBottom,
			NSLayoutRelationEqual,
			self, NSLayoutAttributeBottom,
			1, 0,
			@"uiForm label bottom");
		[self addConstraint:self.bottom];
	}
	return self;
}

- (void)onDestroy
{
	[self removeConstraint:self.trailing];
	self.trailing = nil;
	[self removeConstraint:self.top];
	self.top = nil;
	[self removeConstraint:self.bottom];
	self.bottom = nil;

	[self.label removeFromSuperview];
	self.label = nil;
}

@end

@implementation formView

- (id)init
{
	self = [super initWithFrame:NSZeroRect];
	if (self != nil) {
		self->padded = NO;
		self->children = [NSMutableArray new];
		self->nStretchy = 0;

		self->inBetweens = [NSMutableArray new];
		self->widths = [NSMutableArray new];
		self->leadings = [NSMutableArray new];
		self->middles = [NSMutableArray new];
		self->trailings = [NSMutableArray new];
	}
	return self;
}

- (void)onDestroy
{
	formChild *fc;

	[self removeOurConstraints];
	[self->inBetweens release];
	[self->widths release];
	[self->leadings release];
	[self->middles release];
	[self->trailings release];

	for (fc in self->children) {
		[self removeConstraint:fc.baseline];
		fc.baseline = nil;
		[fc.view removeFromSuperview];
		fc.view = nil;
		[fc onDestroy];
		[fc removeFromSuperview];
	}
	[self->children release];
}

- (void)removeOurConstraints
{
	if (self->first != nil) {
		[self removeConstraint:self->first];
		[self->first release];
		self->first = nil;
	}
	if ([self->inBetweens count] != 0) {
		[self removeConstraints:self->inBetweens];
		[self->inBetweens removeAllObjects];
	}
	if (self->last != nil) {
		[self removeConstraint:self->last];
		[self->last release];
		self->last = nil;
	}
	if ([self->widths count] != 0) {
		[self removeConstraints:self->widths];
		[self->widths removeAllObjects];
	}
	if ([self->leadings count] != 0) {
		[self removeConstraints:self->leadings];
		[self->leadings removeAllObjects];
	}
	if ([self->middles count] != 0) {
		[self removeConstraints:self->middles];
		[self->middles removeAllObjects];
	}
	if ([self->trailings count] != 0) {
		[self removeConstraints:self->trailings];
		[self->trailings removeAllObjects];
	}
}

- (CGFloat)paddingAmount
{
	if (!self->padded)
		return 0.0;
	return 8.0;
}

- (void)establishOurConstraints
{
	formChild *fc;
	CGFloat padding;
	NSView *prev, *prevlabel;
	NSLayoutConstraint *c;
	NSLayoutRelation relation;

	[self removeOurConstraints];
	if ([self->children count] == 0)
		return;
	padding = [self paddingAmount];

	// first arrange the children vertically and make them the same width
	prev = nil;
	for (fc in self->children) {
		if (prev == nil) {			// first view
			self->first = mkConstraint(self, NSLayoutAttributeTop,
				NSLayoutRelationEqual,
				fc.view, NSLayoutAttributeTop,
				1, 0,
				@"uiForm first vertical constraint");
			[self addConstraint:self->first];
			[self->first retain];
			prev = fc.view;
			prevlabel = fc;
			continue;
		}
		// not the first; link it
		c = mkConstraint(prev, NSLayoutAttributeBottom,
			NSLayoutRelationEqual,
			fc.view, NSLayoutAttributeTop,
			1, -padding,
			@"uiForm in-between vertical constraint");
		[self addConstraint:c];
		[self->inBetweens addObject:c];
		// and make the same width
		c = mkConstraint(prev, NSLayoutAttributeWidth,
			NSLayoutRelationEqual,
			fc.view, NSLayoutAttributeWidth,
			1, 0,
			@"uiForm width constraint");
		[self addConstraint:c];
		[self->widths addObject:c];
		c = mkConstraint(prevlabel, NSLayoutAttributeWidth,
			NSLayoutRelationEqual,
			fc, NSLayoutAttributeWidth,
			1, 0,
			@"uiForm label width constraint");
		[self addConstraint:c];
		[self->widths addObject:c];
		prev = fc.view;
		prevlabel = fc;
	}
	relation = NSLayoutRelationEqual;
	if (self->nStretchy != 0)
		relation = NSLayoutRelationLessThanOrEqual;
	self->last = mkConstraint(prev, NSLayoutAttributeBottom,
		NSLayoutRelationEqual,
		self, NSLayoutAttributeBottom,
		1, 0,
		@"uiForm last vertical constraint");
	[self addConstraint:self->last];
	[self->last retain];

	// now arrange the controls horizontally
	for (fc in self->children) {
		c = mkConstraint(self, NSLayoutAttributeLeading,
			NSLayoutRelationEqual,
			fc, NSLayoutAttributeLeading,
			1, 0,
			@"uiForm leading constraint");
		[self addConstraint:c];
		[self->leadings addObject:c];
		c = mkConstraint(fc, NSLayoutAttributeTrailing,
			NSLayoutRelationEqual,
			fc.view, NSLayoutAttributeLeading,
			1, -padding,
			@"uiForm middle constraint");
		[self addConstraint:c];
		[self->middles addObject:c];
		c = mkConstraint(fc.view, NSLayoutAttributeTrailing,
			NSLayoutRelationEqual,
			self, NSLayoutAttributeTrailing,
			1, 0,
			@"uiForm trailing constraint");
		[self addConstraint:c];
		[self->trailings addObject:c];
	}

	// we don't arrange the labels vertically; that's done when we add the control since those constraints don't need to change (they just need to be at their baseline)
}

- (void)append:(NSString *)label c:(NSView *)c stretchy:(BOOL)stretchy
{
	formChild *fc;
	NSLayoutPriority priority;
	NSLayoutAttribute attribute;
	uintmax_t oldnStretchy;

	fc = [[formChild alloc] initWithLabel:newLabel(label)];
	fc.view = c;
	fc.stretchy = stretchy;
	[fc setTranslatesAutoresizingMaskIntoConstraints:NO];
	[self addSubview:fc];

	[self addSubview:fc.view];

	// if a control is stretchy, it should not hug vertically
	// otherwise, it should *forcibly* hug
	if (fc.stretchy)
		priority = NSLayoutPriorityDefaultLow;
	else
		// LONGTERM will default high work?
		priority = NSLayoutPriorityRequired;
	[fc.view setContentHuggingPriority:priority forOrientation:NSLayoutConstraintOrientationVertical];
	// make sure controls don't hug their horizontal direction so they fill the width of the view
	[fc.view setContentHuggingPriority:NSLayoutPriorityDefaultLow forOrientation:NSLayoutConstraintOrientationHorizontal];

	// and constrain the baselines to position the label vertically
	// if the view is a scroll view, align tops, not baselines
	// this is what Interface Builder does
	attribute = NSLayoutAttributeBaseline;
	if ([fc.view isKindOfClass:[NSScrollView class]])
		attribute = NSLayoutAttributeTop;
	fc.baseline = mkConstraint(fc.label, attribute,
		NSLayoutRelationEqual,
		fc.view, attribute,
		1, 0,
		@"uiForm baseline constraint");
	[self addConstraint:fc.baseline];

	[self->children addObject:fc];

	[self establishOurConstraints];
	if (fc.stretchy) {
		oldnStretchy = self->nStretchy;
		self->nStretchy++;
		if (oldnStretchy == 0)
			[self establishOurConstraints];
	}

	[fc release];		// we don't need the initial reference now
}

- (void)setPadded:(BOOL)p
{
	CGFloat padding;
	NSLayoutConstraint *c;

	self->padded = p;
	padding = [self paddingAmount];
	for (c in self->inBetweens)
		[c setConstant:-padding];
	for (c in self->middles)
		[c setConstant:-padding];
}

@end

// demo

NSTextField *newPasswordField(void);
NSTextField *newSearchField(void);
NSButton *newCheckbox(NSString *label);

@interface appDelegate : NSObject<NSApplicationDelegate>
@property (strong) NSWindow *w;
@property (strong) formView *form;
@end

@implementation appDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
	NSView *contentView;
	formView *form;
	NSButton *cb;

	self.w = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, 200, 200)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	contentView = [self.w contentView];

	self.form = [formView new];
	[self.form setTranslatesAutoresizingMaskIntoConstraints:NO];
	[contentView addSubview:self.form];
	[contentView addConstraint:mkConstraint(contentView, NSLayoutAttributeLeading,
		NSLayoutRelationEqual,
		self.form, NSLayoutAttributeLeading,
		1, -20,
		@"content view leading")];
	[contentView addConstraint:mkConstraint(contentView, NSLayoutAttributeTop,
		NSLayoutRelationEqual,
		self.form, NSLayoutAttributeTop,
		1, -20,
		@"content view top")];
	[contentView addConstraint:mkConstraint(contentView, NSLayoutAttributeTrailing,
		NSLayoutRelationEqual,
		self.form, NSLayoutAttributeTrailing,
		1, 20,
		@"content view trailing")];
	[contentView addConstraint:mkConstraint(contentView, NSLayoutAttributeBottom,
		NSLayoutRelationEqual,
		self.form, NSLayoutAttributeBottom,
		1, 20,
		@"content view bottom")];

	[self.form append:@"Password Field"
		c:newPasswordField()
		stretchy:NO];
	[self.form append:@"Search Box"
		c:newSearchField()
		stretchy:NO];
	cb = newCheckbox(@"Padded");
	[self.form append:@""
		c:cb
		stretchy:NO];
	[cb setTarget:self];
	[cb setAction:@selector(onToggled:)];

	[self.w visualizeConstraints:[self.form constraints]];
	[self.w makeKeyAndOrderFront:nil];
}

- (IBAction)onToggled:(id)sender
{
	[self.form setPadded:([sender state] == NSOnState)];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
	return YES;
}

@end

int main(void)
{
	NSApplication *a;

	a = [NSApplication sharedApplication];
	[a setActivationPolicy:NSApplicationActivationPolicyRegular];
	[a setDelegate:[appDelegate new]];
	[a run];
	return 0;
}

// boilerplate

NSLayoutConstraint *mkConstraint(id view1, NSLayoutAttribute attr1, NSLayoutRelation relation, id view2, NSLayoutAttribute attr2, CGFloat multiplier, CGFloat c, NSString *desc)
{
	NSLayoutConstraint *constraint;

	constraint = [NSLayoutConstraint constraintWithItem:view1
		attribute:attr1
		relatedBy:relation
		toItem:view2
		attribute:attr2
		multiplier:multiplier
		constant:c];
	// apparently only added in 10.9
	if ([constraint respondsToSelector:@selector(setIdentifier:)])
		[((id) constraint) setIdentifier:desc];
	return constraint;
}

NSTextField *finishNewTextField(NSTextField *t, BOOL isEntry)
{
	[t setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];

	// THE ORDER OF THESE CALLS IS IMPORTANT; CHANGE IT AND THE BORDERS WILL DISAPPEAR
	[t setBordered:NO];
	[t setBezelStyle:NSTextFieldSquareBezel];
	[t setBezeled:isEntry];

	[[t cell] setLineBreakMode:NSLineBreakByClipping];
	[[t cell] setScrollable:YES];

	[t setTranslatesAutoresizingMaskIntoConstraints:NO];

	return t;
}

NSTextField *newPasswordField(void)
{
	return finishNewTextField([[NSSecureTextField alloc] initWithFrame:NSZeroRect], YES);
}

NSTextField *newSearchField(void)
{
	NSSearchField *s;

	s = (NSSearchField *) finishNewTextField([[NSSearchField alloc] initWithFrame:NSZeroRect], YES);
	[s setSendsSearchStringImmediately:NO];
	[s setSendsWholeSearchString:NO];
	[s setBordered:NO];
	[s setBezelStyle:NSTextFieldRoundedBezel];
	[s setBezeled:YES];
	return s;
}

NSTextField *newLabel(NSString *str)
{
	NSTextField *tf;

	tf = [[NSTextField alloc] initWithFrame:NSZeroRect];
	[tf setStringValue:str];
	[tf setEditable:NO];
	[tf setSelectable:NO];
	[tf setDrawsBackground:NO];
	return finishNewTextField(tf, NO);
}

NSButton *newCheckbox(NSString *label)
{
	NSButton *c;

	c = [[NSButton alloc] initWithFrame:NSZeroRect];
	[c setTitle:label];
	[c setButtonType:NSSwitchButton];
	// doesn't seem to have an associated bezel style
	[c setBordered:NO];
	[c setTransparent:NO];
	[c setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
	[c setTranslatesAutoresizingMaskIntoConstraints:NO];
	return c;
}
