/*#io
Collector ioDoc(
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef IoCollector_DEFINED 
#define IoCollector_DEFINED 1

#include "IoObject.h"

#ifdef __cplusplus
extern "C" {
#endif

IoObject *IoCollector_proto(void *state);

IoObject *IoCollector_collect(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoCollector_setDebug(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoCollector_setAllocsPerMark(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoCollector_allocsPerMark(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoCollector_setMarksPerSweep(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoCollector_marksPerSweep(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoCollector_setSweepsPerGeneration(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoCollector_sweepsPerGeneration(IoObject *self, IoObject *locals, IoMessage *m);

#ifdef __cplusplus
}
#endif
#endif
