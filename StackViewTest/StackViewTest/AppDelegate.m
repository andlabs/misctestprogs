// 20 december 2016
#import "AppDelegate.h"

@interface AppDelegate ()
@property (weak) IBOutlet NSWindow *window;
@property (weak) IBOutlet NSButton *b;
@property (weak) IBOutlet NSTextField *tf;
@property (weak) IBOutlet NSStackView *stackView;
@property (weak) IBOutlet NSImageView *image;
@property (weak) IBOutlet NSView *stretchyView;
@property (weak) IBOutlet NSSlider *slider;
@property (weak) IBOutlet NSButton *horizontal;
@property (weak) IBOutlet NSButton *spaced;
@property (weak) IBOutlet NSButton *stretchyImage;
@property (weak) IBOutlet NSButton *stretchySlider;

@property (strong) NSMutableArray *equalWidthConstraints;
@property (strong) NSMutableArray *equalHeightConstraints;
@property (strong) NSLayoutConstraint *bothStretchyWidthConstraint;
@property (strong) NSLayoutConstraint *bothStretchyHeightConstraint;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	NSView *v;
	
	self.equalWidthConstraints = [NSMutableArray new];
	self.equalHeightConstraints = [NSMutableArray new];
	for (v in [self.stackView views]) {
		[self.equalWidthConstraints addObject:[NSLayoutConstraint constraintWithItem:v attribute:NSLayoutAttributeWidth
			relatedBy:NSLayoutRelationEqual
			toItem:self.stackView attribute:NSLayoutAttributeWidth
			multiplier:1 constant:0]];
		[self.equalHeightConstraints addObject:[NSLayoutConstraint constraintWithItem:v attribute:NSLayoutAttributeHeight
			relatedBy:NSLayoutRelationEqual
			toItem:self.stackView attribute:NSLayoutAttributeHeight
			multiplier:1 constant:0]];
	}
	[NSLayoutConstraint deactivateConstraints:self.equalHeightConstraints];
	[NSLayoutConstraint activateConstraints:self.equalWidthConstraints];
	
	self.bothStretchyWidthConstraint = [NSLayoutConstraint constraintWithItem:self.image attribute:NSLayoutAttributeWidth
		relatedBy:NSLayoutRelationEqual
		toItem:self.slider attribute:NSLayoutAttributeWidth
		multiplier:1 constant:0];
	self.bothStretchyHeightConstraint = [NSLayoutConstraint constraintWithItem:self.image attribute:NSLayoutAttributeHeight
		relatedBy:NSLayoutRelationEqual
		toItem:self.slider attribute:NSLayoutAttributeHeight
		multiplier:1 constant:0];
	
	// TODO why does this not work initially?
	// TODO and why does it not work if set in Interface Builder?
	// it only starts working after switching off and on the stretchy image view one
	[self.stretchyView setHidden:YES];
}

- (void)windowWillClose:(NSNotification *)note
{
	[NSApp terminate:self];
}

- (BOOL)isHorizontal
{
	return [self.horizontal state] == NSOnState;
}

- (BOOL)isSpaced
{
	return [self.spaced state] == NSOnState;
}

- (BOOL)isImageStretchy
{
	return [self.stretchyImage state] == NSOnState;
}

- (BOOL)isSliderStretchy
{
	return [self.stretchySlider state] == NSOnState;
}

- (IBAction)horizontalChanged:(id)sender
{
	NSUserInterfaceLayoutOrientation o;
	
	o = NSUserInterfaceLayoutOrientationVertical;
	if ([self isHorizontal])
		o = NSUserInterfaceLayoutOrientationHorizontal;
	[self.stackView setOrientation:o];
	
	if ([self isHorizontal]) {
		[NSLayoutConstraint deactivateConstraints:self.equalWidthConstraints];
		[NSLayoutConstraint activateConstraints:self.equalHeightConstraints];
		[self.bothStretchyHeightConstraint setActive:NO];
		[self.b setContentHuggingPriority:NSLayoutPriorityDefaultHigh forOrientation:NSLayoutConstraintOrientationHorizontal];
		[self.tf setContentHuggingPriority:NSLayoutPriorityDefaultHigh forOrientation:NSLayoutConstraintOrientationHorizontal];
	} else {
		[NSLayoutConstraint deactivateConstraints:self.equalHeightConstraints];
		[NSLayoutConstraint activateConstraints:self.equalWidthConstraints];
		[self.bothStretchyWidthConstraint setActive:NO];
		[self.b setContentHuggingPriority:NSLayoutPriorityDefaultHigh forOrientation:NSLayoutConstraintOrientationHorizontal];
		[self.tf setContentHuggingPriority:NSLayoutPriorityDefaultLow forOrientation:NSLayoutConstraintOrientationHorizontal];
	}
	[self updateStretchy:sender];
}

- (IBAction)spacedChanged:(id)sender
{
	CGFloat spacing;
	
	spacing = 0;
	if ([self isSpaced])
		spacing = 8;
	[self.stackView setSpacing:spacing];
}

- (IBAction)updateStretchy:(id)sender
{
	NSLayoutConstraintOrientation o;
	NSLayoutPriority pri;
	NSLayoutConstraint *bothc;
	
	o = NSLayoutConstraintOrientationVertical;
	if ([self isHorizontal])
		o = NSLayoutConstraintOrientationHorizontal;

	pri = NSLayoutPriorityDefaultHigh;
	if ([self isImageStretchy])
		pri = NSLayoutPriorityDefaultLow;
	[self.image setContentHuggingPriority:pri forOrientation:o];

	pri = NSLayoutPriorityDefaultHigh;
	if ([self isSliderStretchy])
		pri = NSLayoutPriorityDefaultLow;
	[self.slider setContentHuggingPriority:pri forOrientation:o];

	bothc = self.bothStretchyHeightConstraint;
	if ([self isHorizontal])
		bothc = self.bothStretchyWidthConstraint;
	[bothc setActive:([self isImageStretchy] && [self isSliderStretchy])];
	
	[self.stretchyView setHidden:([self isImageStretchy] || [self isSliderStretchy])];
}

@end

@interface intrinsicSizeTextField : NSTextField
@end

@implementation intrinsicSizeTextField

- (NSSize)intrinsicContentSize
{
	NSSize s;
	
	s = [super intrinsicContentSize];
	s.width = 96;
	return s;
}

@end

@interface intrinsicSizeSlider : NSSlider
@end

@implementation intrinsicSizeSlider

- (NSSize)intrinsicContentSize
{
	NSSize s;
	
	s = [super intrinsicContentSize];
	s.height = 91;
	return s;
}

@end

@interface zeroIntrinsicSizeView : NSView
@end

@implementation zeroIntrinsicSizeView

- (NSSize)intrinsicContentSize
{
	return NSZeroSize;
}

@end
