#import <EtoileFoundation/EtoileFoundation.h>
#import "Parser.h"
#include <ctype.h>
#include "smalltalk.h"
/*
#define TOKEN_BAR                             1
#define TOKEN_WORD                            2
#define TOKEN_COLON                           3
#define TOKEN_EQ                              4
#define TOKEN_STOP                            5
*/



@implementation Parser (Tokenizer)
/* From Lemon: */
void *ParseAlloc(void *(*mallocProc)(size_t));
void Parse(void *yyp, int yymajor, id yyminor ,Parser* p);
void ParseFree(void *p, void (*freeProc)(void*));


typedef unichar(*CIMP)(id, SEL,...);
#define CALL_PARSER(token, arg) Parse(parser, TOKEN_##token, arg, self)//; NSLog(@"Parsing %s", #token)
#define CHAR(x) charAt(s, charSel, x)
#define WHILE(is) int j; for(j=i ; j<[s length]-1 && is(c) ; c=CHAR(++j)) {}
#define WORD_TOKEN substr(s, substrSel, NSMakeRange(i, j-i))
#define CASE(start, end, function)\
	if(start(c))\
	{\
		WHILE(end)\
		function\
		i = MAX(i,j-1);\
	}
//#define PARSE(start, end, type) CASE(start, end, { CALL_PARSER(type, WORD_TOKEN);})
#define CHARCASE(letter, symbol) case letter: CALL_PARSER(symbol, @"char"); break;

- (AST*) parseString:(NSString*)s
{
	/* Cache some IMPs of methods we call a lot */
	SEL charSel = @selector(characterAtIndex:);
	SEL substrSel = @selector(substringWithRange:);
	CIMP charAt = (CIMP)[s methodForSelector:charSel];
  
	IMP substr = [s methodForSelector:substrSel];
	/* Set up the parser */
	void * parser = ParseAlloc( malloc );

	int line = 1;
	for(unsigned int i=0 ; i<[s length] ; i++)
	{
		unichar c = [s characterAtIndex:i];
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
			else
			{
			CALL_PARSER(WORD, word);
			}
        })
		else CASE(isspace, isspace, if(c == '\n'){line++;})
		else
		{
			switch(c)
			{
				CHARCASE('|', BAR)
				CHARCASE(':', COLON)
				CHARCASE('.', STOP)
				CHARCASE('=', EQ)
				CHARCASE('[', LSQBRACK)
				CHARCASE(']', RSQBRACK)
				CHARCASE('^', RETURN)
				case '+':
					CALL_PARSER(WORD, @"+");
					CALL_PARSER(COLON, @"char");
					break;
				default:
					NSLog(@"Weird character '%c' found on line %d", c,line);
			}
		}
	}
	Parse(parser, 0, nil, self);
	ParseFree(parser, free);
	NSLog(@"Returning delegate %@", delegate);
	return delegate;
}
@end

@implementation Parser
//Method replaced by category
//FIXME: Move the declaration of this into a category interface
- (AST*) parseString:(NSString*)s { return nil; }
@end
