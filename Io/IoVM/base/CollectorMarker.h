/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef COLLECTORMARKER_DEFINED
#define COLLECTORMARKER_DEFINED 1

#include "Common.h"
#include "List.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void  (CollectorDoFunc)(void *);

typedef struct CollectorMarker CollectorMarker;

struct CollectorMarker
{
	CollectorMarker *prev; // previous in color list 
	CollectorMarker *next; // next in color list 
	unsigned int color;    // white, gray, black and free
	//int num;
};

CollectorMarker *CollectorMarker_new(void);
CollectorMarker *CollectorMarker_newWithColor_(unsigned int color);
void CollectorMarker_free(CollectorMarker *self);

void CollectorMarker_loop(CollectorMarker *self);
void CollectorMarker_check(CollectorMarker *self);

void CollectorMarker_removeAndInsertAfter_(CollectorMarker *self, CollectorMarker *other);
void CollectorMarker_removeAndInsertBefore_(CollectorMarker *self, CollectorMarker *other);
void CollectorMarker_removeIfNeededAndInsertAfter_(CollectorMarker *self, CollectorMarker *other);

void CollectorMarker_remove(CollectorMarker *self);
int CollectorMarker_count(CollectorMarker *self);
int CollectorMarker_isEmpty(CollectorMarker *self);

/*
int CollectorMarker_num(void *self);
void CollectorMarker_setNext_(void *self, void *v);
void CollectorMarker_setPrev_(void *self, void *v);
*/

#define CollectorMarker_setColor_(self, c) ((CollectorMarker *)self)->color = c; 
#define CollectorMarker_color(self)        ((CollectorMarker *)self)->color; 

#define CollectorMarker_num(self)         (((CollectorMarker *)self)->num);
#define CollectorMarker_setNext_(self, v) ((CollectorMarker *)self)->next = v; 
#define CollectorMarker_setPrev_(self, v) ((CollectorMarker *)self)->prev = v;

#define MARKER(v) ((CollectorMarker *)v)

#define COLLECTMARKER_FOREACH(self, v, code) \
{ \
	CollectorMarker *v = self->next; \
	CollectorMarker *_next; \
	unsigned int c = self->color; \
	\
	while (v->color == c) \
	{  \
		_next = v->next; \
		code; \
	v = _next;  \
	} \
} 

#include "CollectorMarker_inline.h"


#ifdef __cplusplus
}
#endif
#endif
