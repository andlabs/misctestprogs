// 11 january 2017
#import <Cocoa/Cocoa.h>

int main(int argc, char *argv[])
{
	NSFileManager *fm;
	NSURL *dirURL;
	BOOL (^errorHandler)(NSURL *url, NSError *error);
	NSDirectoryEnumerator<NSURL *> *walk;
	NSURL *file;

	fm = [NSFileManager new];
	dirURL = [[NSURL alloc] initFileURLWithPath:[NSString stringWithUTF8String:argv[1]]];
	errorHandler = ^(NSURL *url, NSError *err) {
		NSLog(@"%@", err);
		exit(1);
		return NO;
	};
	walk = [fm enumeratorAtURL:dirURL
		includingPropertiesForKeys:@[NSURLNameKey, NSURLIsRegularFileKey, NSURLIsReadableKey]
		options:0
		errorHandler:errorHandler];

	for (file = [walk nextObject]; file != nil; file = [walk nextObject]) {
		NSNumber *num;
		NSError *err;


		if ([file getResourceValue:&num forKey:NSURLIsRegularFileKey error:&err] == NO)
			errorHandler(file, err);
		if ([num boolValue] == NO)
			continue;
		if ([file getResourceValue:&num forKey:NSURLIsReadableKey error:&err] == NO)
			errorHandler(file, err);
		if ([num boolValue] == NO)
			continue;
		NSLog(@"%@", file);
	}
	return 0;
}
