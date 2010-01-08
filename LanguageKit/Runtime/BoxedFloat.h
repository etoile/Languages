#import <Foundation/Foundation.h>

@interface BoxedFloat : NSObject
{
	@public
	double value;
}
+ (BoxedFloat*) boxedFloatWithCString:(const char*) aString;
+ (BoxedFloat*) boxedFloatWithDouble:(double)aVal;
+ (BoxedFloat*) boxedFloatWithFloat:(float)aVal;
@end