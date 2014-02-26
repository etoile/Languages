#import <LanguageKit/LKAST.h>

@interface LKEnumReference : LKAST
{
	NSString *enumName;
	NSString *enumValue;
	long long value;
}
- (id)initWithValue: (NSString*)aValue inEnumeration: (NSString*)anEnum;
@end
