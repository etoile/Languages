/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef OBJECT_DEFINED 
#define OBJECT_DEFINED 1

#include "Common.h"
#include "PHash.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IoObject_clean(self) PHash_clean(self->slots);
#define IOREF(value) IoObject_addingRef_((IoObject *)self, (IoObject *)value)

#include "IoObject_struct.h"
#include "IoStore.h"
#include "IoMessage.h"
#include "IoSeq.h"
//#include "IoCoroutine.h"

IoObject *IoObject_proto(void *state);
IoObject *IoObject_protoFinish(void *state);
IoObject *IoObject_localsProto(void *state);

IoObject *IOCLONE(IoObject *self);
IoObject *IoObject_rawClone(IoObject *self);
IoObject *IoObject_justClone(IoObject *self);
IoObject *IoObject_rawClonePrimitive(IoObject *self);
IoObject *IoObject_new(void *state);

IoObject *IoObject_addMethod_(IoObject *self, IoSymbol *slotName, IoMethodFunc *fp);
void IoObject_addMethodTable_(IoObject *self, IoMethodTable *methodTable);

void IoObject_dealloc(IoObject *self);
void IoObject_free(IoObject *self);

// inheritance

void IoObject_setupProtos(IoObject *self);
int IoObject_hasProtos(IoObject *self);
int IoObject_rawProtosCount(IoObject *self);
void IoObject_rawAppendProto_(IoObject *self, IoObject *p);
void IoObject_rawPrependProto_(IoObject *self, IoObject *p);
void IoObject_rawRemoveProto_(IoObject *self, IoObject *p);
void IoObject_rawRemoveAllProtos(IoObject *self);
void IoObject_rawSetProto_(IoObject *self, IoObject *proto);

IoObject *IoObject_objectProto(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_setProto(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_setProtos(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_appendProto(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_prependProto(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_removeProto(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_removeAllProtos(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protos(IoObject *self, IoObject *locals, IoMessage *m);
unsigned int IoObject_rawHasProto_(IoObject *self, IoObject *p);

// slots

void IoObject_createSlots(IoObject *self);
void IoObject_setSlot_to_(IoObject *self, IoSymbol *slotName, IoObject *value);
IoObject *IoObject_getSlot_(IoObject *self, IoSymbol *slotName);
//IoObject *IoObject_objectWithSlotValue_(IoObject *self, IoObject *slotValue);
void IoObject_removeSlot_(IoObject *self, IoSymbol *slotName);

// perform and activate 

IoObject *IoObject_activate(IoObject *self, IoObject *target, IoObject *locals, IoMessage *m, IoObject *slotContext);
IoObject *IoObject_perform(IoObject *self, IoObject *locals, IoMessage *m);
//IoObject *IoObject_forward(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_localsForward(IoObject *self, IoObject *locals, IoMessage *m);

// tag functions 

int IoObject_compare(IoObject *self, IoObject *v);
int IoObject_defaultCompare(IoObject *self, IoObject *v);
const char *IoObject_name(IoObject *self);
void IoObject_print(IoObject *self);

// memory

size_t IoObject_memorySize(IoObject *self);
void IoObject_compact(IoObject *self);

char *IoObject_markColorName(IoObject *self);
void IoObject_show(IoObject *self);

// proto 

IoObject *IoObject_clone(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_shallowCopy(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_initClone_(IoObject *self, IoObject *locals, IoMessage *m, IoObject *newObject);

// printing 

IoObject *IoObject_protoPrint(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoWrite(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoWriteLn(IoObject *self, IoObject *locals, IoMessage *m);

// reflection 

IoObject *IoObject_protoPerform(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoPerformWithArgList(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_protoSet_to_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoSetSlotWithType(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_localsUpdateSlot(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoUpdateSlot_to_(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_protoGetSlot_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoGetLocalSlot_(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_protoHasLocalSlot(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoHasProto_(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_protoRemoveSlot(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoSlotNames(IoObject *self, IoObject *locals, IoMessage *m);

//IoObject *IoObject_forward_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_super(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_contextWithSlot(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_ancestorWithSlot(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_doMessage(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_self(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_locals(IoObject *self, IoObject *locals, IoMessage *m);

// memory

IoObject *IoObject_memorySizeMethod(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_compactMethod(IoObject *self, IoObject *locals, IoMessage *m);

// description 

void IoObject_defaultPrint(IoObject *self);

IoObject *IoObject_type(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_lobbyPrint(IoObject *self, IoObject *locals, IoMessage *m);

// logic 

IoObject *IoObject_and(IoObject *self, IoObject *locals, IoMessage *m);

// math

IoObject *IoObject_subtract(IoObject *self, IoObject *locals, IoMessage *m);

// comparison

int IoObject_sortCompare(IoObject **self, IoObject **v);

IoObject *IoObject_equals(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_notEquals(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoCompare(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_isLessThan_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_isLessThanOrEqualTo_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_isGreaterThan_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_isGreaterThanOrEqualTo_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_isNil(IoObject *self, IoObject *locals, IoMessage *m);

// meta 

IoObject *IoObject_evalArg(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_evalArgAndReturnNil(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_evalArgAndReturnSelf(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_uniqueId(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_do(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_message(IoObject *self, IoObject *locals, IoMessage *m);

// compiler

IoObject *IoObject_rawDoString_label_(IoObject *self, IoSymbol *string, IoSymbol *label);
IoObject *IoObject_doString(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_doFile(IoObject *self, IoObject *locals, IoMessage *m);
//IoObject *IoObject_unpack(IoObject *self, IoObject *locals, IoMessage *m);

// activatable

IoObject *IoObject_setIsActivatable(IoObject *self, IoObject *locals, IoMessage *m);

// eval

IoObject *IoObject_rawDoMessage(IoObject *self, IoMessage *m);
IoObject *IoObject_eval(IoObject *self, IoMessage *m, IoObject *locals);

IoObject *IoObject_argIsActivationRecord(IoObject *self, IoMessage *m, IoObject *locals);
IoObject *IoObject_argIsCall(IoObject *self, IoMessage *m, IoObject *locals);

ByteArray *IoObject_rawGetByteArraySlot(IoObject *self, 
								IoObject *locals, 
								IoMessage *m, 
								IoSymbol *slotName);
								
ByteArray *IoObject_rawGetMutableByteArraySlot(IoObject *self, 
								IoObject *locals, 
								IoMessage *m, 
								IoSymbol *slotName);
								
// free listeners ---------------------------------------------

void IoObject_addListener_(IoObject *self, void *listener);
void IoObject_removeListener_(IoObject *self, void *listener);

#include "IoObject_flow.h"
#include "IoObject_inline.h"

#ifdef __cplusplus
}
#endif
#endif
