/*
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef IOMESSAGE_PARSER_DEFINED
#define IOMESSAGE_PARSER_DEFINED 1

#include "Common.h"
#include "IoMessage.h"
#include "IoLexer.h"

#ifdef __cplusplus
extern "C" {
#endif

	IoMessage *IoMessage_newFromText_label_(void *state, const char *text, const char *label);
IoMessage *IoMessage_newParse(void *state, IoLexer *lexer);
void IoMessage_parseName(IoMessage *self, IoLexer *lexer);
void IoMessage_parseArgs(IoMessage *self, IoLexer *lexer);
void IoMessage_parseAttached(IoMessage *self, IoLexer *lexer);
void IoMessage_parseNext(IoMessage *self, IoLexer *lexer);

IoMessage *IoMessage_opShuffle(IoMessage *self, IoObject *locals, IoMessage *m);
void IoMessage_opShuffle_(IoMessage *self);

#ifdef __cplusplus
}
#endif
#endif
