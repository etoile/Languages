#import "LKComment.h"

@implementation LKComment
- (LKComment*) initWithString:(NSString*)aString
{
	SUPERINIT;
	ASSIGN(comment, aString);
	return self;
}
+ (LKComment*) commentWithString:(NSString*)aString
{
	return [[LKComment alloc] initWithString:aString];
}
- (NSString*) description
{
	return [NSString stringWithFormat:@"\"%@\"", comment];
}
- (NSString*) stringValue
{
	return comment;
}
- (void*) compileWithGenerator: (id<LKCodeGenerator>)aGenerator
{
	// Comments do not turn into code.
	return NULL;
}
- (BOOL) isComment
{
	return YES;
}
// Comments are always semantically valid - no checking needed.
- (BOOL) check { return YES; }
@end
