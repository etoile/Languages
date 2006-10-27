/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef IOMESSAGE_DEFINED
#define IOMESSAGE_DEFINED 1

#include "Common.h"
#include "List.h"
#include "ByteArray.h"
#include "IoTag.h"
#include "IoObject_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MESSAGE_STOP_STATUS_NORMAL   0
#define MESSAGE_STOP_STATUS_BREAK    1
#define MESSAGE_STOP_STATUS_CONTINUE 2
#define MESSAGE_STOP_STATUS_RETURN   4

#define ISMESSAGE(self) IoObject_hasCloneFunc_(self, (TagCloneFunc *)IoMessage_rawClone)

#if !defined(IoSymbol_DEFINED) 
  #define IoSymbol_DEFINED
  typedef IoObject IoSymbol;
#endif

typedef IoObject IoMessage;

typedef struct
{
    IoSymbol *name; 
    List *args;
    IoMessage *attachedMessage;
    IoMessage *nextMessage;
    IoObject *cachedResult;
    
    /* debugging info */
    //int charNumber;
    int lineNumber;
    IoSymbol *label; 
} IoMessageData;

#define IOMESSAGEDATA(self) ((IoMessageData *)IoObject_dataPointer(self))

// basics 

IoMessage *IoMessage_proto(void *state);
IoMessage *IoMessage_rawClone(IoMessage *m);
IoMessage *IoMessage_new(void *state);
void IoMessage_copy_(IoMessage *self, IoMessage *other);
IoMessage *IoMessage_deepCopyOf_(IoMessage *m);
IoMessage *IoMessage_newWithName_(void *state, IoSymbol *symbol);
IoMessage *IoMessage_newWithName_label_(void *state, IoSymbol *symbol, IoSymbol *label);
IoMessage *IoMessage_newWithName_returnsValue_(void *state, IoSymbol *symbol, IoObject *v);
IoMessage *IoMessage_newWithName_andCachedArg_(void *state, IoSymbol *symbol, IoObject *v);

void IoMessage_mark(IoMessage *self);
void IoMessage_free(IoMessage *self);

void IoMessage_label_(IoMessage *self, IoSymbol *ioSymbol); /* sets label for children too */
int IoMessage_rawLineNumber(IoMessage *self);
int IoMessage_rawCharNumber(IoMessage *self);
void IoMessage_rawSetLineNumber_(IoMessage *self, int n);
void IoMessage_rawSetCharNumber_(IoMessage *self, int n);
List *IoMessage_rawArgList(IoMessage *self);
unsigned char IoMessage_needsEvaluation(IoMessage *self);

void IoMessage_addCachedArg_(IoMessage *self, IoObject *v);
void IoMessage_setCachedArg_to_(IoMessage *self, int n, IoObject *v);
void IoMessage_setCachedArg_toInt_(IoMessage *self, int n, int anInt);
void IoMessage_cachedResult_(IoMessage *self, IoObject *v);

IoObject *IoMessage_lineNumber(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_characterNumber(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_label(IoMessage *self, IoObject *locals, IoMessage *m);

// perform

IoObject *IoMessage_doInContext(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_locals_performOn_(IoMessage *self, IoObject *locals, IoObject *target);

// args

IoObject *IoMessage_locals_valueArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_valueOrEvalArgAt_(IoMessage *self, IoObject *locals, int n);
void IoMessage_assertArgCount_receiver_(IoMessage *self, int n, IoObject *receiver);
int IoMessage_argCount(IoMessage *self);

void IoMessage_locals_numberArgAt_errorForType_(
  IoMessage *self, 
  IoObject *locals, 
  int n, 
  const char *typeName);
  
IoObject *IoMessage_locals_numberArgAt_(IoMessage *self, IoObject *locals, int n);
int IoMessage_locals_intArgAt_(IoMessage *self, IoObject *locals, int n);
long IoMessage_locals_longArgAt_(IoMessage *self, IoObject *locals, int n);
double IoMessage_locals_doubleArgAt_(IoMessage *self, IoObject *locals, int n);
float IoMessage_locals_floatArgAt_(IoMessage *self, IoObject *locals, int n);

IoObject *IoMessage_locals_symbolArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_seqArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_mutableSeqArgAt_(IoMessage *self, IoObject *locals, int n);
char *IoMessage_locals_cStringArgAt_(IoMessage *self, IoObject *locals, int n);

IoObject *IoMessage_locals_blockArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_dateArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_mapArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_messageArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_listArgAt_(IoMessage *self, IoObject *locals, int n);

// printing 

void IoMessage_print(IoMessage *self);
void IoMessage_printWithReturn(IoMessage *self);
ByteArray *IoMessage_description(IoMessage *self);
ByteArray *IoMessage_descriptionJustSelfAndArgs(IoMessage *self); /* caller must free returned */
void IoMessage_appendDescriptionTo_follow_(IoMessage *self, ByteArray *ba, int follow);
IoObject *IoMessage_asString(IoMessage *self, IoObject *locals, IoMessage *m);

// primitive methods
IoObject *IoMessage_clone(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_protoName(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_protoSetName(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_descriptionString(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_nextMessage(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_setNextMessage(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_attachedMessage(IoMessage *self, IoObject *locals, IoMessage *m);

void IoMessage_rawSetAttachedMessage(IoMessage *self, IoMessage *m);
void IoMessage_rawSetNextMessage(IoMessage *self, IoMessage *m);
IoObject *IoMessage_setAttachedMessage(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_argAt(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_arguments(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_setArguments(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_appendArg(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_argCount_(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_cachedResult(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_setCachedResult(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_removeCachedResult(IoMessage *self, IoObject *locals, IoMessage *m);

IoObject *IoMessage_setLineNumber(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_setCharacterNumber(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_setLabel(IoMessage *self, IoObject *locals, IoMessage *m);

IoObject *IoMessage_fromString(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_argsEvaluatedIn(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_evaluatedArgs(IoMessage *self, IoObject *locals, IoMessage *m);

void IoMessage_foreachArgs(IoMessage *self, 
    IoObject *object, 
    IoSymbol **indexSlotName, 
    IoSymbol **valueSlotName, 
    IoMessage **doMessage);

IoMessage *IoMessage_asMessageWithEvaluatedArgs(IoMessage *self, IoObject *locals, IoMessage *m);

//  ------------------------------ 

#include "IoMessage_inline.h"
#include "IoMessage_parser.h"

//IoSymbol *IoMessage_name(IoMessage *self);

#define IoMessage_name(self)  (((IoMessageData *)IoObject_dataPointer(self))->name)
#define IoMessage_rawCachedResult(self) (((IoMessageData *)IoObject_dataPointer(self))->cachedResult)

void IoMessage_addArg_(IoMessage *self, IoMessage *m);
IoMessage *IoMessage_rawArgAt_(IoMessage *self, int n);
IoSymbol *IoMessage_rawLabel(IoMessage *self);
List *IoMessage_rawArgs(IoMessage *self);

ByteArray *IoMessage_asMinimalStackEntryDescription(IoMessage *self);

#ifdef __cplusplus
}
#endif
#endif
