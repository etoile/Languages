#import "AST.h"

@interface LKComment : LKAST {
	NSString *comment;
}
+ (LKComment*) commentForString:(NSString*)aString;
@end
