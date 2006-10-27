/*
 docCopyright("Steve Dekorte", 2002)
 docLicense("BSD revised")
 */

#include "IoMessage_parser.h"
#include "IoObject.h"
#include "IoSeq.h"
#include "IoMap.h"
#include "IoNumber.h"
#include "IoState.h"
#include "IoLexer.h"
#include "IoToken_parser.h"
#include <ctype.h>

#define DATA(self) ((IoMessageData *)IoObject_dataPointer(self))

void IoMessage_ifPossibleCacheToken_(IoMessage *self, IoToken *p) 
{
	IoSymbol *method = DATA(self)->name;
	
	switch ((int)IoToken_type(p))
	{
		case TRIQUOTE_TOKEN:
			IoMessage_cachedResult_(self, IoState_symbolWithCString_length_(IOSTATE, 
															    IoSeq_asCString(method) + 3, 
															    IoSeq_rawSize(method) - 6)); 
			break;
			
		case MONOQUOTE_TOKEN:
			IoMessage_cachedResult_(self, 
							    IoSeq_rawAsUnescapedSymbol(IoSeq_rawAsUnquotedSymbol(method))); 
			break;
			
		case NUMBER_TOKEN:
			IoMessage_cachedResult_(self, IONUMBER(IoSeq_asDouble(method)));
			break;
			
		case HEXNUMBER_TOKEN:
			IoMessage_cachedResult_(self, IONUMBER(IoSeq_rawAsDoubleFromHex(method)));
			break;      
			
		default:
			if (IoSeq_rawEqualsCString_(method, "Nil"))
			{ 
				IoMessage_cachedResult_(self, IONIL(self)); 
			}
	}
}

IoMessage *IoMessage_newFromIoToken_(void *state, IoToken *p) 
{
	IoSymbol *s = IoState_symbolWithCString_((IoState *)state, IoToken_name(p));
	IoMessage *m = IoMessage_newWithName_(state, s);
	int i;
	
	IoMessage_ifPossibleCacheToken_(m, p);
	
	for (i = 0; i < List_size(p->args); i ++)
	{
		IoToken *parg = List_at_(p->args, i);
		IoMessage_addArg_(m, IoMessage_newFromIoToken_(state, parg));
	}
	
	if (p->attached) 
	{
		DATA(m)->attachedMessage = IoMessage_newFromIoToken_(state, p->attached);
	}
	
	if (p->next) 
	{
		DATA(m)->nextMessage = IoMessage_newFromIoToken_(state, p->next);
	}
	
	DATA(m)->lineNumber = IoToken_lineNumber(p);
	IoMessage_rawSetCharNumber_(m, IoToken_charNumber(p));
	
	return m;
}



IoMessage *IoMessage_newFromText_label_(void *state, const char *text, const char *label)
{
	IoLexer *lexer = IoLexer_new();
	IoMessage *msg;
	
	IoLexer_string_(lexer, text);
	IoLexer_lex(lexer);
	
	
	msg = IoMessage_newParse(state, lexer);
	
	/*
	if(lexer->errorToken)
	{
		IoToken *errorToken = lexer->errorToken;
		IoLexer_free(lexer);
		IoState_error_(state, 0x0, "compile error: %s %i", errorToken->error, errorToken->lineNumber);
	}
	*/
	
	//puts("Pre shuffle");
	//IoMessage_deepDump(msg);
	//puts("\n");
	
	IoMessage_opShuffle_(msg);
	
	//puts("Post shuffle");
	//IoMessage_deepDump(msg);
	//puts("\n");
	
	{
		IoSymbol *labelSymbol = IoState_symbolWithCString_((IoState *)state, label);
		IoMessage_label_(msg, labelSymbol); 
	}
	
	IoLexer_free(lexer);
	
	//{
	//	IoMessage *old = IoMessage_newFromText_label_old(state, text, label);
	//	puts("Old Parse");
	//	IoMessage_deepDump(old);
	//	puts("\n");
	//}
		
	return msg;
}

// -------------------------------

IoMessage *IoMessage_newFromIoToken_(void *state, IoToken *p);
int IoToken_LevelForOp_(char *s);

IoMessage *IoMessage_newParseAttachedMessgeChain(void *state, IoLexer *lexer);
IoMessage *IoMessage_newParseNextMessageChain(void *state, IoLexer *lexer);

IoMessage *IoMessage_newParse(void *state, IoLexer *lexer)
{
	if (IoLexer_errorToken(lexer))
	{ 
		IoMessage *m; 
		IoSymbol *errorString;

		m = IoMessage_newFromIoToken_(state, IoLexer_errorToken(lexer)); 
		IoMessage_label_(m, IoState_symbolWithCString_((IoState *)state, "[lexer]"));
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

	return IoMessage_newWithName_returnsValue_(
		state,
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
		IoMessage_parseAttached(self, lexer);
	}

	while (IoLexer_topType(lexer) == TERMINATOR_TOKEN)
	{
		IoLexer_pop(lexer);

		if (IoTokenType_isValidMessageName(IoLexer_topType(lexer)))
		{
			IoMessage_parseNext(self, lexer);
		}
	}

	return self;
}

IoMessage *IoMessage_newParseAttachedMessgeChain(void *state, IoLexer *lexer)
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
		IoMessage_parseAttached(self, lexer);
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

void IoMessage_parseAttached(IoMessage *self, IoLexer *lexer)
{
	IoMessage *attached = IoMessage_newParseAttachedMessgeChain(IOSTATE, lexer);
	IoMessage_rawSetAttachedMessage(self, attached);
}

void IoMessage_parseNext(IoMessage *self, IoLexer *lexer)
{
	IoMessage *next = IoMessage_newParseNextMessageChain(IOSTATE, lexer);
	IoMessage_rawSetNextMessage(self, next);
}


// Op shuffling

// TODO move the IO_OP_MAX_LEVEL define to be adjacent to the op table.
#define IO_OP_MAX_LEVEL 17

enum LevelType {ATTACH, ARG, NEW, UNUSED};

typedef struct {
	IoMessage *message;
	enum LevelType type;
	int precedence;
} Level;

void Level_finish(Level *self);
void Level_attachAndReplace(Level *self, IoMessage *msg);
void Level_setAwaitingFirstArg(Level *self, IoMessage *msg, int precedence);

typedef struct {
	Level pool[IO_OP_MAX_LEVEL];
	int currentLevel;

	List *stack;
        IoMap *operatorTable;
} Levels;

void Levels_reset(Levels *self)
{
	int i;
	self->currentLevel = 1;

	for (i=0; i < IO_OP_MAX_LEVEL; i++)
	{
		Level *level = &self->pool[i];
		level->type = UNUSED;
	}

	{
		Level *level = &self->pool[0];
		level->message = 0x0;
		level->type = NEW;
		level->precedence = IO_OP_MAX_LEVEL;
	}

	List_removeAll(self->stack);
	List_append_(self->stack, &self->pool[0]);
}


Levels *Levels_new(IoMessage *msg)
{
	Levels *self = calloc(1, sizeof(Levels));
        IoObject *opTable = IoObject_rawGetSlot_(msg, IoState_symbolWithCString_(msg->state, "OperatorTable"));
        self->operatorTable = 0;
        if (opTable)
        {
                IoObject *rightOperators = IoObject_rawGetSlot_(opTable, IoState_symbolWithCString_(msg->state, "rightOperators"));
                if (rightOperators)
                {
                        if (ISMAP(rightOperators))
                        {
                                self->operatorTable = rightOperators;
                        }
                        else
                        {
                                IoState_error_(msg->state, 0x0, "compile error: Message OperatorTable rightOperators is not a Map. %p %s", rightOperators, CSTRING(IoMessage_name(msg)));
                        }
                }
        }
	self->stack = List_new();
	Levels_reset(self);
	return self;
}

void Levels_free(Levels *self)
{
	free(self);
}

inline Level *Levels_currentLevel(Levels *self)
{
	return List_top(self->stack);
}

void Levels_popDownTo(Levels *self, int targetLevel)
{
	Level *level;
	
	while (level = List_top(self->stack), level->precedence <= targetLevel && level->type != ARG)
	{
		Level_finish(List_pop(self->stack));
		self->currentLevel--;
	}
}

void Levels_attachToTopAndPush(Levels *self, IoMessage *msg, int precedence)
{
	Level *level = NULL;
	{
		Level *top = List_top(self->stack);
		Level_attachAndReplace(top, msg);
	}

	{
		// TODO: Check for overflow of the pool.
		if (self->currentLevel >= IO_OP_MAX_LEVEL)
		{
			IoState_error_(msg->state, 0x0, "compile error: Overflowed operator stack. Only %d levels of operators currently supported.", IO_OP_MAX_LEVEL-1);
		}
		level = &self->pool[self->currentLevel++];
		Level_setAwaitingFirstArg(level, msg, precedence);
		List_append_(self->stack, level);
	}
}

void Level_finish(Level *self)
{
	if (self->message)
	{
		IoMessage_rawSetAttachedMessage(self->message, 0x0);
	}
	self->type = UNUSED;
}

void Level_attach(Level *self, IoMessage *msg)
{
	switch (self->type)
	{
		case ATTACH:
			IoMessage_rawSetAttachedMessage(self->message, msg);
			break;

		case ARG:
			IoMessage_addArg_(self->message, msg);
			break;

		case NEW:
			self->message = msg;
			break;

		case UNUSED:
			break;
	}
}

void Level_attachAndReplace(Level *self, IoMessage *msg)
{
	Level_attach(self, msg);
	self->type = ATTACH;
	self->message = msg;
}

void Level_setAwaitingFirstArg(Level *self, IoMessage *msg, int precedence)
{
	self->type = ARG;
	self->message = msg;
	self->precedence = precedence;
}

void Level_setAlreadyHasArgs(Level *self, IoMessage *msg)
{
	self->type = ATTACH;
	self->message = msg;
}

int Levels_levelForOp(Levels *self, char *messageName, IoSymbol *messageSymbol, IoMessage *msg)
{
        if (self->operatorTable)
        {
                IoObject *value = IoMap_rawAt(self->operatorTable, messageSymbol);
                if (!value)
                {
                        return -1;
                }
                if (ISNUMBER(value))
                {
                        return IoNumber_asInt((IoNumber*)value);
                }
                else
                {
			IoState_error_(msg->state, msg, "compile error: Value for '%s' in Message OperatorTable is not a number. Values in the OperatorTable are numbers which indicate the precedence of the operator.", messageName);
                        return -1; // To keep the compiler happy.
                }
        }
        else
        {
                return IoToken_LevelForOp_(messageName);
        }
}

void Levels_attach(Levels *self, IoMessage *msg, List *expressions)
{
	// TODO clean up this method.
	
        IoSymbol *messageSymbol = IoMessage_name(msg);
	char *messageName = CSTRING(messageSymbol);
	int precedence = Levels_levelForOp(self, messageName, messageSymbol, msg);

	if (strcmp(messageName, ":=") == 0 || strcmp(messageName, "=") == 0)
	{
		IoState *state = (IoState *)(msg->state);
		Level *currentLevel = Levels_currentLevel(self);
		IoMessage *attaching = currentLevel->message;
		IoSymbol *setSlotName;

		if (attaching == 0x0)
		{
			// Could be handled as, message(:= 42) -> setSlot(nil, 42)

			IoState_error_(state, msg, "compile error: %s requires a symbol to its left.", messageName);
		}

		if (IoMessage_argCount(attaching) > 0)
		{
			IoState_error_(state, msg, "compile error: The symbol to the left of %s cannot have arguments.", messageName);
		}


		{
			IoSymbol *slotName = DATA(attaching)->name;
			IoSymbol *quotedSlotName = IoSeq_newSymbolWithFormat_(state, "\"%s\"", CSTRING(slotName));
			IoMessage *slotNameMessage = IoMessage_newWithName_returnsValue_(state, quotedSlotName, slotName);
			IoMessage_addArg_(attaching, slotNameMessage);

			if (strcmp(messageName, ":=") == 0)
			{
				if (isupper(CSTRING(slotName)[0]))
				{
					setSlotName = state->setSlotWithTypeSymbol;
				}
				else
				{
					setSlotName = state->setSlotSymbol;
				}
			}
			else
			{
				setSlotName = state->updateSlotSymbol;
			}
		}

		
		DATA(attaching)->name = IoObject_addingRef_(attaching, setSlotName);

		currentLevel->type = ATTACH;

		if (IoMessage_argCount(msg) > 0)
		{
			IoMessage *foo = IoMessage_newWithName_(state, IoState_symbolWithCString_(state, ""));
			IoMessage *arg = IoMessage_rawArgAt_(msg, 0);

			IoMessage_addArg_(foo, arg);
			IoMessage_addArg_(attaching, foo);

			IoMessage_rawSetAttachedMessage(foo, DATA(msg)->attachedMessage);
		}
		else
		{
			if (DATA(msg)->attachedMessage == 0x0)
			{
				IoState_error_(state, msg, "compile error: %s must be followed by a value.", messageName);
			}

			IoMessage_addArg_(attaching, DATA(msg)->attachedMessage);
		}

		List_push_(expressions, DATA(msg)->attachedMessage);

		IoMessage_rawSetAttachedMessage(attaching, 0x0);
		IoMessage_cachedResult_(attaching, 0x0);
		IoMessage_rawSetAttachedMessage(msg, 0x0);
	}
	else if (precedence != -1)
	{
		if (IoMessage_argCount(msg) > 0)
		{
			Level_setAlreadyHasArgs(Levels_currentLevel(self), msg);
		}
		else
		{
			Levels_popDownTo(self, precedence);
			Levels_attachToTopAndPush(self, msg, precedence);
		}
	}
	else
	{
		Level_attachAndReplace(Levels_currentLevel(self), msg);
	}
}

void Levels_nextMessage(Levels *self)
{
	Level *level;
	while ((level = List_pop(self->stack)))
	{
		Level_finish(level);
	}

	Levels_reset(self);
}

void IoMessage_opShuffle_(IoMessage *self)
{
        if (IoObject_rawGetSlot_(self, IOSTATE->opShuffleSymbol) && IoMessage_name(self) != IOSTATE->noShufflingSymbol)
        {
                IoMessage_locals_performOn_(IOSTATE->opShuffleMessage, IOSTATE->lobby, self);
        }
}

IoMessage *IoMessage_opShuffle(IoMessage *self, IoObject *locals, IoMessage *m)
{
	Levels *levels = Levels_new(self);
	List *expressions = List_new();
	
	List_push_(expressions, self);
	
	while (List_size(expressions) >= 1)
	{
		IoMessage *n = List_pop(expressions);
		for (; n; n = DATA(n)->nextMessage)
		{
			IoMessage *a = n;
			for (; a; a = DATA(a)->attachedMessage)
			{
				Levels_attach(levels, a, expressions);
				List_appendSeq_(expressions, DATA(a)->args);
			}
			Levels_nextMessage(levels);
		}
	}
	Levels_free(levels);
	return self;
}

