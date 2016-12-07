// 6 december 2016
#import <Cocoa/Cocoa.h>
#import <objc/objc-runtime.h>
#import <stdint.h>
#import <stdlib.h>
#import <string.h>

@protocol DecodedType<NSObject>
+ (BOOL)decodes:(const char *)encoded;
- (id)init:(const char *)encoded next:(const char **)next;
- (void)dump:(id)obj ivar:(Ivar)ivar indent:(int)indent;
- (void)dump:(void *)val indent:(int)indent;
@end

#define DECODED(name) @interface name : NSObject<DecodedType> \
	@end \
	@implementation name

DECODED(InvalidType) {
	const char *e;
}

+ (BOOL)decodes:(const char *)encoded
{
	return YES;
}

- (id)init:(const char *)encoded next:(const char **)next
{
	self = [super init];
	if (self) {
		self->e = encoded;
		*next = encoded + strlen(encoded);
	}
	return self;
}

- (void)dump:(id)obj ivar:(Ivar)ivar indent:(int)indent
{
	[self dump:NULL indent:indent];
}

- (void)dump:(void *)val indent:(int)indent
{
	printf("%*s(invalid type %s)", indent, "", self->e);
}

@end

static NSMutableArray *decodedTypes = nil;

#define REGISTER \
	+ (void)load \
	{ \
		if (decodedTypes == nil) \
			decodedTypes = [NSMutableArray new]; \
		[decodedTypes addObject:[NSValue valueWithPointer:self]]; \
	}

static id<DecodedType> decodeType(const char *encoded, const char **next)
{
	NSValue *v;

	for (v in decodedTypes) {
		// TODO make this specific somehow
		Class c;

		c = (Class) [v pointerValue];
		if ([c decodes:encoded])
			return [[c alloc] init:encoded next:next];
	}
	return [[InvalidType alloc] init:encoded next:next];
}

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
			id<DecodedType> decoded;
			const char *next;

			name = ivar_getName(ivar);
			indent++;
			printf("%*s%s ", indent, "", name);

			enc = ivar_getTypeEncoding(ivar);
			decoded = decodeType(enc, &next);
			[decoded dump:s ivar:ivar indent:indent];
			printf("\n");
			[decoded release];

			indent--;
		}
		free(ivars);

		printf("%*s}\n", indent, "");
		indent--;
	}
	printf("%*s]\n", indent, "");
}


DECODED(BasicType) {
	char c;
}

REGISTER

+ (BOOL)decodes:(const char *)encoded
{
	return strchr("cislqCISLQfdbv*#:", *encoded) != NULL;
}

- (id)init:(const char *)encoded next:(const char **)next
{
	self = [super init];
	if (self) {
		self->c = *encoded;
		*next = encoded + 1;
	}
	return self;
}

- (void)dump:(id)obj ivar:(Ivar)ivar indent:(int)indent
{
	ptrdiff_t off;
	uint8_t *base = (uint8_t *) obj;

	off = ivar_getOffset(ivar);
	base += off;
	// on 64-bit, isa is mangled
	if (off == 0 && self->c == '#') {
		printf("Class");
		return;
	}
	[self dump:base indent:indent];
}

- (void)dump:(void *)val indent:(int)indent
{
	switch (self->c) {
#define C(ch, type, fmt) case ch: printf("%s = " fmt, #type, *((type *) val)); break;
	C('c', char, "0x%02hhX")
	C('i', int, "%d")
	C('s', short, "%hd")
	// TODO cap at 32 bits?
	C('l', long, "%ld")
	C('q', long long, "%lld")
	C('C', unsigned char, "0x%02hhX")
	C('I', unsigned int, "%u")
	C('S', unsigned short, "%hu")
	// TODO cap at 32 bits?
	C('L', unsigned long, "%lu")
	C('Q', unsigned long long, "%llu")
	case 'f': printf("float = %g", (double) (*((float *) val))); break;
	C('d', double, "%g")
	case 'b': printf("_Bool = %d", (int) (*((_Bool *) val))); break;
	case 'v': printf("void"); break;
	C('*', char *, "%s")
	case '#': printf("Class = %s", [NSStringFromClass(*((Class *) val)) UTF8String]); break;
	case ':': printf("SEL = %s", [NSStringFromSelector(*((SEL *) val)) UTF8String]); break;
#undef C
	}
}

@end

DECODED(ObjCObjectType) {
	NSString *typename;
}

REGISTER

+ (BOOL)decodes:(const char *)encoded
{
	return *encoded == '@';
}

- (id)init:(const char *)encoded next:(const char **)next
{
	self = [super init];
	if (self) {
		encoded++;
		if (*encoded == '"') {
			const char *end;

			encoded++;
			for (end = encoded; *end != '"'; end++)
				;
			self->typename = [[NSString alloc] initWithBytes:encoded
				length:(end - encoded)
				encoding:NSUTF8StringEncoding];
			end++;
			encoded = end;
		} else
			self->typename = nil;
		*next = encoded;
	}
	return self;
}

- (void)dealloc
{
	if (self->typename != nil)
		[self->typename release];
	[super dealloc];
}

- (void)dump:(id)obj ivar:(Ivar)ivar indent:(int)indent
{
	id val;

	val = object_getIvar(obj, ivar);
	[self dump:((void *) val) indent:indent];
}

- (void)dump:(void *)val indent:(int)indent
{
	const char *tn;
	id obj = (id) val;

	tn = "id";
	if (self->typename != nil)
		tn = [self->typename UTF8String];
	printf("%s ", tn);
	if (obj == nil)
		printf("= nil");
	else
		dumpIvar(obj, indent);
}

@end

/*
class ArrayType : DecodedType {
	private var elem: DecodedType
	private var count: UIntMax = 0
	
	public var String: String {
		get {
			return self.elem.String + "[\(self.count)]"
		}
	}
	
	public static func Decodes(_ encoded: String) -> Bool {
		return firstChar(encoded) == "[" &&
			encoded.characters.count >= 4
		// TODO also look for the ]? and the number?
	}
	
	public required init(_ encoded: String) {
		var e2 = restOfString(encoded)
		e2 = swallowNumber(e2, &self.count)
		self.elem = decodeEncodedType(e2)
	}
}
*/
