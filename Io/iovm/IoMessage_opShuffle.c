/*
 docCopyright("Jonathan Wright; Steve Dekorte (2002)", 2006)
 docLicense("BSD revised")
 */

#include "IoMessage_opShuffle.h"
#include "IoMap.h"
#include "IoNumber.h"
#include <ctype.h>

#define DATA(self) ((IoMessageData *)IoObject_dataPointer(self))

typedef struct IoOpTable {
	char *opString;
	int level;
} IoOpTable;

int IoToken_LevelForOp_(char *s)
{
	IoOpTable ops[] = { 
	{"@",   0},
	{"@@",  0},
	{"'",   0},
	{".",   0},
	{"?",   0},
	{"(",   0},
	{")",   0},
		
	{"**",  1},
		
	{"++",  2},
	{"--",  2},
		
	{"*",   3},
	{"/",   3},
	{"%",   3},
		
	{"+",   4},
	{"-",   4},
		
	{"<<",  5},
	{">>",  5},
		
	{">",   6},
	{"<",   6},
	{"<=",  6},
	{">=",  6},
		
	{"==",  7},
	{"!=",  7},
		
	{"&",   8},
		
	{"^",   9},
		
	{"|",   10},
		
	{"and", 11},
	{"&&",  11},
		
	{"or",  12},
	{"||",  12},
		
	{"..",  13},
		
	{"=",   14},
	{"+=",  14},
	{"-=",  14},
	{"*=",  14},
	{"/=",  14},
	{"%=",  14},
	{"&=",  14},
	{"^=",  14},
	{"|=",  14},
	{"<<=", 14},
	{">>=", 14},
	{":=",  14},
	{"<-",  14},
	{"<->", 14},
	{"->",  14},
		
	{"return", 15},
		
	{",", 16},
	{NULL, 0},
	};
	
	IoOpTable *entry = ops;
	
	while (entry->opString) 
	{
		if (strcmp(entry->opString, s) == 0) 
		{
			return entry->level;
		}
		entry ++;
	}
	
	return -1;
}

// TODO move the IO_OP_MAX_LEVEL define to be adjacent to the op table.
// TODO Set IO_OP_MAX_LEVEL to max-1 or something massive otherwise people playing with custom operators don't have many levels available
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
	
	for (i = 0; i < IO_OP_MAX_LEVEL; i ++)
	{
		Level *level = &self->pool[i];
		level->type = UNUSED;
	}
	
	{
		Level *level = &self->pool[0];
		level->message = NULL;
		level->type = NEW;
		level->precedence = IO_OP_MAX_LEVEL;
	}
	
	List_removeAll(self->stack);
	List_append_(self->stack, &self->pool[0]);
}

// --- Levels ----------------------------------------------------------

Levels *Levels_new(IoMessage *msg)
{
	Levels *self = calloc(1, sizeof(Levels));
	IoObject *opTable = IoObject_rawGetSlot_(msg, IoState_symbolWithCString_(msg->state, "OperatorTable"));
	
	self->operatorTable = NULL;
	
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
				IoState_error_(msg->state, NULL, "compile error: Message OperatorTable rightOperators is not a Map. %p %s", rightOperators, CSTRING(IoMessage_name(msg)));
			}
		}
	}
	
	self->stack = List_new();
	Levels_reset(self);
	return self;
}

void Levels_free(Levels *self)
{
        List_free(self->stack);
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
			IoState_error_(msg->state, NULL, "compile error: Overflowed operator stack. Only %d levels of operators currently supported.", IO_OP_MAX_LEVEL-1);
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
		IoMessage_rawSetNext(self->message, NULL);
	}
	
	self->type = UNUSED;
}

void Level_attach(Level *self, IoMessage *msg)
{
	switch (self->type)
	{
		case ATTACH:
			IoMessage_rawSetNext(self->message, msg);
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
	
	// `o a := b c ; d`  becomes  `o setSlot("a", b c) ; d`
	// 
	// a      attaching
	// :=     msg
	// b c    msg->next
	
	if (strcmp(messageName, ":=") == 0 || strcmp(messageName, "=") == 0)
	{
		IoState *state = (IoState *)(msg->state);
		Level *currentLevel = Levels_currentLevel(self);
		IoMessage *attaching = currentLevel->message;
		IoSymbol *setSlotName;
		
		if (attaching == NULL) // `:= b ;`
		{
			// Could be handled as, message(:= 42) -> setSlot(nil, 42)
			
			IoState_error_(state, msg, "compile error: %s requires a symbol to its left.", messageName);
		}
		
		if (IoMessage_argCount(attaching) > 0) // `a(1,2,3) := b ;`
		{
			IoState_error_(state, msg, "compile error: The symbol to the left of %s cannot have arguments.", messageName);
		}
		
		
		{
			// `a := b ;`
			IoSymbol *slotName = DATA(attaching)->name;
			IoSymbol *quotedSlotName = IoSeq_newSymbolWithFormat_(state, "\"%s\"", CSTRING(slotName));
			IoMessage *slotNameMessage = IoMessage_newWithName_returnsValue_(state, quotedSlotName, slotName);
			
			// `a := b ;`  ->  `a("a") := b ;`
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
		
		// `a("a") := b ;`  ->  `setSlot("a") := b ;`
		DATA(attaching)->name = IoObject_addingRef_(attaching, setSlotName);
		
		currentLevel->type = ATTACH;
		
		if (IoMessage_argCount(msg) > 0) // `setSlot("a") :=(b c) d e ;`
		{
			// `b c`
			IoMessage *arg = IoMessage_rawArgAt_(msg, 0);
			
                        if (DATA(msg)->next == NULL || IoMessage_isEOL(DATA(msg)->next))
                        {
                                IoMessage_addArg_(attaching, arg);
                        }
                        else
                        {
                                // `()`
                                IoMessage *foo = IoMessage_newWithName_(state, IoState_symbolWithCString_(state, ""));
                                
                                // `()`  ->  `(b c)`
                                IoMessage_addArg_(foo, arg);
                                
                                // `(b c)`  ->  `(b c) d e ;`
                                IoMessage_rawSetNext(foo, DATA(msg)->next);
                                
                                // `setSlot("a") :=(b c) d e ;`  ->  `setSlot("a", (b c) d e ;) :=(b c) d e ;`
                                IoMessage_addArg_(attaching, foo);
                        }
		}
		else // `setSlot("a") := b ;`
		{
			// `setSlot("a") :=` or `setSlot("a") := ;`
			IoMessage *mn = DATA(msg)->next;
			IoSymbol *name = mn ? DATA(mn)->name : NULL;
			IoSymbol *semi = ((IoState *)(msg->tag->state))->semicolonSymbol;
			
			//if (mn == NULL || IoMessage_isEOL(mn))
			if (mn == NULL || name == semi)
			{
				IoState_error_(state, msg, "compile error: %s must be followed by a value.", messageName);
			}
			
			// `setSlot("a") := b c ;`  ->  `setSlot("a", b c ;) := b c ;`
			IoMessage_addArg_(attaching, DATA(msg)->next);
		}
		
		// process the value (`b c d`) later  (`setSlot("a", b c d) := b c d ;`)
		if (DATA(msg)->next != NULL && !IoMessage_isEOL(DATA(msg)->next))
		{
			List_push_(expressions, DATA(msg)->next);
		}
		
		{
			IoMessage *last = msg;
			while (DATA(last)->next != NULL && !IoMessage_isEOL(DATA(last)->next))
			{
				last = DATA(last)->next;
			}

			IoMessage_rawSetNext(attaching, DATA(last)->next);
			
			// Continue processing in IoMessage_opShuffle loop
			IoMessage_rawSetNext(msg, DATA(last)->next);
			
                        if (last != msg)
                        {
                                IoMessage_rawSetNext(last, NULL);
                        }
		}
		
		// make sure b in `1 := b` gets executed
		IoMessage_cachedResult_(attaching, NULL);
	}
	else if (IoMessage_isEOL(msg))
	{
		Levels_popDownTo(self, IO_OP_MAX_LEVEL-1);
		Level_attachAndReplace(Levels_currentLevel(self), msg);
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
		
		do
		{
			Levels_attach(levels, n, expressions);
			List_appendSeq_(expressions, DATA(n)->args);
		} while ((n = DATA(n)->next));
		
		Levels_nextMessage(levels);
	}

    List_free(expressions);
	Levels_free(levels);

	return self;
}

