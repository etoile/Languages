/*   
*/

#ifndef CORO_DEFINED
#define CORO_DEFINED 1

#include "Common.h"

#if defined(__SYMBIAN32__)
	#define CORO_STACK_SIZE     8192
	#define CORO_STACK_SIZE_MIN 1024
#else
     #define CORO_STACK_SIZE     1048576
     //IoSeq_lowercase#define CORO_STACK_SIZE     65536
	#define CORO_STACK_SIZE_MIN 8192
#endif

#if defined(WIN32) && defined(HAS_FIBERS)
	#define CORO_IMPLEMENTATION "fibers"
#elif defined(HAS_UCONTEXT)
	#include <ucontext.h>
	#define CORO_IMPLEMENTATION "ucontext"
#else
	#include <setjmp.h>
	#define CORO_IMPLEMENTATION "setjmp"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Coro Coro;

struct Coro
{        
	size_t stackSize;
	void *stack;

#ifdef USE_VALGRIND
	unsigned int valgrindStackId;
#endif

#if defined(HAS_FIBERS)
    void *fiber;
#else
	#if defined(HAS_UCONTEXT)
	    ucontext_t env;
	#else
	    jmp_buf env;
	#endif
#endif

	char isMain;
};

Coro *Coro_new(void);
void Coro_free(Coro *self);

// stack

void *Coro_stack(Coro *self);
size_t Coro_stackSize(Coro *self);
ptrdiff_t Coro_bytesLeftOnStack(Coro *self);
int Coro_stackSpaceAlmostGone(Coro *self);

void Coro_initializeMainCoro(Coro *self);

typedef void (CoroStartCallback)(void *);

void Coro_startCoro_(Coro *self, Coro *other, void *context, CoroStartCallback *callback);
void Coro_switchTo_(Coro *self, Coro *next);
void Coro_setup(Coro *self, void *arg); // private

#ifdef __cplusplus
}
#endif
#endif
