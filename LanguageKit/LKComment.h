#import "LKAST.h"

@interface LKComment : LKAST {
	NSString *comment;
}
/**
 * Creates a new comment with the given string.
 */
+ (LKComment*) commentForString:(NSString*)aString;
/**
 * Returns the string value of the comment.
 */
- (NSString*) stringValue;
@end
