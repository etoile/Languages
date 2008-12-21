#import <EtoileFoundation/EtoileFoundation.h>
#import <LanguageKit/LKToken.h>
#import <LanguageKit/LKAST.h>
#import "EScriptParser.h"
#include <ctype.h>
#include "escript.h"

typedef unichar(*CIMP)(id, SEL, unsigned);


NSMapTable *keywords;

@implementation EScriptParser
/* From Lemon: */
void *EScriptParseAlloc(void *(*mallocProc)(size_t));
void EScriptParse(void *yyp, int yymajor, id yyminor, EScriptParser* p);
void EScriptParseFree(void *p, void (*freeProc)(void*));


#define CALL_PARSER(token, arg) EScriptParse(parser, TOKEN_##token, arg, self);// NSLog(@"Parsing %@ (%s)", arg, #token)
#define CHAR(x) charAt(s, charSel, x)
#define WHILE(is) for(j=i ; j<sLength-1 && is(c) ; c=CHAR(++j)) {}
#define WORD_TOKEN substr(LKTokenClass, substrSel, NSMakeRange(i, j-i), s)
#define CASE(start, end, function)\
	if(start(c))\
	{\
		WHILE(end)\
		function\
		i = MAX(i,j-1);\
	}
//#define PARSE(start, end, type) CASE(start, end, { CALL_PARSER(type, WORD_TOKEN);})
#define CHARCASE(letter, token) case letter: CALL_PARSER(token, WORD_TOKEN); break;

#define SET_KEYWORD(key, token) \
	NSMapInsert(keywords, @#key, (void*)(uintptr_t)TOKEN_ ## token)
+ (void) initialize
{
	keywords = NSCreateMapTable(NSObjectMapKeyCallBacks, NSIntMapValueCallBacks, 2);
	SET_KEYWORD(new, NEW);
	SET_KEYWORD(function, FUNCTION);
	SET_KEYWORD(if, IF);
	SET_KEYWORD(while, WHILE);
	SET_KEYWORD(var, VAR);
}

- (AST*) parseString:(NSString*)s
{
	unsigned sLength = [s length];
	/* Cache some IMPs of methods we call a lot */
	SEL charSel = @selector(characterAtIndex:);
	SEL substrSel = @selector(tokenWithRange:inSource:);
	CIMP charAt = (CIMP)[s methodForSelector:charSel];

	IMP substr = [LKToken methodForSelector:substrSel];
	Class LKTokenClass = [LKToken class];
	/* Set up the parser */
	void * parser = EScriptParseAlloc( malloc );

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
			int token = NSMapGet(keywords, word);
			if (token == 0)
			{
				token = WORD_TOKEN;
			}
			EScriptParse(parser, token, word, self);
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
				default:
					NSLog(@"Weird character '%c' found on line %d", c, line);
			}
		}
	}
	NS_HANDLER
		EScriptParseFree(parser, free);
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
	EScriptParse(parser, 0, nil, self);
	EScriptParseFree(parser, free);
	return delegate;
}

- (void) setDelegate:(AST*)ast
{
	ASSIGN(delegate, ast);
}
@end
