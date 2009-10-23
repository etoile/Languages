#import "LKToken.h"
#import <EtoileFoundation/EtoileFoundation.h>

typedef unichar(*CIMP)(id, SEL, unsigned);

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
	return [[[LKToken alloc] initWithRange:aRange inSource:aString] autorelease];
}
- (unsigned) length
{
	return range.length;
}
- (unichar) characterAtIndex:(unsigned)index
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
- (void) dealloc
{
	[source release];
	[super dealloc];
}
@end
