#include "IoState.h"

void IoAddonsInit(IoObject *context); 

//#define IOBINDINGS 

int main(int argc, const char *argv[])
{
    IoState *self = IoState_new();
    #ifdef IOBINDINGS
    IoState_setBindingsInitCallback(self, (IoStateBindingsInitCallback *)IoAddonsInit);
    #endif
    IoState_init(self);
    IoState_argc_argv_(self, argc, argv);
    IoState_runCLI(self);
    IoState_free(self);
    return 0;
}
