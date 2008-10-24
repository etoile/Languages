#import "Comment.h"

@implementation LKComment
- (LKComment*) initWithString:(NSString*)aString
{
	SELFINIT;
	ASSIGN(comment, aString);
	return self;
}
+ (LKComment*) commentForString:(NSString*)aString
{
	return [[[LKComment alloc] initWithString:aString] autorelease];
}
- (NSString*) description
{
	return [NSString stringWithFormat:@"\"%@\"", comment];
}
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	// Comments do not turn into code.
	return NULL;
}
// Comments are always semantically valid - no checking needed.
- (void) check {}
- (void) dealloc
{
	[comment release];
	[super dealloc];
}
@end
