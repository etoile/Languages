/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
docDescription("""
    List - an array of void pointers
    User is responsible for freeing items
""")
*/

#ifndef LIST_DEFINED 
#define LIST_DEFINED 1

#include "Common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOW_MEMORY_SYSTEM
  #define LIST_START_SIZE 1
  #define LIST_RESIZE_FACTOR 2
#else
  #define LIST_START_SIZE 1
  #define LIST_RESIZE_FACTOR 2
#endif

#define LIST_AT_(self, n) self->items[n]
				 

typedef void  (ListDoCallback)(void *);
typedef void  (ListDoWithCallback)(void *, void *);
typedef void *(ListCollectCallback)(void *);
typedef int   (ListSelectCallback)(void *);
typedef int   (ListDetectCallback)(void *);
typedef int   (ListSortCallback)(const void *, const void *);
typedef int   (ListCompareFunc)(const void *, const void *);

typedef struct
{
    void **items;
    size_t memSize;
    int size;
} List;

typedef struct 
{	
	List *list;
	size_t index;
} ListCursor;

List *List_new(void);
List *List_clone(List *self);
List *List_cloneSlice(List *self, int startIndex, int endIndex);

void List_free(List *self);
void List_removeAll(List *self);
void List_copy_(List *self, List *otherList);
int  List_equals_(List *self, List *otherList);
size_t List_memorySize(List *self);

// sizing  

void List_preallocateToSize_(List *self, size_t index);
void List_setSize_(List *self, size_t index);
void List_compact(List *self);

// utility

void List_print(List *self);
void List_removeItemsAfterLastNULL_(List *self);
void List_sliceInPlace(List *self, int startIndex, int endIndex);

// enumeration

void List_target_do_(List *self, void *target, ListDoWithCallback *callback);
void List_do_(List *self, ListDoCallback *callback);
void List_do_with_(List *self, ListDoWithCallback *callback, void *arg);

List *List_map_(List *self, ListCollectCallback *callback);
void List_mapInPlace_(List *self, ListCollectCallback *callback);
void *List_detect_(List *self, ListDetectCallback *callback);
void *List_detect_withArg_(List *self, ListDetectCallback *callback, void *arg);
List *List_select_(List *self, ListSelectCallback *callback);

void *List_anyOne(List *self);
void List_shuffle(List *self);
void *List_removeLast(List *self);

#include "List_inline.h"

void List_append_sortedBy_(List *self, void *item, ListSortCallback *callback);

#ifdef __cplusplus
}
#endif
#endif
