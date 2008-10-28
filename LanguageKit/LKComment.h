#import "LKAST.h"

@interface LKComment : LKAST {
	NSString *comment;
}
+ (LKComment*) commentForString:(NSString*)aString;
@end
