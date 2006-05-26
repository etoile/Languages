#ifdef IORANGE_C 
#define IO_IN_C_FILE
#endif
#include "Common_inline.h"
#ifdef IO_DECLARE_INLINES

#define DATA(self) ((IoRangeData *)IoObject_dataPointer(self))

IOINLINE IoObject *IoRange_getFirst(IoRange *self)
{
    return DATA(self)->start;
}

IOINLINE void IoRange_setFirst(IoRange *self, IoObject *v)
{
    DATA(self)->start = v;
}

IOINLINE IoObject *IoRange_getLast(IoRange *self)
{
    return DATA(self)->end;
}

IOINLINE void IoRange_setLast(IoRange *self, IoObject *v)
{
    DATA(self)->end = v;
}

IOINLINE IoObject *IoRange_getCurrent(IoRange *self)
{
    return DATA(self)->curr;
}

IOINLINE void IoRange_setCurrent(IoRange *self, IoObject *v)
{
    DATA(self)->curr = v;
}

IOINLINE IoObject *IoRange_getIncrement(IoRange *self)
{
    return DATA(self)->increment;
}

IOINLINE void IoRange_setIncrement(IoRange *self, IoObject *v)
{
    DATA(self)->increment = v;
}

IOINLINE IoObject *IoRange_getIndex(IoRange *self)
{
    return DATA(self)->index;
}

IOINLINE void IoRange_setIndex(IoRange *self, IoObject *v)
{
    DATA(self)->index = v;
}

#undef IO_IN_C_FILE
#endif
