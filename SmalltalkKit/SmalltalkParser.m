#import <EtoileFoundation/EtoileFoundation.h>
#import "SmalltalkParser.h"
#include <ctype.h>
#include "smalltalk.h"

typedef unichar(*CIMP)(id, SEL, unsigned);

@interface Token : NSString {
	NSString *source;
	NSRange range;
	CIMP charAtIndex;
}
+ (Token*) tokenWithRange:(NSRange)aRange inSource:(NSString*)aString;
@end
@implementation Token
- (Token*) initWithRange:(NSRange)aRange inSource:(NSString*)aString
{
	SELFINIT;
	charAtIndex = (CIMP)[aString methodForSelector:@selector(characterAtIndex:)];
	ASSIGN(source, aString);
	range = aRange;
	return self;
}
+ (Token*) tokenWithRange:(NSRange)aRange inSource:(NSString*)aString
{
	return [[[Token alloc] initWithRange:aRange inSource:aString] autorelease];
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
- (void) dealloc
{
	[source release];
	[super dealloc];
}
@end




@implementation SmalltalkParser
/* From Lemon: */
void *ParseAlloc(void *(*mallocProc)(size_t));
void Parse(void *yyp, int yymajor, id yyminor, SmalltalkParser* p);
void ParseFree(void *p, void (*freeProc)(void*));


#define CALL_PARSER(token, arg) Parse(parser, TOKEN_##token, arg, self);// NSLog(@"Parsing %@ (%s)", arg, #token)
#define CHAR(x) charAt(s, charSel, x)
#define WHILE(is) for(j=i ; j<sLength-1 && is(c) ; c=CHAR(++j)) {}
#define WORD_TOKEN substr(TokenClass, substrSel, NSMakeRange(i, j-i), s)
#define CASE(start, end, function)\
	if(start(c))\
	{\
		WHILE(end)\
		function\
		i = MAX(i,j-1);\
	}
//#define PARSE(start, end, type) CASE(start, end, { CALL_PARSER(type, WORD_TOKEN);})
#define CHARCASE(letter, token) case letter: CALL_PARSER(token, @"char"); break;

- (AST*) parseString:(NSString*)s
{
	unsigned sLength = [s length];
	/* Cache some IMPs of methods we call a lot */
	SEL charSel = @selector(characterAtIndex:);
	SEL substrSel = @selector(tokenWithRange:inSource:);
	CIMP charAt = (CIMP)[s methodForSelector:charSel];

	IMP substr = [Token methodForSelector:substrSel];
	Class TokenClass = [Token class];
	/* Set up the parser */
	void * parser = ParseAlloc( malloc );

	// Volatile to ensure that they are preserved over longjmp calls.  This is
	// going to make things a bit slower, so a better solution might be to move
	// them into ivars.
	volatile int line = 1;
	volatile unsigned int j;
	volatile unsigned int i;
	unsigned lineStart = 0;
	NS_DURING
	for(i=0 ; i<sLength; i++)
	{
		unichar c = CHAR(i);
		CASE(isalpha, isalnum, 
		{
			NSString * word = WORD_TOKEN;
			if([word isEqualToString:@"subclass"])
			{
				CALL_PARSER(SUBCLASS, word);
			}
			else if([word isEqualToString:@"extend"])
			{
				CALL_PARSER(EXTEND, word);
			}
			else if(':' == c)
			{
				j++;
				CALL_PARSER(KEYWORD, WORD_TOKEN);
			}
			else
			{
				CALL_PARSER(WORD, word);
			}
		})
		else if (isspace(c))
		{
			for(j=i ; j<sLength-1 && isspace(c) ; c=CHAR(++j))
		   	{
				if (c == '\n')
				{
					line++;
					lineStart = j;
				}
			}
			i = MAX(i,j-1);
		}
		else if ('"' == c && i<sLength - 2)
		{
			c=CHAR(++i);
			for(j=i ; j<sLength-1 && '"' != c ; c=CHAR(++j))
			{
				if (c == '\n')
				{
					line++;
					lineStart = j;
				}
			}
			CALL_PARSER(COMMENT, WORD_TOKEN);
			i = j;
		}
		else if ('\'' == c && i<sLength - 2)
		{
			j = i;
			do
			{
				c=CHAR(++j);
			} while (j<sLength-1 && '\'' != c);
			i++;
			CALL_PARSER(STRING, WORD_TOKEN);
			j++;
			i = MAX(i,j-1);
		}
		else CASE(isdigit, isdigit, {CALL_PARSER(NUMBER, WORD_TOKEN);})
		else
		{
			switch(c)
			{
				CHARCASE('|', BAR)
				CHARCASE('@', AT)
				//CHARCASE('#', HASH)
				CHARCASE(',', COMMA)
				CHARCASE(':', COLON)
				CHARCASE(';', SEMICOLON)
				CHARCASE('.', STOP)
				CHARCASE('+', PLUS)
				CHARCASE('-', MINUS)
				CHARCASE('*', STAR)
				CHARCASE('/', SLASH)
				CHARCASE('=', EQ)
				CHARCASE('<', LT)
				CHARCASE('>', GT)
				CHARCASE('(', LPAREN)
				CHARCASE(')', RPAREN)
				CHARCASE('[', LSQBRACK)
				CHARCASE(']', RSQBRACK)
				CHARCASE('{', LBRACE)
				CHARCASE('}', RBRACE)
				CHARCASE('^', RETURN)
				case '#':
					j = i++;
					do
					{
						c=CHAR(++j);
					} while (j<sLength-1 && !isspace(c));
					CALL_PARSER(SYMBOL, WORD_TOKEN);
					NSLog(@"Symbol: %@", WORD_TOKEN);
					i = j;
					break;
				default:
					NSLog(@"Weird character '%c' found on line %d", c, line);
			}
		}
	}
	NS_HANDLER
		ParseFree(parser, free);
		NSString * errorLine = [s substringFromIndex:lineStart+1];
		NSRange lineEnd = [errorLine rangeOfString:@"\n"];
		if (lineEnd.location != NSNotFound)
		{
			errorLine = [errorLine substringToIndex:lineEnd.location];
		}
		NSDictionary *userinfo = D(
		                          [NSNumber numberWithInt:line], @"lineNumber",
		                          [NSNumber numberWithInt:(i-lineStart)], @"character",
		                          errorLine, @"line");
		[[NSException exceptionWithName:@"ParseError"
		                         reason:@"Unexpected token"
		                       userInfo:userinfo] raise];
	NS_ENDHANDLER
	Parse(parser, 0, nil, self);
	ParseFree(parser, free);
	return delegate;
}

- (void) setDelegate:(AST*)ast
{
	ASSIGN(delegate, ast);
}
@end
