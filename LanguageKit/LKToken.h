#import <Foundation/NSString.h>

/**
 * LKToken implements a token for use in parsers.  A token is a symbol or word
 * in a source program.  The LKToken class stores this as a range and a pointer
 * to the source program, allowing it to be easily mapped back to the original.
 */
@interface LKToken : NSString {
	/** Source string. */
	NSString *source;
	/** Range within the source string */
	NSRange range;
	/** IMP used for looking up the character at the specified index in the
	 * source string. */
	unichar(*charAtIndex)(id, SEL, unsigned);
}
/**
 * Create a token from a range in the specified source string.
 */
+ (LKToken*) tokenWithRange:(NSRange)aRange inSource:(NSString*)aString;
/**
 * Returns the location in the original program of this token.
 */
- (NSRange) sourceLocation;
/**
 * Returns the source file from which this token was generated.
 */
- (NSString*) sourceDocument;
@end
