// 8 june 2017
#import <Cocoa/Cocoa.h>
#import <stdio.h>
#import <string.h>
#import <mach/mach.h>
#import <mach/mach_time.h>

// TODO consider MediaLibrary?

const char *scriptSource =
	"tell application \"iTunes\"\n"
	"	set allItems to {}\n"
	"	repeat with t in tracks\n"
	"		set yr to year of t\n"
	"		set ar to album artist of t\n"
	"		if ar is \"\" then\n"
	"			set ar to artist of t\n"
	"		end if\n"
	"		set al to album of t\n"
	"		set newItem to {|year|:yr, |artist|:ar, |album|:al}\n"
	"		set end of allItems to newItem\n"
	"	end repeat\n"
	"	allItems\n"
	"end tell\n";

static NSDictionary *unpackRecord(NSAppleEventDescriptor *desc)
{
	NSMutableDictionary *dict;
	NSInteger i, n;

	// TODO figure out why this is needed; free desc afterward?
	desc = [desc descriptorAtIndex:1];
	dict = [NSMutableDictionary new];
	n = [desc numberOfItems];
	// note: 1-based
	for (i = 1; i <= n; i += 2) {
		NSAppleEventDescriptor *key, *value;
		NSString *keystr;
		id valueobj;

		key = [desc descriptorAtIndex:i];
		value = [desc descriptorAtIndex:(i + 1)];
		keystr = [key stringValue];
		if ([keystr isEqualToString:@"year"]) {
			SInt32 v;

			v = [value int32Value];
			valueobj = [NSNumber numberWithInteger:((NSInteger) v)];
			// TODO do not free valueobj
		} else
			valueobj = [value stringValue];
		[dict setObject:valueobj forKey:keystr];
		// TODO release key, value, keystr, or valueobj?
	}
	return dict;
}

int main(int argc, char *argv[])
{
	BOOL verbose = NO;
	NSString *source;
	NSAppleScript *script;
	NSAppleEventDescriptor *desc;
	NSDictionary *err;
	uint64_t start, end;
	NSInteger i, trackCount;

	switch (argc) {
	case 1:
		break;
	case 2:
		if (strcmp(argv[1], "-v") == 0) {
			verbose = YES;
			break;
		}
		// fall through
	default:
		fprintf(stderr, "usage: %s [-v]\n", argv[0]);
		return 1;
	}

	source = [NSString stringWithUTF8String:scriptSource];
	script = [[NSAppleScript alloc] initWithSource:source];

	start = mach_absolute_time();
	desc = [script executeAndReturnError:&err];
	end = mach_absolute_time();
	if (desc == nil) {
		fprintf(stderr, "error: script execution failed: %s\n",
			[[err description] UTF8String]);
		return 1;
	}
	if (verbose) {
		mach_timebase_info_data_t mt;
		double sec;

		// should not fail; see http://stackoverflow.com/questions/31450517/what-are-the-possible-return-values-for-mach-timebase-info
		// also true on 10.12 at least: https://opensource.apple.com/source/xnu/xnu-3789.1.32/libsyscall/wrappers/mach_timebase_info.c.auto.html + https://opensource.apple.com/source/xnu/xnu-3789.1.32/osfmk/kern/clock.c.auto.html
		mach_timebase_info(&mt);
		end -= start;
		end = end * mt.numer / mt.denom;
		sec = ((double) end) / ((double) NSEC_PER_SEC);
		printf("time to issue script: %gs\n", sec);
	}

	trackCount = [desc numberOfItems];
	if (verbose)
		printf("track count: %ld\n", (long) trackCount);
	// note: 1-based
	for (i = 1; i <= trackCount; i++) {
		NSAppleEventDescriptor *track;
		NSDictionary *data;

		track = [desc descriptorAtIndex:i];
		data = unpackRecord(track);
		printf("%s\n", [[data description] UTF8String]);
		[data release];// TODO
		// TODO release track?
	}

	// TODO clean up?
	return 0;
}
