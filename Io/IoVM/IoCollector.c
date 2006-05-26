/*#io
Collector ioDoc(
			 docCopyright("Steve Dekorte", 2002)
			 docLicense("BSD revised")
			 docCategory("Core")
			 docDescription("""Contains methods related to Io's garbage collector. Io currently uses a incremental, non-moving, generational collector based on the tri-color (black/gray/white) algorithm with a write-barrier. 
<p>
Every N number of object allocs, the collector will walk some of the objects marked as gray, marking their connected white objects as gray and turning themselves black. Every M allocs, it will pause for a sweep where it makes sure all grays are marked black and frees all whites. 
<p>
If the sweepsPerGeneration is set to zero, it will immediately mark all blacks as white again and mark the root objects as gray. Otherwise, it will wait until the sweepsPerGeneration count is reached to do this. By adjusting the allocsPerSweep and sweepsPerGeneration appropriately, the collector can be tuned efficiently for various usage cases. Generally, the more objects in your heap, the larger you'll want this number.""")
			 */

#include "IoCollector.h"
#include "IoNumber.h"

IoObject *IoCollector_proto(void *state)
{
	IoMethodTable methodTable[] = { 
	{"setDebug", IoCollector_setDebug},
	{"collect", IoCollector_collect},
		
	{"setAllocsPerMark", IoCollector_setAllocsPerMark},
	{"allocsPerMark", IoCollector_allocsPerMark},
		
	{"setMarksPerSweep", IoCollector_setMarksPerSweep},
	{"marksPerSweep", IoCollector_marksPerSweep},
		
	{"setSweepsPerGeneration", IoCollector_setSweepsPerGeneration},
	{"sweepsPerGeneration", IoCollector_sweepsPerGeneration},
	{0x0, 0x0},
	};
	
	IoObject *self = IoObject_new(state);
	IoObject_addMethodTable_(self, methodTable);
	return self;
}


IoObject *IoCollector_collect(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("collect", "Runs garbage collector. Returns the number of items collected. ")
	*/
	
	int count = Collector_collect(IOSTATE->collector);
	return IONUMBER(count); 
}

IoObject *IoCollector_setDebug(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setDebug(aBool)", "Turns on/off printing of collector debugging messages. Returns self.")
	*/
	IoObject *bool = IoMessage_locals_valueArgAt_(m, locals, 0);
	
	Collector_setDebug_(IOSTATE->collector, ISTRUE(bool));
	return self; 
}

IoObject *IoCollector_setAllocsPerMark(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setAllocsPerMark(aNumber)", "Sets the number of object allocations per mark pass. Returns self.")
	*/
	
	int n = IoMessage_locals_intArgAt_(m, locals, 0);
	
	Collector_setAllocsPerMark_(IOSTATE->collector, n);
	return self; 
}

IoObject *IoCollector_allocsPerMark(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("allocsPerMark", 
		   "Return the number of allocations per collector mark pass.")
	*/
	
	return IONUMBER(Collector_allocsPerMark(IOSTATE->collector));
}

IoObject *IoCollector_setMarksPerSweep(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setMarksPerSweep(aNumber)", "Sets the number of object marks per sweep pass. Returns self.")
	*/
	
	int n = IoMessage_locals_intArgAt_(m, locals, 0);
	
	Collector_setMarksPerSweep_(IOSTATE->collector, n);
	return self; 
}

IoObject *IoCollector_marksPerSweep(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("marksPerSweep", 
		   "Return the number of marks per sweep pass.")
	*/
	
	return IONUMBER(Collector_marksPerSweep(IOSTATE->collector));
}


IoObject *IoCollector_setSweepsPerGeneration(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("setSweepsPerGeneration(aNumber)", 
		   "Sets the number of sweeps before black marks are turned to white. Returns self.")
	*/
	
	int n = IoMessage_locals_intArgAt_(m, locals, 0);
	
	Collector_setSweepsPerGeneration_(IOSTATE->collector, n);
	return self; 
}

IoObject *IoCollector_sweepsPerGeneration(IoObject *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("sweepsPerGeneration", "Return the number of sweeps per generation.")
	*/
	
	return IONUMBER(Collector_sweepsPerGeneration(IOSTATE->collector));
}
