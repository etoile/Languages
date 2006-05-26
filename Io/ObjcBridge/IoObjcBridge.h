/*   Copyright (c) 2003, Steve Dekorte
docLicense("BSD revised")
 *
 *   A bridge between Io and Objective-C
 *   This object is a singleton that tracks
 *   the lists of proxies on each side.
 */

#ifndef IOOBJCBRIDGE_DEFINED
#define IOOBJCBRIDGE_DEFINED 1

#include "IoState.h"
#include "Hash.h"
#include <Foundation/Foundation.h>

#define ISOBJCBRIDGE(self) IoObject_hasCloneFunc_(self, (TagCloneFunc *)IoObjcBridge_rawClone)

typedef IoObject IoObjcBridge;

typedef struct
{
  Hash *io2objcs;
  Hash *objc2ios;

  union cValue_tag
  {
    id o;
    Class class;
    SEL sel;
    char c;
    unsigned char C;
    short s;
    unsigned short S;
    int i;
    unsigned int I;
    long l;
    unsigned long L;
    float f;
    double d;    
    // bitfield
    void *v;
    char *cp;
    unsigned char *ucp;
    NSPoint point;
    NSSize size;
    NSRect rect;
  } cValue;

  char *methodNameBuffer;
  int methodNameBufferSize;
  int debug;
  List *allClasses;
} IoObjcBridgeData;

IoObjcBridge *IoObjcBridge_sharedBridge(void);
List *IoObjcBridge_allClasses(IoObjcBridge *self);

IoObjcBridge *IoObjcBridge_rawClone(IoObjcBridge *self); 
IoObjcBridge *IoObjcBridge_proto(void *state);
IoObjcBridge *IoObjcBridge_new(void *state);

void IoObjcBridge_free(IoObjcBridge *self);
void IoObjcBridge_mark(IoObjcBridge *self);

int IoObjcBridge_rawDebugOn(IoObjcBridge *self);

IoObject *IoObjcBridge_clone(IoObjcBridge *self, IoObject *locals, IoMessage *m);

IoObject *IoObjcBridge_autoLookupClassNamesOn(IoObjcBridge *self, IoObject *locals, IoMessage *m);
IoObject *IoObjcBridge_autoLookupClassNamesOff(IoObjcBridge *self, IoObject *locals, IoMessage *m);

IoObject *IoObjcBridge_debugOn(IoObjcBridge *self, IoObject *locals, IoMessage *m);
IoObject *IoObjcBridge_debugOff(IoObjcBridge *self, IoObject *locals, IoMessage *m);

IoObject *IoObjcBridge_NSSelectorFromString(IoObjcBridge *self, IoObject *locals, IoMessage *m);
IoObject *IoObjcBridge_NSStringFromSelector(IoObjcBridge *self, IoObject *locals, IoMessage *m);

/* ----------------------------------------------------------------- */
IoObject *IoObjcBridge_classNamed(IoObjcBridge *self, IoObject *locals, IoMessage *m);

void *IoObjcBridge_proxyForId_(IoObjcBridge *self, id obj);
void *IoObjcBridge_proxyForIoObject_(IoObjcBridge *self, IoObject *v);

void IoObjcBridge_removeId_(IoObjcBridge *self, id obj);
void IoObjcBridge_removeValue_(IoObjcBridge *self, IoObject *v);
void IoObjcBridge_addValue_(IoObjcBridge *self, IoObject *v, id obj);

/* ----------------------------------------------------------------- */
IoObject *IoObjcBridge_ioValueForCValue_ofType_(IoObjcBridge *self, void *cValue, char *cType);
void *IoObjcBridge_cValueForIoObject_ofType_error_(IoObjcBridge *self, IoObject *value, char *cType, char **error);

/* --- method name buffer ----------------------------------- */
void IoObjcBridge_setMethodBuffer_(IoObjcBridge *self, char *name);
char *IoObjcBridge_ioMethodFor_(IoObjcBridge *self, char *name);
char *IoObjcBridge_objcMethodFor_(IoObjcBridge *self, char *name);

/* --- new classes -------------------------------------------- */
IoObject *IoObjcBridge_newClassNamed_withProto_(IoObjcBridge *self, IoObject *locals, IoMessage *m);

char *IoObjcBridge_nameForTypeChar_(IoObjcBridge *self, char type);

#endif
