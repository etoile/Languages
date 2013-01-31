#import "LKToken.h"
#import <EtoileFoundation/EtoileFoundation.h>

typedef unichar(*CIMP)(id, SEL, unsigned);

@implementation NSString (Token)
- (NSRange) sourceLocation
{
	return NSMakeRange(0, 0);
}
@end

@interface LKCompositeToken : NSString
{
	NSString *str;
	NSRange range;
}
- (id)initWithStart: (NSString*)s end: (NSString*)e;
@end
@implementation LKCompositeToken
- (id)initWithStart: (NSString*)s end: (NSString*)e
{
	self = [super init];
	NSRange r = [e sourceLocation];
	str = [NSString stringWithFormat: @"%@%@", s, e];
	range.location = [s sourceLocation].location;
	range.length = r.location - range.location + r.length;
	return self;
}
- (NSString*)stringByAppendingString: (NSString*)other
{
	NSRange secondRange = [other sourceLocation];
	NSUInteger end = secondRange.location + secondRange.length;
	if (end < range.location + range.length)
	{
		return [super stringByAppendingString: other];
	}
	return [[LKCompositeToken alloc] initWithStart: self end: other];
}
- (NSUInteger) length
{
	return [str length];
}
- (unichar) characterAtIndex:(NSUInteger)index
{
	return [str characterAtIndex: index];
}
- (NSRange) sourceLocation
{
	return range;
}
@end


@implementation LKToken
- (LKToken*) initWithRange:(NSRange)aRange inSource:(NSString*)aString
{
	SUPERINIT;
	charAtIndex = (CIMP)[aString methodForSelector:@selector(characterAtIndex:)];
	ASSIGN(source, aString);
	range = aRange;
	return self;
}
+ (LKToken*) tokenWithRange:(NSRange)aRange inSource:(NSString*)aString
{
	return [[LKToken alloc] initWithRange:aRange inSource:aString];
}
- (NSUInteger) length
{
	return range.length;
}
- (unichar) characterAtIndex:(NSUInteger)index
{
	return charAtIndex(source, @selector(characterAtIndex:), index +
			range.location);
}
- (NSRange) sourceLocation
{
	return range;
}
- (NSString*) sourceDocument
{
	return source;
}
- (NSString*)stringByAppendingString: (NSString*)str
{
	NSRange secondRange = [str sourceLocation];
	NSUInteger end = secondRange.location + secondRange.length;
	if (end < range.location + range.length)
	{
		return [super stringByAppendingString: str];
	}
	return [[LKCompositeToken alloc] initWithStart: self end: str];
}
@end
