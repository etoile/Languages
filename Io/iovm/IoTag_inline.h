/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised") 
*/

#ifdef IOTAG_C 
#define IO_IN_C_FILE
#endif
#include "Common_inline.h"
#ifdef IO_DECLARE_INLINES

// state

IOINLINE void IoTag_state_(IoTag *self, void *state)
{
    self->state = state;
}

IOINLINE void *IoTag_state(IoTag *self)
{
    return self->state;
}

// activate

IOINLINE void IoTag_activateFunc_(IoTag *self, TagActivateFunc *func)
{
    self->activateFunc = func;
}

IOINLINE TagActivateFunc *IoTag_activateFunc(IoTag *self)
{
    return self->activateFunc;
}

// free

IOINLINE void IoTag_freeFunc_(IoTag *self, TagFreeFunc *func)
{
    self->freeFunc = func;
}

IOINLINE TagFreeFunc *IoTag_freeFunc(IoTag *self)
{
    return self->freeFunc;
}

// mark

IOINLINE void IoTag_markFunc_(IoTag *self, TagMarkFunc *func)
{
    self->markFunc = func;
}

IOINLINE TagMarkFunc *IoTag_markFunc(IoTag *self)
{
    return self->markFunc;
}

// compare

IOINLINE void IoTag_compareFunc_(IoTag *self, TagCompareFunc *func)
{
    self->compareFunc = func;
}

IOINLINE TagCompareFunc *IoTag_compareFunc(IoTag *self)
{
    return self->compareFunc;
}

#undef IO_IN_C_FILE
#endif

