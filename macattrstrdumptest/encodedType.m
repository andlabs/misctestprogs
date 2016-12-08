// 6 december 2016
#import <Cocoa/Cocoa.h>
#import <objc/objc-runtime.h>
#import <stdint.h>
#import <stdlib.h>
#import <string.h>

// because NSGetSizeAndAlignment() does not handle bit fields
static const char *safeGetSizeAndAlignment(const char *typePtr, NSUInteger *sizep, NSUInteger *alignp)
{
	unsigned long long n;
	NSUInteger size;
	BOOL manual;

	manual = YES;
	if (*typePtr != 'b')
		manual = NO;
	else if (typePtr[1] < '0' || typePtr[1] > '9')
		// pass invalid b sequencies to the real function so an exception can be thrown
		manual = NO;
	if (!manual)
		return NSGetSizeAndAlignment(typePtr, sizep, alignp);
	typePtr++;
	n = strtoull(typePtr, &typePtr, 10);
	size = n / 8;
	if (n % 8 != 0)
		size++;
	if (sizep != NULL)
		*sizep = size;
	if (alignp != NULL)
		*alignp = size;
	return typePtr;
}

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
	printf("(invalid type %s)", self->e);
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

#define permissive 0

#if permissive
static jmp_buf segvjmp;

static void dumpSEGV(int sig)
{
	longjmp(segvjmp, 1);
}
#endif

void dumpIvar(id s, int indent)
{
	Class class;
	int origIndent = indent;
#if permissive
	void (*prevSignal)(int);
	jmp_buf prevjb;

	prevSignal = signal(SIGSEGV, dumpSEGV);
	memmove(prevjb, segvjmp, sizeof (jmp_buf));
	if (setjmp(segvjmp) != 0) {
		printf("\n** fail\n");
		indent = origIndent;
		goto bail;
	}
#endif

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
			printf("%*s%s: ", indent, "", name);

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

bail:
	printf("%*s]", indent, "");
#if permissive
	memmove(segvjmp, prevjb, sizeof (jmp_buf));
	signal(SIGSEGV, prevSignal);
#endif
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
	[self dump:&val indent:indent];
}

- (void)dump:(void *)val indent:(int)indent
{
	const char *tn;
	id obj;

	obj = *((id *) val);
	tn = "id";
	if (self->typename != nil)
		tn = [self->typename UTF8String];
	printf("object(%s) ", tn);
	if (obj == nil)
		printf("= nil");
	else
		dumpIvar(obj, indent);
}

@end

DECODED(ArrayType) {
	unsigned long long count;
	id<DecodedType> elem;
	NSUInteger size;
	NSUInteger alignment;
}

REGISTER

+ (BOOL)process:(const char *)encoded count:(unsigned long long *)c elem:(id<DecodedType> *)e size:(NSUInteger *)s alignment:(NSUInteger *)a next:(const char **)next
{
	if (*encoded != '[')
		return NO;
	encoded++;
	if (*encoded < '0' || *encoded > '9')
		return NO;
	*c = strtoull(encoded, next, 10);
	encoded = *next;
	*e = decodeType(encoded, next);
	if ([*e isKindOfClass:[InvalidType class]]) {
		[*e release];
		*e = nil;
		return NO;
	}
	safeGetSizeAndAlignment(encoded, s, a);
	encoded = *next;
	if (*encoded != ']') {
		[*e release];
		*e = nil;
		return NO;
	}
	encoded++;
	*next = encoded;
	return YES;
}

+ (BOOL)decodes:(const char *)encoded
{
	BOOL ret;
	unsigned long long c;
	id<DecodedType> e = nil;
	const char *next;
	NSUInteger s, a;

	ret = [self process:encoded count:&c elem:&e size:&s alignment:&a next:&next];
	if (e != nil)
		[e release];
	return ret;
}

- (id)init:(const char *)encoded next:(const char **)next
{
	self = [super init];
	if (self)
		[ArrayType process:encoded
			count:&(self->count)
			elem:&(self->elem)
			size:&(self->size)
			alignment:&(self->alignment)
			next:next];
	return self;
}

- (void)dump:(id)obj ivar:(Ivar)ivar indent:(int)indent
{
	ptrdiff_t off;
	uint8_t *base = (uint8_t *) obj;

	off = ivar_getOffset(ivar);
	base += off;
	[self dump:base indent:indent];
}

- (void)dump:(void *)val indent:(int)indent
{
	unsigned long long i, n;
	uint8_t *p = (uint8_t *) val;

	printf("array(%llu) [\n", self->count);
	indent++;

	n = self->count;
	if (n == 0) {
		// override for zero-length trailing dynamic arrays
		n = 5;
	}
	for (i = 0; i < n; i++) {
		printf("%*s", indent, "");
		[self->elem dump:p indent:indent];
		printf(",\n");
		p += self->alignment;
	}

	indent--;
	printf("%*s]", indent, "");
}

@end

DECODED(StructUnionType) {
	BOOL isUnion;
	NSString *name;
	NSMutableArray *elemNames;
	NSMutableArray *elemTypes;
	NSMutableArray *elemSizes;
	NSMutableArray *elemAlignments;
}

REGISTER

+ (BOOL)prepare:(const char *)encoded isUnion:(BOOL *)iu name:(NSString **)nm elemNames:(NSMutableArray **)enames elemTypes:(NSMutableArray **)etypes elemSizes:(NSMutableArray **)esizes elemAlignments:(NSMutableArray **)ealigns next:(const char **)next
{
	char closing;

	*nm = nil;
	*enames = nil;
	*etypes = nil;
	*esizes = nil;
	*ealigns = nil;

	switch (*encoded) {
	case '{':
		*iu = NO;
		closing = '}';
		break;
	case '(':
		*iu = YES;
		closing = ')';
		break;
	default:
		goto fail;
	}
	encoded++;
	if (*encoded == '?') {
		*nm = nil;
		encoded++;
		if (*encoded != '=')
			goto fail;
	} else {
		const char *end;

		for (end = encoded; *end != '\0' && *end != '='; end++)
			;
		if (*end == '\0')
			goto fail;
		*nm = [[NSString alloc] initWithBytes:encoded
			length:(end - encoded)
			encoding:NSUTF8StringEncoding];
		encoded = end;
	}
	encoded++;

	*enames = [NSMutableArray new];
	*etypes = [NSMutableArray new];
	*esizes = [NSMutableArray new];
	*ealigns = [NSMutableArray new];
	while (*encoded != '\0' && *encoded != closing) {
		NSString *fn;
		id<DecodedType> dt;
		NSUInteger esize, ealign;

		if (*encoded == '"') {
			const char *end;

			encoded++;
			for (end = encoded; *end != '\0' && *end != '"'; end++)
				;
			if (*end == '\0')
				goto fail;
			if (end == encoded)		// empty string
				fn = [@"_" copy];
			else
				fn = [[NSString alloc] initWithBytes:encoded
					length:(end - encoded)
					encoding:NSUTF8StringEncoding];
			encoded = end;
			encoded++;
		} else
			fn = [@"_" copy];
		[*enames addObject:fn];
		[fn release];

		dt = decodeType(encoded, next);
		if ([dt isKindOfClass:[InvalidType class]]) {
			[dt release];
			goto fail;
		}
		safeGetSizeAndAlignment(encoded, &esize, &ealign);
		encoded = *next;
		[*etypes addObject:dt];
		[dt release];
		[*esizes addObject:[NSNumber numberWithUnsignedInteger:esize]];
		[*ealigns addObject:[NSNumber numberWithUnsignedInteger:ealign]];
	}

	if (*encoded == closing) {
		encoded++;
		*next = encoded;
		return YES;
	}
	// otherwise fall through

fail:
	if (*ealigns != nil) {
		[*ealigns release];
		*ealigns = nil;
	}
	if (*esizes != nil) {
		[*esizes release];
		*esizes = nil;
	}
	if (*etypes != nil) {
		[*etypes release];
		*etypes = nil;
	}
	if (*enames != nil) {
		[*enames release];
		*enames = nil;
	}
	if (*nm != nil) {
		[*nm release];
		*nm = nil;
	}
	return NO;
}

+ (BOOL)decodes:(const char *)encoded
{
	BOOL ret, iu;
	NSString *n;
	NSMutableArray *en, *et, *es, *ea;
	const char *next;

	ret = [self prepare:encoded
		isUnion:&iu
		name:&n
		elemNames:&en
		elemTypes:&et
		elemSizes:&es
		elemAlignments:&ea
		next:&next];
	if (ret) {
		[ea release];
		[es release];
		[et release];
		[en release];
		if (n != nil)
			[n release];
	}
	return ret;
}

- (id)init:(const char *)encoded next:(const char **)next
{
	self = [super init];
	if (self)
		[StructUnionType prepare:encoded
			isUnion:&(self->isUnion)
			name:&(self->name)
			elemNames:&(self->elemNames)
			elemTypes:&(self->elemTypes)
			elemSizes:&(self->elemSizes)
			elemAlignments:&(self->elemAlignments)
			next:next];
	return self;
}

- (void)dealloc
{
	[self->elemAlignments release];
	[self->elemSizes release];
	[self->elemTypes release];
	[self->elemNames release];
	if (self->name != nil)
		[self->name release];
	[super dealloc];
}

- (void)dump:(id)obj ivar:(Ivar)ivar indent:(int)indent
{
	ptrdiff_t off;
	uint8_t *base = (uint8_t *) obj;

	off = ivar_getOffset(ivar);
	base += off;
	[self dump:base indent:indent];
}

- (void)dump:(void *)val indent:(int)indent
{
	NSUInteger i, n;
	uint8_t *p = (uint8_t *) val;

	if (self->isUnion)
		printf("union ");
	else
		printf("struct ");
	if (self->name != nil)
		printf("%s ", [self->name UTF8String]);
	printf("{\n");
	indent++;

	n = [self->elemNames count];
	for (i = 0; i < n; i++) {
		NSString *nam;
		id<DecodedType> dt;

		printf("%*s", indent, "");

		nam = (NSString *) [self->elemNames objectAtIndex:i];
		printf("%s: ", [nam UTF8String]);
		dt = (id<DecodedType>) [self->elemTypes objectAtIndex:i];
		[dt dump:p indent:indent];
		printf("\n");

		if (!self->isUnion) {
			NSNumber *num;

			num = (NSNumber *) [self->elemAlignments objectAtIndex:i];
			p += [num unsignedIntegerValue];
		}
	}

	indent--;
	printf("%*s}", indent, "");
}

@end

DECODED(BitsType) {
	unsigned long long n;
}

REGISTER

+ (BOOL)prepare:(const char *)encoded n:(unsigned long long *)nn next:(const char **)next
{
	if (*encoded != 'b')
		return NO;
	encoded++;
	if (*encoded < '0' || *encoded > '9')
		return NO;
	*nn = strtoull(encoded, next, 10);
	return YES;
}

+ (BOOL)decodes:(const char *)encoded
{
	unsigned long long nn;
	const char *next;

	return [self prepare:encoded n:&nn next:&next];
}

- (id)init:(const char *)encoded next:(const char **)next
{
	self = [super init];
	if (self)
		[BitsType prepare:encoded n:&(self->n) next:next];
	return self;
}

- (void)dump:(id)obj ivar:(Ivar)ivar indent:(int)indent
{
	ptrdiff_t off;
	uint8_t *base = (uint8_t *) obj;

	off = ivar_getOffset(ivar);
	base += off;
	[self dump:base indent:indent];
}

- (void)dump:(void *)val indent:(int)indent
{
	// TODO
	printf("bits(%llu)", self->n);
}

@end

DECODED(PointerType) {
	id<DecodedType> elem;
}

REGISTER

+ (BOOL)prepare:(const char *)encoded elem:(id<DecodedType> *)et next:(const char **)next
{
	*et = nil;
	if (*encoded != '^')
		return NO;
	encoded++;
	*et = decodeType(encoded, next);
	if ([*et isKindOfClass:[InvalidType class]]) {
		[*et release];
		*et = nil;
		return NO;
	}
	return YES;
}

+ (BOOL)decodes:(const char *)encoded
{
	BOOL ret;
	id<DecodedType> dt;
	const char *next;

	ret = [self prepare:encoded elem:&dt next:&next];
	if (dt != nil)
		[dt release];
	return ret;
}

- (id)init:(const char *)encoded next:(const char **)next
{
	self = [super init];
	if (self)
		[PointerType prepare:encoded elem:&(self->elem) next:next];
	return self;
}

- (void)dealloc
{
	[self->elem release];
	[super dealloc];
}

- (void)dump:(id)obj ivar:(Ivar)ivar indent:(int)indent
{
	ptrdiff_t off;
	uint8_t *base = (uint8_t *) obj;

	off = ivar_getOffset(ivar);
	base += off;
	[self dump:base indent:indent];
}

- (void)dump:(void *)val indent:(int)indent
{
	uintptr_t *p = (uintptr_t *) val;
	uintptr_t addr;

	printf("pointer = ");
	addr = *p;
	if (addr == 0) {
		printf("NULL");
		return;
	}
	[self->elem dump:((void *) addr) indent:indent];
}

@end
