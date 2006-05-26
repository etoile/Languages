#include "IoState.h"

void IoBindingsInit(IoState *self, IoObject *context)

//#define IOBINDINGS 
int main(int argc, const char *argv[])
{
    IoState *self = IoState_new();
    #ifdef IOBINDINGS
    IoState_setBindingsInitCallback(self, (IoStateBindingsInitCallback *)IoBindingsInit);
    #endif
    IoState_init(self);
    IoState_argc_argv_(self, argc, argv);
    IoState_runCLI(self);
    IoState_free(self);
    return 0;
}
