/*#io
docCopyright("Steve Dekorte", 2006)
docLicense("BSD revised")
docDescription("""A tricolor collector using a Baker treadmill.""")
*/

#ifndef Collector_DEFINED 
#define Collector_DEFINED 1

#if defined(WIN32)
#if defined(BUILDING_COLLECTOR_DLL) || defined(BUILDING_IOVMALL_DLL)
#define COLLECTOR_API __declspec(dllexport)
#else
#define COLLECTOR_API __declspec(dllimport)
#endif

#else
#define COLLECTOR_API
#endif

#include "CollectorMarker.h"

#ifdef __cplusplus
extern "C" {
#endif


#define COLLECTOR_FOREACH(self, v, code) \
	COLLECTMARKER_FOREACH(self->whites,  v, code;); \
	COLLECTMARKER_FOREACH(self->grays,   v, code;); \
	COLLECTMARKER_FOREACH(self->blacks,  v, code;);

typedef enum
{
	COLLECTOR_INITIAL_WHITE,
	COLLECTOR_GRAY,
	COLLECTOR_INITIAL_BLACK,
	COLLECTOR_FREE
} CollectorColor;

typedef void  (CollectorFreeFunc)(void *);
typedef void  (CollectorMarkFunc)(void *);

typedef struct
{
	List *retainedValues;
	void *markBeforeSweepValue;
	
	int pauseCount;
	
	CollectorMarker *whites;
	CollectorMarker *grays;
	CollectorMarker *blacks;
	CollectorMarker *freed;
	
	size_t allocsPerMark;          
	size_t allocsUntilMark; 
	
	size_t marksPerSweep;
	size_t marksUntilSweep;

	size_t sweepsPerGeneration;
	size_t sweepsUntilGeneration;
	
	CollectorFreeFunc *freeFunc;
	CollectorMarkFunc *markFunc;
	
	int debugOn;
} Collector;

COLLECTOR_API Collector *Collector_new(void);
COLLECTOR_API void Collector_free(Collector *self);

COLLECTOR_API void Collector_check(Collector *self);

COLLECTOR_API void Collector_setMarkBeforeSweepValue_(Collector *self, void *v);

// callbacks

COLLECTOR_API void Collector_setFreeFunc_(Collector *self, CollectorFreeFunc *func);
COLLECTOR_API void Collector_setMarkFunc_(Collector *self, CollectorMarkFunc *func);

// allocs per mark 

COLLECTOR_API void Collector_setAllocsPerMark_(Collector *self, size_t n);
COLLECTOR_API size_t Collector_allocsPerMark(Collector *self);

// marks per sweep

COLLECTOR_API void Collector_setMarksPerSweep_(Collector *self, size_t n);
COLLECTOR_API size_t Collector_marksPerSweep(Collector *self);

// sweeps per generation

COLLECTOR_API void Collector_setSweepsPerGeneration_(Collector *self, size_t n);
COLLECTOR_API size_t Collector_sweepsPerGeneration(Collector *self);

COLLECTOR_API void Collector_setDebug_(Collector *self, int b);

// retaining

COLLECTOR_API void *Collector_retain_(Collector *self, void *v);
COLLECTOR_API void Collector_stopRetaining_(Collector *self, void *v);
COLLECTOR_API void Collector_removeAllRetainedValues(Collector *self);

// adding

COLLECTOR_API void Collector_addValue_(Collector *self, void *v);

// collection

COLLECTOR_API void Collector_initPhase(Collector *self);
COLLECTOR_API size_t Collector_sweepPhase(Collector *self);
COLLECTOR_API void Collector_markPhase(Collector *self);
COLLECTOR_API void Collector_generationPhase(Collector *self);

COLLECTOR_API size_t Collector_collect(Collector *self);
COLLECTOR_API size_t Collector_freeAllValues(Collector *self);

// changing colors

#define Collector_shouldMark_(self, v) Collector_makeGrayIfWhite_(self, v)

COLLECTOR_API void Collector_makeGrayIfWhite_(Collector *self, void *v);
COLLECTOR_API void Collector_makeGray_(Collector *self, void *v);
//void Collector_makeLastGray_(Collector *self, void *v);

COLLECTOR_API void Collector_makeBWhite_(Collector *self, void *v);
COLLECTOR_API void Collector_makeGray_(Collector *self, void *v);
COLLECTOR_API void Collector_makeBlack_(Collector *self, void *v);

COLLECTOR_API int Collector_markerIsWhite_(Collector *self, CollectorMarker *m);
COLLECTOR_API int Collector_markerIsGray_(Collector *self, CollectorMarker *m);
COLLECTOR_API int Collector_markerIsBlack_(Collector *self, CollectorMarker *m);

COLLECTOR_API char *Collector_colorNameFor_(Collector *self, void *v);

COLLECTOR_API void *Collector_value_addingRefTo_(Collector *self, void *v, void *ref);

// pause/resume stack

COLLECTOR_API void Collector_pushPause(Collector *self);
COLLECTOR_API void Collector_popPause(Collector *self);
COLLECTOR_API int Collector_isPaused(Collector *self);

#include "Collector_inline.h"

#ifdef __cplusplus
}
#endif
#endif
