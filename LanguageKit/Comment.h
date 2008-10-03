#import "AST.h"

@interface Comment : AST {
	NSString *comment;
}
+ (Comment*) commentForString:(NSString*)aString;
@end
