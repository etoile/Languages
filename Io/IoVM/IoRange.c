/*#io
Range ioDoc(
    	 docCategory("DataStructures")
    	 docCopyright("Jeremy Tregunna", 2006)
    	 docLicense("BSD")
         docInclude("_ioCode/RangeCursor.io")
    	 docDescription("Simple datastructure representing the items at and between two specific points.")
    	 */

#include "IoState.h"
#define IORANGE_C 
#include "IoRange.h"
#undef IORANGE_C
#include "IoNumber.h"

#define DATA(self) ((IoRangeData *)IoObject_dataPointer(self))

IoTag *IoRange_tag(void *state)
{
    IoTag *tag = IoTag_newWithName_("Range");
    tag->state = state;
    tag->freeFunc = (TagFreeFunc *)IoRange_free;
    tag->cloneFunc = (TagCloneFunc *)IoRange_rawClone;
    tag->markFunc = (TagMarkFunc *)IoRange_mark;
    return tag;
}

IoRange *IoRange_proto(void *state)
{
    IoObject *self = IoObject_new(state);

    self->tag = IoRange_tag(state);
    IoObject_setDataPointer_(self, calloc(1, sizeof(IoRangeData)));
    DATA(self)->start = IONIL(self);
    DATA(self)->end = IONIL(self);
    DATA(self)->curr = IONIL(self);
    DATA(self)->increment = IONIL(self);
    DATA(self)->index = IONIL(self);

    IoState_registerProtoWithFunc_((IoState *)state, self, IoRange_proto);

    {
    	IoMethodTable methodTable[] = {
            {"first", IoRange_first},
            {"last", IoRange_last},
            {"next", IoRange_next},
            {"previous", IoRange_previous},
            {"index", IoRange_index},
            {"value", IoRange_value},
            {"foreach", IoRange_foreach},
            {"setRange", IoRange_setRange},
    	    {NULL, NULL},
    	};
    	IoObject_addMethodTable_(self, methodTable);
    }

    return self;
}

IoRange *IoRange_rawClone(IoRange *proto) 
{ 
    IoObject *self = IoObject_rawClonePrimitive(proto);
    self->tag = proto->tag;
    IoObject_setDataPointer_(self, calloc(1, sizeof(IoRangeData)));
    return self; 
}

IoRange *IoRange_new(void *state)
{
    IoRange *proto = IoState_protoWithInitFunction_(state, IoRange_proto);
    return IOCLONE(proto);
}

void IoRange_free(IoRange *self) 
{
    free(IoObject_dataPointer(self)); 
}

void IoRange_mark(IoRange *self)
{
    /* XXX: I have the strongest feeling I should be doing something here. */
}

/* ----------------------------------------------------------- */

IoObject *IoRange_first(IoRange *self, IoObject *locals, IoMessage *m)
{
    /*#io
     docSlot("first", "Moves the current cursor to the beginning of the range, and returns it.")
     */

    IoRangeData *rd = DATA(self);
    rd->curr = rd->start;
    return rd->curr;
}

IoObject *IoRange_last(IoRange *self, IoObject *locals, IoMessage *m)
{
    /*#io
     docSlot("last", "Moves the current cursor to the end of the range, and returns it.")
     */

    IoRangeData *rd = DATA(self);
    rd->curr = rd->end;
    return rd->curr;
}

IoObject *IoRange_next(IoRange *self, IoObject *locals, IoMessage *m)
{
    /*#io
     docSlot("next", "Sets the current item in the range to the next item in the range, and returns a boolean value indicating whether it is not at the end of the range.")
     */

    IoRangeData *rd = DATA(self);
    double newPos;

    if (strcmp(IoObject_name(rd->curr), "Number") == 0)
    {
        double end = CNUMBER(IoRange_getLast(self));
        double curr = CNUMBER(IoRange_getCurrent(self));
        double increment = CNUMBER(IoRange_getIncrement(self));
        double index = CNUMBER(IoRange_getIndex(self));
        unsigned int ret;
        newPos = curr + increment;
        ret = newPos <= end;
        if(ret)
        {
            IoRange_setCurrent(self, IONUMBER(newPos));
            IoRange_setIndex(self, IONUMBER(index + increment));
        }
        return IOBOOL(self, ret);
    }
    else
    {
        IoState_error_(IOSTATE, m, "'next' requires an implementation for types other than Number.");
        return IOFALSE(self);
    }
}

IoObject *IoRange_previous(IoRange *self, IoObject *locals, IoMessage *m)
{
    /*#io
     docSlot("previous", "Sets the current item in the range to the previous item in the range, and returns a boolean value indicating whether it is not at the beginning of the range.")
     */

    IoRangeData *rd = DATA(self);
    double newPos;

    if (strcmp(IoObject_name(rd->curr), "Number") == 0)
    {
        double start = CNUMBER(rd->start);
        double curr = CNUMBER(rd->curr);
        double increment = CNUMBER(rd->increment);
        unsigned int ret;
        newPos = curr - increment;
        ret = newPos >= start;
        if(ret)
            rd->curr = IONUMBER(newPos);
        return IOBOOL(self, ret);
    }
    else
    {
        IoState_error_(IOSTATE, m, "'previous' requires an implementation for types other than Number.");
        return IOFALSE(self);
    }
}

IoObject *IoRange_index(IoRange *self, IoObject *locals, IoMessage *m)
{
    /*#io
     docSlot("index", "Returns the current index number starting from zero and extending outward up to the maximum number of items in the range.")
     */

    return DATA(self)->index;
}

IoObject *IoRange_value(IoRange *self, IoObject *locals, IoMessage *m)
{
    /*#io
     docSlot("value", "Returns the value of the current item in the range.")
     */

    return DATA(self)->curr;
}

/* ----------------------------------------------------------- */

IoRange *IoRange_setRange(IoRange *self, IoObject *locals, IoMessage *m)
{
    /*#io
     docSlot("setRange(start, end, increment)", "Has several modes of operation. First, if only two parameters are specified, the increment value is set to 1 by default, while the first parameter represents the point to start from, and the second parameter represents the point to end at. If the second parameter is smaller than the first, the range will operate backwards. If the third parameter is specified, a custom iteration value will be used instead of 1.")
     */

    IoObject *start = IoMessage_locals_valueArgAt_(m, locals, 0);
    IoObject *end = IoMessage_locals_valueArgAt_(m, locals, 1);
    IoNumber *increment;

    if (IoMessage_argCount(m) == 3)
        increment = IoMessage_locals_numberArgAt_(m, locals, 2);
    else
        increment = IONUMBER(1);

    DATA(self)->start = IOREF(start);
    DATA(self)->end = IOREF(end);
    DATA(self)->curr = DATA(self)->start;
    DATA(self)->increment = IOREF(increment);
    DATA(self)->index = IONUMBER(0);

    return self;
}

IoObject *IoRange_each(IoRange *self, IoObject *locals, IoMessage *m)
{
    IoState *state = IOSTATE;
    IoObject *result = IONIL(self);
    IoMessage *doMessage = IoMessage_rawArgAt_(m, 0);

    if (strcmp(IoObject_name(IoRange_getFirst(self)), "Number") == 0)
    {
        double increment = CNUMBER(IoRange_getIncrement(self));
        double index;

        for(index = 0; ; index += increment)
        {
            IoState_clearTopPool(state);
            result = IoMessage_locals_performOn_(doMessage, locals, locals);
            if (IoRange_next(self, locals, m) == IOFALSE(self)) goto done;
            if (IoState_handleStatus(state)) goto done;
        }
    }
    else
    {
        IoState_error_(state, m, "Operation on items other than Numbers are not supported at this time.");
    }

done:
    IoState_popRetainPoolExceptFor_(state, result);
    return result;
}

IoObject *IoRange_foreach(IoRange *self, IoObject *locals, IoMessage *m)
{
    /*#io
     docSlot("foreach(optionalIndex, value, message)", """Iterates over each item beginning with the starting point, and finishing at the ending point inclusive. This method can operate several ways; these include: (1) Takes one argument, the message tree to be executed during each iteration; (2) Takes two arguments, the first argument is the name of the current value being iterated over, and the second is the message tree to be executed during each iteration; (3) Takes three arguments: the first is the current index within the range, the second is the name of the current value being iterated over, and the third is the message tree to be executed during each iteration. For example:
<pre>
    // First method (operating on numbers)
    1 to(10) foreach("iterating" print) // prints "iterating" 10 times
    // Second method (operating on numbers)
    1 to(10) foreach(v, v print) // prints each value
    // Third method (operating on numbers)
    1 to(10) foreach(i, v, writeln(i .. ": " .. v)) // prints "index: value"
</pre>
     """)
     */

    IoState *state = IOSTATE;
    IoObject *result = IONIL(self);
    IoSymbol *indexName;
    IoSymbol *valueName;
    IoMessage *doMessage;
    IoRangeData *rd = DATA(self);

    if (IoMessage_argCount(m) == 1)
    {
        return IoRange_each(self, locals, m);
    }

    IoMessage_foreachArgs(m, self, &indexName, &valueName, &doMessage);
    IoState_pushRetainPool(state);

    if (strcmp(IoObject_name(IoRange_getFirst(self)), "Number") == 0)
    {
        double increment = CNUMBER(IoRange_getIncrement(self));
        double index;

        for(index = 0; ; index += increment)
        {
            IoState_clearTopPool(state);
            if (indexName)
                IoObject_setSlot_to_(locals, indexName, IONUMBER(index));
            IoObject_setSlot_to_(locals, valueName, rd->curr);
            result = IoMessage_locals_performOn_(doMessage, locals, locals);
            if (IoState_handleStatus(state)) break;
            if (IoRange_next(self, locals, m) == IOFALSE(self)) break;
        }
    }
    else
    {
        IoState_error_(state, m, "Operation on items other than Numbers are not supported at this time.");
    }

    IoState_popRetainPoolExceptFor_(state, result);
    return result;
}

