/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef IOCFUNCTION_DEFINED
#define IOCFUNCTION_DEFINED 1

#include "Common.h"
#include "IoObject.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ISCFUNCTION(self) \
  IoObject_hasCloneFunc_(self, (TagCloneFunc *)IoCFunction_rawClone)

#define IOCFUNCTION(func, tag) IoCFunction_newWithFunctionPointer_tag_name_(IOSTATE, (IoUserFunction *)func, tag, "?")

typedef IoObject *(IoUserFunction)(IoObject *, IoObject *, IoMessage *);
typedef IoObject IoCFunction;

typedef struct
{
    IoTag *typeTag; // pointer to tag of type excepted for self value to have as data 
    IoUserFunction *func;
    IoSymbol *uniqueName;
} IoCFunctionData;

IoCFunction *IoCFunction_proto(void *state);
void IoCFunction_protoFinish(void *state);
IoCFunction *IoCFunction_rawClone(IoCFunction *self);
IoCFunction *IoCFunction_newWithFunctionPointer_tag_name_(void *state, IoUserFunction *s, IoTag *typeTag, const char *name);

void IoCFunction_mark(IoCFunction *self);
void IoCFunction_free(IoCFunction *self);
void IoCFunction_print(IoCFunction *self);

IoObject *IoCFunction_id(IoCFunction *self, IoObject *locals, IoMessage *m);
IoObject *IoCFunction_uniqueName(IoCFunction *self, IoObject *locals, IoMessage *m);
IoObject *IoCFunction_typeName(IoCFunction *self, IoObject *locals, IoMessage *m);
IoObject *IoCFunction_equals(IoCFunction *self, IoObject *locals, IoMessage *m);
IoObject *IoCFunction_activate(IoCFunction *self, IoObject *target, IoObject *locals, IoMessage *m, IoObject *slotContext);

IoObject *IoFunction_performOn(IoCFunction *self, IoObject *locals, IoMessage *m);

#ifdef __cplusplus
}
#endif
#endif

