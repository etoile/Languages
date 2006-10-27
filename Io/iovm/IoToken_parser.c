/*   Copyright (c) 2003, Steve Dekorte
*   All rights reserved. See _BSDLicense.txt.
*/

#include "IoLexer.h"
#include "IoToken_parser.h"
#include "Stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
		
	{"^",   1},
		
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
		
	{"|",   9},
		
	{"and", 10},
	{"&&",  10},
		
	{"or",  11},
	{"||",  11},
		
	{"..",  12},
		
	{"=",   13},
	{"+=",  13},
	{"-=",  13},
	{"*=",  13},
	{"/=",  13},
	{"%=",  13},
	{"&=",  13},
	{"^=",  13},
	{"|=",  13},
	{"<<=", 13},
	{">>=", 13},
	{":=",  13},
	{"<-",  13},
	{"<->", 13},
	{"->",  13},
		
	{"return",	14},
		
	{",", 15},
	{0x0, 0},
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

int IoToken_shouldBreakFor_(IoToken *self, IoToken *m)
{ 
	int i1 = IoToken_LevelForOp_(IOTOKEN_NAME(self));
	int i2 = IoToken_LevelForOp_(IOTOKEN_NAME(m));
	int r = !(i1 < i2);
	
	if (i1 < 0 || i2 < 0) 
	{
		return 0;
	}
	/*printf("IoToken_shouldBreakFor_('%s', '%s') = %i\n", 
		IoToken_name(self), IoToken_name(m), r);*/
	return r;  
}


// parsing ---------------------------------------------------------- 

IoToken *IoToken_popFromLexer_andSetParent_(IoLexer *lexer, IoToken *parent)
{
	IoTokenType t;
	
	if (!IoLexer_top(lexer)) 
	{
		return 0x0;
	}
	
	t = IoLexer_topType(lexer);
	if (t == COMMA_TOKEN || 
	    t == TERMINATOR_TOKEN || 
	    t == OPENPAREN_TOKEN ||
	    t == CLOSEPAREN_TOKEN ||
	    t == COMMENT_TOKEN )
	{ 
		return 0x0; 
	}
	
	{
		IoToken *p = IoLexer_pop(lexer);
		p->parent = parent;
		return p;
	}
}

IoToken *IoToken_root(IoToken *self)
{
	if (!self->parent) 
	{
		return self;
	}
	
	return IoToken_root(self->parent);
}

void IoToken_error_(IoToken *self, const char *error)
{ 
	self->error = strcpy((char *)realloc(self->error, strlen(error) + 1), error); 
}

char *IoToken_error(IoToken *self) 
{ 
	return self->error ? self->error : (char *)""; 
}

// checks ----------------------------------------------------

int IoToken_hasArg_(IoToken *self, IoToken *t)   
{ 
	return (List_indexOf_(self->args, t) != -1); 
} 

int IoToken_isOperator(IoToken *self)
{
	return (self->type == OPERATOR_TOKEN) ||
	(IoToken_LevelForOp_(IOTOKEN_NAME(self)) != -1);
}

IoToken *IoToken_lastOperator(IoToken *self, IoLexer *lexer)
{
	if (!self) 
	{
		return 0x0;
	}
	
	if (IoToken_isOperator(self))
	{
		return self;
	}
	
	if (self->parent && !self->parent->openParen)
	{
		if (IoToken_hasArg_(self->parent, self) || self->parent->attached == self)
		{
			return IoToken_lastOperator(self->parent, lexer); 
		}
	}
	return 0x0;
}

IoToken *IoToken_lastOperatorToBreakOn(IoToken *self, IoLexer *lexer)
{
	IoToken *op = IoToken_lastOperator(self->parent, lexer);
	
	while (op && IoToken_shouldBreakFor_(self, op))
	{
		IoToken *nextOp = IoToken_lastOperator(op->parent, lexer);
		
		if (!nextOp || !IoToken_shouldBreakFor_(self, nextOp)) 
		{ 
			return op; 
		}
		
		op = nextOp;
	}
	
	return 0x0;
}

IoToken *IoToken_lastParen(IoToken *self)
{
	if (!self) 
	{
		return 0x0;
	}
	
	if (self->parent)
	{
		if (self->openParen) 
		{
			return self;
		}
		
		if (self->parent->next != self)
		{
			return IoToken_lastParen(self->parent); 
		}
	}
	
	return 0x0;
}

// parsing --------------------------------------------

void IoToken_read(IoToken *self, IoLexer *lexer, IoToken **error)
{
	/*printf("parsing token '%s' parent '%s'\n", self->name, self->parent ? self->parent->name : "NULL");*/
	IoToken_parseArgs(self, lexer, error); 
	
	if (IoToken_nameIs_(self, "=" ) || 
	    IoToken_nameIs_(self, ":="))
	{
		/* 
		special syntax for assignment
		change 'a =(1)' to 'setSlot("a", 1)' 
		*/
		
		int isEqual = IoToken_nameIs_(self, "=");
		/*int isColonEqual = IoToken_nameIs_(self, ":=");*/
		
		if (!List_size(self->args))
		{ 
			IoToken_error_(self, "no tokens right of ="); 
			*error = self; 
			return; 
		}
		
		if ((!self->parent) || self->parent->attached != self)
		{ 
			IoToken_error_(self, "no tokens left of ="); 
			*error = self; 
			return; 
		}
		
		/*printf("%s=\n", IoToken_name(self->parent));*/
		IoToken_quoteName_(self, IOTOKEN_NAME(self->parent));
		self->type = MONOQUOTE_TOKEN;
		
		if (isEqual) 
		{ 
			IoToken_name_(self->parent, "updateSlot"); 
		} 
		else 
		{ 
			IoToken_name_(self->parent, "setSlot"); 
		}
		
		self->parent->type = IDENTIFIER_TOKEN;
		self->parent->attached = 0x0;
		List_append_(self->parent->args, self);
		List_appendSeq_(self->parent->args, self->args);
		List_removeAll(self->args);
		/*self = self->parent->args->items[1];*/ /* Tobias Peters' fix for a := (b) + c */
	}
	
	// handle operator precedence
	
	else if (	IoToken_isOperator(self) && 
			!self->openParen && 
			self->parent && 
			self->parent->attached == self)
	{
		IoToken *lop = IoToken_lastOperatorToBreakOn(self, lexer);
		
		if (lop && lop != self->parent)
		{
			if (lop->attached)
			{ 
				self->attached = lop->attached; 
			}
			
			/*printf("attaching %s (parent %s) to %s (parent %s)\n", self->name, self->parent->name, lop->name, lop->parent->name);*/
			lop->attached = self;
			self->parent->attached = 0x0;
			/*printf("after op reorg: "); IoToken_printParsed(IoToken_root(self)); printf("\n");*/
			return;
		}
	}
	
	if (*error) 
	{
		return;
	}
	
	if (!self->attached)
	{
		IoToken_parseAttached(self, lexer, error);
		
		if (*error) 
		{
			return;
		}
	}
	
	if (!self->parent || // is root token 
		self->parent->next == self || 
		(IoToken_hasArg_(self->parent, self) && 
		 self->parent->openParen))
{ 
		/*printf("next stack: "); IoToken_printParseStack(self); printf("\n");*/
		IoToken_parseNext(self, lexer, error); 
}
}

void IoToken_parseArgs(IoToken *self, IoLexer *lexer, IoToken **error)
{
	if (!IoLexer_top(lexer)) 
	{ 
		IoToken_error_(self, "EOF"); 
		*error = self; 
		return; 
	}
	
	if (IoLexer_topType(lexer) == TERMINATOR_TOKEN) 
	{ 
		return; 
	}
	
	if (IoLexer_topType(lexer) == OPENPAREN_TOKEN) 
	{
		IoLexer_pop(lexer);
		self->openParen = 1;
		
		for (;;) 
		{
			IoToken_parseArg(self, lexer, error); 
			if (*error) return;
			
			if (IoLexer_topType(lexer) == CLOSEPAREN_TOKEN) 
			{ 
				IoLexer_pop(lexer); 
				return; 
			}
			
			if (IoLexer_topType(lexer) == COMMA_TOKEN) 
			{ 
				IoLexer_pop(lexer); 
				continue; 
			}
			
			IoToken_error_(self, "unable to properly parse args in ()s"); 
			return;
		}
	}
	else if (IoToken_isOperator(self))
	{
		// binary message - need to check precedence first 
		IoToken_parseArg(self, lexer, error);
	}
}

void IoToken_parseArg(IoToken *self, IoLexer *lexer, IoToken **error)
{
	IoToken *p = IoToken_popFromLexer_andSetParent_(lexer, self);
	
	if (p)
	{
		List_append_(self->args, p);
		IoToken_read(p, lexer, error);
	}
}

void IoToken_parseAttached(IoToken *self, IoLexer *lexer, IoToken **error)
{
	self->attached = IoToken_popFromLexer_andSetParent_(lexer, self);
	
	if (self->attached) 
	{
		IoToken_read(self->attached, lexer, error);
	}
}

void IoToken_parseNext(IoToken *self, IoLexer *lexer, IoToken **error)
{
	if (IoLexer_topType(lexer) != TERMINATOR_TOKEN)  
	{ 
		return; 
	}
	
	IoLexer_pop(lexer);
	
	if (    IoLexer_topType(lexer) == COMMA_TOKEN || 
		   IoLexer_topType(lexer) == CLOSEPAREN_TOKEN) 
	{ 
		return; 
	}
	
	self->next = IoToken_popFromLexer_andSetParent_(lexer, self);
	
	if (self->next) 
	{
		IoToken_read(self->next, lexer, error);
	}
}

// ------------------------------------------------------- 

void IoToken_printParsed(IoToken *self)
{
	/*printf("'%s'", self->name);*/
	printf("%s", IoToken_name(self));
	
	if (List_size(self->args))
	{		
		printf("(");
		
		LIST_FOREACH(self->args, i, p,
			IoToken_printParsed((IoToken *)p);
			
			if (i < List_size(self->args) - 1)  
			{
				printf(", ");
			}
		);
		
		printf(")");
	}
	
	if (self->attached)
	{
		printf(" ");
		IoToken_printParsed(self->attached);
	}
	
	if (self->next)
	{
		printf(";\n");
		
		if (IoToken_nameIs_(self, "setSlot")) 
		{
			printf("\n");
		}
		
		IoToken_printParsed(self->next);
	}
}

void IoToken_printParseStack(IoToken *self)
{
	if (!self->parent) 
	{ 
		printf("%s", IoToken_name(self)); 
		return; 
	}
	
	IoToken_printParseStack(self->parent);
	printf(" -> %s", IoToken_name(self));
}
