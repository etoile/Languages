/* Copyright (c) 2006, Yen-Ju Chen
   All Rights Reserved. See COPYING.*/

/* Taken from IoVM/main.c */
#include "IoState.h"

void IoVectorInit(IoObject *context);
void IoObjcBridgeInit(IoObject *context);

/* Taken from IoBindingsInit.c generated from io language */
void IoBindingsInit(IoState *self, IoObject *context)
{
        IoVectorInit(context);
        IoObjcBridgeInit(context);
}

int main(int argc, const char *argv[])
{
    IoState *self = IoState_new();
    IoState_setBindingsInitCallback(self, (IoStateBindingsInitCallback *)IoBindingsInit);
    IoState_init(self);
    IoState_argc_argv_(self, argc, argv);
    IoState_runCLI(self);
    IoState_free(self);
    return 0;
}
