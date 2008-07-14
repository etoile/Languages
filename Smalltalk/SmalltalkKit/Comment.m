#import "Comment.h"

@implementation Comment
- (Comment*) initWithString:(NSString*)aString
{
	SELFINIT;
	ASSIGN(comment, aString);
	return self;
}
+ (Comment*) commentForString:(NSString*)aString
{
	return [[[Comment alloc] initWithString:aString] autorelease];
}
- (NSString*) description
{
	return [NSString stringWithFormat:@"\"%@\"", comment];
}
- (void*) compileWith:(id<CodeGenerator>)aGenerator
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
