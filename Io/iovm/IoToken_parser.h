/*   Copyright (c) 2003, Steve Dekorte
docLicense("BSD revised")
 *
 *   The Token Parser takes a token stream from the lexer
 *   and organizes it into a parse tree.
 * 
 *   IoMessage will then map the parse tree directly into a Message tree.
 */

#ifndef IOTOKEN_PARSER_DEFINED
#define IOTOKEN_PARSER_DEFINED 1

#include "IoLexer.h"
#include "IoToken.h"

#ifdef __cplusplus
extern "C" {
#endif

// IoLexer Token

// setters 
    
void IoToken_error_(IoToken *self, const char *name);
char *IoToken_error(IoToken *self);
void IoToken_addArg_(IoToken *self, IoToken *arg);

// checks 

int IoToken_hasArg_(IoToken *self, IoToken *t);
IoToken *IoToken_lastOperator(IoToken *self, IoLexer *lexer);
IoToken *IoToken_lastParen(IoToken *self);
IoToken *IoToken_lastNonNextOrParenArg(IoToken *self);
IoToken *IoToken_newFromIoLexer_(IoLexer *lexer, IoToken **error);

// parsing 

IoToken *IoToken_popFromLexer_andSetParent_(IoLexer *lexer, IoToken *parent);
void IoToken_read(IoToken *self, IoLexer *lexer, IoToken **error);

void IoToken_parseArgs(IoToken *self, IoLexer *lexer, IoToken **error);
void IoToken_parseArg(IoToken *self, IoLexer *lexer, IoToken **error);
void IoToken_parseAttached(IoToken *self, IoLexer *lexer, IoToken **error);
void IoToken_parseNext(IoToken *self, IoLexer *lexer, IoToken **error);

// printing 

void IoToken_printParsed(IoToken *self);
void IoToken_printParseStack(IoToken *self);

#ifdef __cplusplus
}
#endif
#endif
