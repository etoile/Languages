/*   
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#include "IoToken.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

IoToken *IoToken_new(void)
{
    IoToken *self = (IoToken *)calloc(1, sizeof(IoToken));
    self->name = 0x0;
    self->charNumber = -1;
    
    // parsed 
    
    self->args = List_new();
    return self;
}

void IoToken_free(IoToken *self)
{
    //if (self->nextToken) IoToken_free(self->nextToken);
    if (self->name) free(self->name);
    if (self->error) free(self->error);
    
    // parsed 
    //List_do_(self->args, (ListDoCallback *)IoToken_free);
	
    List_free(self->args);
    /*
	 if (self->attached) IoToken_free(self->attached);
	 if (self->next) IoToken_free(self->next);
	 */
    free(self);
}

const char *IoToken_typeName(IoToken *self)
{ 
    switch (self->type)
    {
		case NO_TOKEN:         return "NoToken"; 
		case OPENPAREN_TOKEN:  return "OpenParen"; 
		case COMMA_TOKEN:      return "Comma"; 
		case CLOSEPAREN_TOKEN: return "CloseParen"; 
		case MONOQUOTE_TOKEN:  return "MonoQuote"; 
		case TRIQUOTE_TOKEN:   return "TriQuote"; 
		case OPERATOR_TOKEN:   return "Operator"; 
		case IDENTIFIER_TOKEN: return "Identifier"; 
		case TERMINATOR_TOKEN: return "Terminator"; 
		case COMMENT_TOKEN:    return "Comment";
		case NUMBER_TOKEN:     return "Number"; 
		case HEXNUMBER_TOKEN:  return "HexNumber";
    }
    return "UNKNOWN_TOKEN";
}

void IoToken_name_length_(IoToken *self, const char *name, size_t len)
{ 
    self->name = strncpy(realloc(self->name, len + 1), name, len);
    self->name[len] = (char)0x0;
    self->length = len;
}

void IoToken_name_(IoToken *self, const char *name)
{ 
    self->name = strcpy((char *)realloc(self->name, strlen(name) + 1), name);
    self->length = strlen(name);
}

char *IoToken_name(IoToken *self) 
{ 
    return self->name ? self->name : (char *)""; 
}

int IoToken_nameIs_(IoToken *self, const char *name)
{ 
    if (strlen(self->name) == 0 && strlen(name) != 0) 
    {
		return 0;
    }
    //return !strncmp(self->name, name, self->length);
    return !strcmp(self->name, name);
}

IoTokenType IoToken_type(IoToken *self) 
{ 
    return self->type; 
}

int IoToken_lineNumber(IoToken *self) 
{ 
    return self->lineNumber; 
}

int IoToken_charNumber(IoToken *self) 
{ 
    return self->charNumber; 
}

void IoToken_quoteName_(IoToken *self, const char *name)
{ 
    char *old = self->name;
    size_t length = strlen(name) + 3;
    self->name = malloc(length); 
    snprintf(self->name, length, "\"%s\"", name);
    
    if (old) 
    {
	    free(old);
	}
}

void IoToken_type_(IoToken *self, IoTokenType type)
{ 
    self->type = type; 
}

void IoToken_nextToken_(IoToken *self, IoToken *nextToken)
{
    if (self == nextToken) 
    { 
		printf("next == self!\n"); 
		exit(1); 
    }
    
    if (self->nextToken) 
    {
		IoToken_free(self->nextToken);
    }
    
    self->nextToken = nextToken;
}

void IoToken_print(IoToken *self)
{
    IoToken_printSelf(self);
    /*
     if (self->nextToken) 
     {
		 printf(", ");
		 IoToken_print(self->nextToken);
     }
     */
}

void IoToken_printSelf(IoToken *self)
{
    size_t i;
    printf("'");
    
    for (i = 0; i < self->length; i ++) 
    {
		putchar(self->name[i]);
    }
    
    printf("' ");
}

int IoTokenType_isValidMessageName(IoTokenType self)
{
	switch (self)
	{
		case IDENTIFIER_TOKEN:
		case OPERATOR_TOKEN:
		case MONOQUOTE_TOKEN:
		case TRIQUOTE_TOKEN:
		case NUMBER_TOKEN:
		case HEXNUMBER_TOKEN:
			return 1;
		default:
			return 0;
	}
	return 0;
}

/*
 void IoToken_valueFromState_(IoToken *self, IoState *state)
 {   
	 IoSeq *name = IoSeq_newWithData_length_(self->name, self->length);
	 
	 switch ((int)IoToken_type(self))
	 {
		 case TRIQUOTE_TOKEN:
			 return IoState_symbolWithCString_length_(state, 
													  IoSeq_asCString(name) + 3, 
													  IoSeq_rawSize(name) - 6); 
			 
		 case MONOQUOTE_TOKEN:
			 return IoSeq_rawAsUnescapedSymbol(IoSeq_rawAsUnquotedSymbol(name)); 
			 
		 case NUMBER_TOKEN:
			 return IONUMBER(IoSeq_asDouble(name));
			 
		 case HEXNUMBER_TOKEN:
			 return IONUMBER(IoSeq_rawAsDoubleFromHex(name));
			 
	 }
	 
	 return state->ioNil; 
 }
 */
