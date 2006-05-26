/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
docDescription("""
    Stack - array of void pointers
    supports setting marks - when a mark is popped,
    all stack items above it are popped as well 
    
    Designed to optimize push, pushMark and popMark 
    at the expense of pop (since pop requires a mark check)
""")
*/

#ifndef STACK_DEFINED
#define STACK_DEFINED 1

#include "Common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "List.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOW_MEMORY_SYSTEM
  #define STACK_START_SIZE 512
  #define STACK_RESIZE_FACTOR 2
#else
  #define STACK_START_SIZE 512
  #define STACK_RESIZE_FACTOR 2
#endif

typedef void (StackDoCallback)(void *);
typedef void (StackDoOnCallback)(void *, void *);

typedef struct
{
  void **items;
  void **memEnd;
  void **top;
  ptrdiff_t lastMark;
} Stack;

Stack *Stack_new(void);
void Stack_free(Stack *self);
Stack *Stack_clone(Stack *self);
void Stack_copy_(Stack *self, Stack *other);

size_t Stack_memorySize(Stack *self);
void Stack_compact(Stack *self);

void Stack_resize(Stack *self);

void Stack_popToMark_(Stack *self, ptrdiff_t mark);

// not high performance 

void Stack_makeMarksNull(Stack *self);
Stack *Stack_newCopyWithNullMarks(Stack *self);
void Stack_do_on_(Stack *self, StackDoOnCallback *callback, void *target);

List *Stack_asList(Stack *self);

#include "Stack_inline.h"

#ifdef __cplusplus
}
#endif
#endif
