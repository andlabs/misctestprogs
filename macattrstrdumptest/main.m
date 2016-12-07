// 6 december 2016
#import <Cocoa/Cocoa.h>
#import <objc/objc-runtime.h>
#import <stdio.h>

void dumpIvar(id s, int indent)
{
	Class class;

	printf("[\n");
	for (class = object_getClass(s); class != Nil; class = class_getSuperclass(class)) {
		Ivar *ivars;
		unsigned int i, n;

		indent++;
		printf("%*s%s {\n", indent, "", class_getName(class));

		ivars = class_copyIvarList(class, &n);
		for (i = 0; i < n; i++) {
			Ivar ivar = ivars[i];
			const char *name;
			const char *enc;

			name = ivar_getName(ivar);
			indent++;
			printf("%*s%s ", indent, "", name);

			enc = ivar_getTypeEncoding(ivar);
/*
			var decoded = decodeEncodedType(typ)
			print("\(decoded.String)")
			if !(decoded is ObjCObjectType) {		// object_getIvar() returns objects
				print("\(name): \(typ)")
				return
			}
			let val = object_getIvar(s, ivar)
*/
			printf("%s\n", enc);
/*
			if let realval = val {
				print("[")
				dumpIvar(realval as! NSObject)
				print("]")
			}
*/

			indent--;
		}
		free(ivars);

		printf("%*s}\n", indent, "");
		indent--;
	}
	printf("%*s]\n", indent, "");
}

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

	printf("==\n");

	[s addAttributes:@{
		NSForegroundColorAttributeName:		[NSColor controlHighlightColor],
	} range:NSMakeRange(0, 1)];
	printf("%s\n", [[s description] UTF8String]);
	printf("--\n");
	toplist(s);
	printf("--\n");
	dumpIvar(s, 0);

	[s release];
	return 0;
}
