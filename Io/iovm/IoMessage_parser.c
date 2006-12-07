/*
 docCopyright("Steve Dekorte; Jonathan Wright (2006)", 2002)
 docLicense("BSD revised")
 */

#include "IoMessage_parser.h"
#include "IoMessage_opShuffle.h"
#include "IoObject.h"
#include "IoSeq.h"
#include "IoMap.h"
#include "IoNumber.h"
#include "IoState.h"
#include "IoLexer.h"
#include <ctype.h>

#define DATA(self) ((IoMessageData *)IoObject_dataPointer(self))

void IoMessage_parseName(IoMessage *self, IoLexer *lexer);
void IoMessage_parseArgs(IoMessage *self, IoLexer *lexer);
void IoMessage_parseAttached(IoMessage *self, IoLexer *lexer);
void IoMessage_parseNext(IoMessage *self, IoLexer *lexer);
IoMessage *IoMessage_newParseNextMessageChain(void *state, IoLexer *lexer);
void IoMessage_ifPossibleCacheToken_(IoMessage *self, IoToken *p);

void IoMessage_ifPossibleCacheToken_(IoMessage *self, IoToken *p) 
{
	IoSymbol *method = DATA(self)->name;
	IoObject *r = NULL;
	
	switch ((int)IoToken_type(p))
	{
		case TRIQUOTE_TOKEN:
			r =  IoState_symbolWithCString_length_(IOSTATE, IoSeq_asCString(method) + 3, IoSeq_rawSize(method) - 6); 
			break;
			
		case MONOQUOTE_TOKEN:
			r =  IoSeq_rawAsUnescapedSymbol(IoSeq_rawAsUnquotedSymbol(method)); 
			break;
			
		case NUMBER_TOKEN:
			r =  IONUMBER(IoSeq_asDouble(method));
			break;
			
		case HEXNUMBER_TOKEN:
			r =  IONUMBER(IoSeq_rawAsDoubleFromHex(method));
			break;      
			
		default:
			if (IoSeq_rawEqualsCString_(method, "nil"))
			{ 
				r = IONIL(self); 
			}
	}
	
	IoMessage_cachedResult_(self, r);
}

IoMessage *IoMessage_newFromText_label_(void *state, const char *text, const char *label)
{
	IoLexer *lexer = IoLexer_new();
	IoMessage *msg;
	
	IoLexer_string_(lexer, text);
	IoLexer_lex(lexer);
	
	
	msg = IoMessage_newParse(state, lexer);
	
	IoMessage_opShuffle_(msg);
	
	{
		IoSymbol *labelSymbol = IoState_symbolWithCString_((IoState *)state, label);
		IoMessage_label_(msg, labelSymbol); 
	}
	
	IoLexer_free(lexer);
	
	return msg;
}

// -------------------------------

IoMessage *IoMessage_newParse(void *state, IoLexer *lexer)
{
	if (IoLexer_errorToken(lexer))
	{ 
		IoMessage *m; 
		IoSymbol *errorString;
		
        // Maybe the nil message could be used here. Or even a NULL.
        IoSymbol *error = IoState_symbolWithCString_(state, "Error");
		m = IoMessage_newWithName_returnsValue_(state, error, error);
		errorString = IoState_symbolWithCString_((IoState *)state, IoLexer_errorDescription(lexer));
		IoState_error_(state, m, "compile error: %s", CSTRING(errorString));
	}
	
	if (IoLexer_topType(lexer) == TERMINATOR_TOKEN)
	{
		IoLexer_pop(lexer);
	}
	
	if (IoTokenType_isValidMessageName(IoLexer_topType(lexer)))
	{
		IoMessage *self = IoMessage_newParseNextMessageChain(state, lexer);
		
		if (IoLexer_topType(lexer) != NO_TOKEN)
		{
			// TODO: Exception as the end was expected
			IoState_error_(state, self, "compile error: %s", "unused tokens");
		}
		
		return self;
	}
	
	return IoMessage_newWithName_returnsValue_(state,
											   IoState_symbolWithCString_((IoState*)state, "nil"),
											   ((IoState*)state)->ioNil
											   );
}

IoMessage *IoMessage_newParseNextMessageChain(void *state, IoLexer *lexer)
{
	IoMessage *self = IoMessage_new(state);
	
	if (IoTokenType_isValidMessageName(IoLexer_topType(lexer)))
	{
		IoMessage_parseName(self, lexer);
	}
	
	if (IoLexer_topType(lexer) == OPENPAREN_TOKEN)
	{
		IoMessage_parseArgs(self, lexer);
	}
	
	if (IoTokenType_isValidMessageName(IoLexer_topType(lexer)))
	{
		IoMessage_parseNext(self, lexer);
	}
	
	while (IoLexer_topType(lexer) == TERMINATOR_TOKEN)
	{
		IoLexer_pop(lexer);
		
		if (IoTokenType_isValidMessageName(IoLexer_topType(lexer)))
		{
			IoMessage *eol = IoMessage_newWithName_(state, ((IoState*)state)->semicolonSymbol);
			IoMessage_rawSetNext(self, eol);
			IoMessage_parseNext(eol, lexer);
		}
	}
	
	return self;
}

void IoMessage_parseName(IoMessage *self, IoLexer *lexer)
{
	IoToken *token = IoLexer_pop(lexer);
	
	DATA(self)->name = IOREF(IOSYMBOL(IoToken_name(token)));
	
	IoMessage_ifPossibleCacheToken_(self, token);
	IoMessage_rawSetLineNumber_(self, IoToken_lineNumber(token));
	IoMessage_rawSetCharNumber_(self, IoToken_charNumber(token));
}

void IoMessage_parseArgs(IoMessage *self, IoLexer *lexer)
{
	IoLexer_pop(lexer);
	
	if (IoTokenType_isValidMessageName(IoLexer_topType(lexer)))
	{
		IoMessage *arg = IoMessage_newParseNextMessageChain(IOSTATE, lexer);
		IoMessage_addArg_(self, arg);
		
		while (IoLexer_topType(lexer) == COMMA_TOKEN)
		{
			IoLexer_pop(lexer);
			
			if (IoTokenType_isValidMessageName(IoLexer_topType(lexer)))
			{
				IoMessage *arg = IoMessage_newParseNextMessageChain(IOSTATE, lexer);
				IoMessage_addArg_(self, arg);
			}
			// Doesn't actually work because the lexer detects this case and reports the error before we can handle it...
			//else if (IoLexer_topType(lexer) == CLOSEPAREN_TOKEN)
			//{
			//	// Allow the last arg to be empty as in, "foo(a,b,c,)".
			//}
			else
			{
				// TODO: Exception, missing message
			}
		}
	}
	
	if (IoLexer_topType(lexer) != CLOSEPAREN_TOKEN)
	{
		// TODO: Exception, missing close paren
	}
	IoLexer_pop(lexer);
}

void IoMessage_parseNext(IoMessage *self, IoLexer *lexer)
{
	IoMessage *next = IoMessage_newParseNextMessageChain(IOSTATE, lexer);
	IoMessage_rawSetNext(self, next);
}

