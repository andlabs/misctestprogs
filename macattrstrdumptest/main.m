// 6 december 2016
#import <Cocoa/Cocoa.h>
#import <objc/objc-runtime.h>
#import <stdio.h>

extern void dumpIvar(id s, int indent);

static void toplist(id s)
{
	NSMutableData *md;
	NSKeyedArchiver *ka;
	NSString *str;

	md = [NSMutableData new];
	ka = [[NSKeyedArchiver alloc] initForWritingWithMutableData:md];
	[ka setOutputFormat:NSPropertyListXMLFormat_v1_0];
	[ka encodeObject:s forKey:@"root"];
	[ka finishEncoding];
	str = [[NSString alloc] initWithData:md encoding:NSUTF8StringEncoding];
	printf("%s\n", [str UTF8String]);
	[str release];
	[ka release];
	[md release];
}

int main(void)
{
	NSMutableAttributedString *s;

	s = [[NSMutableAttributedString alloc] initWithString:@"abc"];
	[s setAttributes:@{
		NSForegroundColorAttributeName:		[NSColor redColor],
	} range:NSMakeRange(0, 1)];
	[s setAttributes:@{
		NSForegroundColorAttributeName:		[NSColor greenColor],
	} range:NSMakeRange(1, 1)];
	[s setAttributes:@{
		NSForegroundColorAttributeName:		[NSColor blueColor],
	} range:NSMakeRange(2, 1)];
	printf("%s\n", [[s description] UTF8String]);

	printf("==\n");

	[s replaceCharactersInRange:NSMakeRange(1, 0) withString:@"d"];
	printf("%s\n", [[s description] UTF8String]);
	printf("--\n");
	toplist(s);
	printf("--\n");
	dumpIvar(s, 0);

	printf("\n==\n");

	[s addAttributes:@{
		NSForegroundColorAttributeName:		[NSColor controlHighlightColor],
	} range:NSMakeRange(0, 1)];
	printf("%s\n", [[s description] UTF8String]);
	printf("--\n");
	toplist(s);
	printf("--\n");
	dumpIvar(s, 0);

	printf("\n==\n");

	[s addAttributes:@{
		NSForegroundColorAttributeName:		[NSColor purpleColor],
	} range:NSMakeRange(1, 1)];
	printf("%s\n", [[s description] UTF8String]);
	printf("--\n");
	toplist(s);
	printf("--\n");
	dumpIvar(s, 0);

	printf("\n");

	[s release];
	return 0;
}
