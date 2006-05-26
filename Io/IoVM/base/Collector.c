#define COLLECTOR_C
#include "Collector.h"
#undef COLLECTOR_C

#include <assert.h>

Collector *Collector_new(void)
{
	Collector *self = (Collector *)calloc(1, sizeof(Collector));
	
	self->retainedValues = List_new();
	
	self->whites = CollectorMarker_new();
	self->grays  = CollectorMarker_new();
	self->blacks = CollectorMarker_new();

	CollectorMarker_loop(self->whites);
	CollectorMarker_removeIfNeededAndInsertAfter_(self->grays, self->whites);
	CollectorMarker_removeIfNeededAndInsertAfter_(self->blacks, self->grays);

	// important to set colors *after* inserts, since inserts set colors
	CollectorMarker_setColor_(self->whites, COLLECTOR_INITIAL_WHITE);
	CollectorMarker_setColor_(self->blacks, COLLECTOR_INITIAL_BLACK);
	CollectorMarker_setColor_(self->grays, COLLECTOR_GRAY);

/*
	self->freed = CollectorMarker_new();
	CollectorMarker_removeIfNeededAndInsertAfter_(self->freed, self->blacks);
	CollectorMarker_setColor_(self->freed, COLLECTOR_INITIAL_FREE);
*/

	self->allocsPerMark = 10;
	self->allocsUntilMark = self->allocsPerMark;
	
	self->marksPerSweep = 1000;
	self->marksUntilSweep = self->marksPerSweep;
	
	self->sweepsPerGeneration = 3;
	self->sweepsUntilGeneration = self->sweepsPerGeneration;
	
	Collector_check(self);
	
	return self;
}

void Collector_check(Collector *self)
{ 	
	CollectorMarker *w = self->whites;
	CollectorMarker *g = self->grays;
	CollectorMarker *b = self->blacks;
	
	// colors are different
	assert(w->color != g->color);
	assert(w->color != b->color);
	assert(g->color != b->color);
	
	// prev color is different
	assert(w->prev->color != w->color);
	assert(g->prev->color != g->color);
	assert(b->prev->color != b->color);
	
	CollectorMarker_check(w);
}

void Collector_free(Collector *self)
{
	List_free(self->retainedValues);
	CollectorMarker_free(self->whites);
	CollectorMarker_free(self->grays);
	CollectorMarker_free(self->blacks);
	//CollectorMarker_free(self->freed);
	free(self);
}

void Collector_setMarkBeforeSweepValue_(Collector *self, void *v)
{
	self->markBeforeSweepValue = v;
}

// callbacks

void Collector_setFreeFunc_(Collector *self, CollectorFreeFunc *func)
{
	self->freeFunc = func;
}

void Collector_setMarkFunc_(Collector *self, CollectorMarkFunc *func)
{
	self->markFunc = func;
}

// allocs per mark 

void Collector_setAllocsPerMark_(Collector *self, size_t n)
{
	self->allocsPerMark = n;
}

size_t Collector_allocsPerMark(Collector *self)
{
	return self->allocsPerMark;
}

// marks per sweep

void Collector_setMarksPerSweep_(Collector *self, size_t n)
{
	self->marksPerSweep = n;
}

size_t Collector_marksPerSweep(Collector *self)
{
	return self->marksPerSweep;
}

// sweeps per generation

void Collector_setSweepsPerGeneration_(Collector *self, size_t n)
{
	self->sweepsPerGeneration = n;
}

size_t Collector_sweepsPerGeneration(Collector *self)
{
	return self->sweepsPerGeneration;
}

void Collector_setDebug_(Collector *self, int b)
{
	self->debugOn = b ? 1 : 0;
}

// retain/release --------------------------------------------

void *Collector_retain_(Collector *self, void *v)
{ 
	//Collector_check(self);
	List_append_(self->retainedValues, v); 
	CollectorMarker_removeIfNeededAndInsertAfter_(v, self->grays);
	//Collector_check(self);
	return v;
}

void Collector_stopRetaining_(Collector *self, void *v)
{ 
	List_removeLast_(self->retainedValues, v);
}

void Collector_removeAllRetainedValues(Collector *self)
{
	List_removeAll(self->retainedValues); 
}

// pausing -------------------------------------------------------------------

int Collector_isPaused(Collector *self)
{ 
	return (self->pauseCount != 0); 
}

void Collector_pushPause(Collector *self)
{
	self->pauseCount ++;
}

void Collector_popPause(Collector *self)
{
	assert(self->pauseCount > 0);
	
	self->pauseCount --;
	
	if (self->pauseCount == 0 && self->allocsUntilMark == 0) 
	{
		Collector_markPhase(self);
	}
}

// adding ------------------------------------------------

//static int valueCount = 0;

void Collector_addValue_(Collector *self, void *v)
{ 
	//MARKER(v)->num = valueCount;
	//valueCount ++;
	//Collector_check(self);
	
	CollectorMarker_removeIfNeededAndInsertAfter_(v, self->whites);
	
	if (self->allocsUntilMark == 0) 
	{
		if (self->pauseCount == 0)
		{
			Collector_markPhase(self);
		}
	}
	else
	{
		self->allocsUntilMark --;
	}
	//Collector_check(self);
}

// collection ------------------------------------------------

int Collector_markGrays(Collector *self)
{
	int count = 0;
	CollectorFreeFunc *markFunc = self->markFunc;
	
	COLLECTMARKER_FOREACH(self->grays, v, 
					  (*markFunc)(v);
					  Collector_makeBlack_(self, v);
					  count ++;
					  );
	return count;
}

int Collector_freeWhites(Collector *self)
{
	int count = 0;
	CollectorFreeFunc *freeFunc = self->freeFunc;
	
	COLLECTMARKER_FOREACH(self->whites, v, 
					  (*freeFunc)(v);
					  CollectorMarker_remove(v);
					  count ++;
					  );
	return count;
}

// ---------------------

void Collector_initPhase(Collector *self)
{
	LIST_FOREACH(self->retainedValues, i, v, Collector_makeGray_(self, v));
}

void Collector_markPhase(Collector *self)
{
	Collector_markGrays(self);
	
	self->allocsUntilMark = self->allocsPerMark;

	if (!(self->marksUntilSweep --)) 
	{
		Collector_sweepPhase(self);
	}
}

size_t Collector_sweepPhase(Collector *self)
{
	size_t freedCount = 0;
	
	if (self->debugOn)
	{
		printf("Collector_sweepPhase()\n");
	}
	
	self->marksUntilSweep = self->marksPerSweep;
		
	if (self->markBeforeSweepValue)
	{
		Collector_makeGray_(self, self->markBeforeSweepValue);
		//printf("self->markBeforeSweepValue = %p\n", (void *)self->markBeforeSweepValue);
	}
	
	while (!CollectorMarker_isEmpty(self->grays)) 
	{
		Collector_markGrays(self);
		//int marked = Collector_markGrays(self);
		//printf("grays marked %i\n", marked);
	}
	
	freedCount = Collector_freeWhites(self);
	//printf("whites freed %i\n", (int)freedCount);

	if (!(self->sweepsUntilGeneration --))
	{
		Collector_generationPhase(self);
	}
		
	return freedCount;
}

void Collector_generationPhase(Collector *self)
{
	CollectorMarker *b = self->blacks;
	
	self->blacks = self->whites;
	self->whites = b;
	
	self->sweepsUntilGeneration = self->sweepsPerGeneration;
	Collector_initPhase(self);
}

size_t Collector_collect(Collector *self)
{
	size_t count = 0;
	
	// collect twice to ensure that any now unreachable blacks get collected
	
	if (self->pauseCount)
	{
		printf("Collector warning: attempt to force collection while pause count = %i\n", self->pauseCount);
	}
	else
	{
		count += Collector_sweepPhase(self);
		Collector_generationPhase(self);
		
		count += Collector_sweepPhase(self);
		Collector_generationPhase(self);
	}
	
	return count;
}

size_t Collector_freeAllValues(Collector *self)
{
	size_t count = 0;
	CollectorFreeFunc *freeFunc = self->freeFunc;
	
	// collect twice to ensure that any now unreachable blacks get collected
	
	COLLECTMARKER_FOREACH(self->whites,  v, (*freeFunc)(v); CollectorMarker_remove(v); count ++;);
	COLLECTMARKER_FOREACH(self->grays,   v, (*freeFunc)(v); CollectorMarker_remove(v); count ++;);
	COLLECTMARKER_FOREACH(self->blacks,  v, (*freeFunc)(v); CollectorMarker_remove(v); count ++;);
	//COLLECTMARKER_FOREACH(self->freed, v, (*freeFunc)(v); CollectorMarker_remove(v); count ++;);
	
	return count;
}

// helpers ----------------------------------------------------------------------------

void Collector_show(Collector *self)
{		
	printf("black: %i\n", (int)CollectorMarker_count(self->blacks));
	printf("gray:  %i\n", (int)CollectorMarker_count(self->grays));
	printf("white: %i\n", (int)CollectorMarker_count(self->whites));
}

char *Collector_colorNameFor_(Collector *self, void *v)
{
	if (self->whites->color == MARKER(v)->color) return "white";
	if (self->grays->color  == MARKER(v)->color) return "gray";
	if (self->blacks->color == MARKER(v)->color) return "black";
	return "off-white";
}

