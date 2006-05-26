/*#io
docCopyright("Steve Dekorte", 2006)
docLicense("BSD revised")
docDescription("""A tricolor collector using a Baker treadmill.""")
*/

#ifndef Collector_DEFINED 
#define Collector_DEFINED 1

#include "CollectorMarker.h"

#ifdef __cplusplus
extern "C" {
#endif

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

Collector *Collector_new(void);
void Collector_free(Collector *self);

void Collector_check(Collector *self);

void Collector_setMarkBeforeSweepValue_(Collector *self, void *v);

// callbacks

void Collector_setFreeFunc_(Collector *self, CollectorFreeFunc *func);
void Collector_setMarkFunc_(Collector *self, CollectorMarkFunc *func);

// allocs per mark 

void Collector_setAllocsPerMark_(Collector *self, size_t n);
size_t Collector_allocsPerMark(Collector *self);

// marks per sweep

void Collector_setMarksPerSweep_(Collector *self, size_t n);
size_t Collector_marksPerSweep(Collector *self);

// sweeps per generation

void Collector_setSweepsPerGeneration_(Collector *self, size_t n);
size_t Collector_sweepsPerGeneration(Collector *self);

void Collector_setDebug_(Collector *self, int b);

// retaining

void *Collector_retain_(Collector *self, void *v);
void Collector_stopRetaining_(Collector *self, void *v);
void Collector_removeAllRetainedValues(Collector *self);

// adding

void Collector_addValue_(Collector *self, void *v);

// collection

void Collector_initPhase(Collector *self);
size_t Collector_sweepPhase(Collector *self);
void Collector_markPhase(Collector *self);
void Collector_generationPhase(Collector *self);

size_t Collector_collect(Collector *self);
size_t Collector_freeAllValues(Collector *self);

// changing colors

#define Collector_shouldMark_(self, v) Collector_makeGrayIfWhite_(self, v)

void Collector_makeGrayIfWhite_(Collector *self, void *v);
void Collector_makeGray_(Collector *self, void *v);
//void Collector_makeLastGray_(Collector *self, void *v);

void Collector_makeBWhite_(Collector *self, void *v);
void Collector_makeGray_(Collector *self, void *v);
void Collector_makeBlack_(Collector *self, void *v);

int Collector_markerIsWhite_(Collector *self, CollectorMarker *m);
int Collector_markerIsGray_(Collector *self, CollectorMarker *m);
int Collector_markerIsBlack_(Collector *self, CollectorMarker *m);

char *Collector_colorNameFor_(Collector *self, void *v);

void *Collector_value_addingRefTo_(Collector *self, void *v, void *ref);

// pause/resume stack

void Collector_pushPause(Collector *self);
void Collector_popPause(Collector *self);
int Collector_isPaused(Collector *self);

#include "Collector_inline.h"

#ifdef __cplusplus
}
#endif
#endif
