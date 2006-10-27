/*#io
docCopyright("Steve Dekorte", 2002)
docLicense("BSD revised")
*/

#ifndef OBJECT_STRUCT_DEFINED 
#define OBJECT_STRUCT_DEFINED 1

#include "Common.h"
#include "PHash.h"
#include "BStream.h"
#include "IoTag.h"
#include "Collector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IoObject IoObject;

#define IoObject_setHasDoneLookup_(self, v) (self)->hasDoneLookup = v;
#define IoObject_hasDoneLookup(self)        (self)->hasDoneLookup
	
#define IoObject_setIsReferenced_(self, v)  (self)->marker.isReferenced = v; 
#define IoObject_isReferenced(self)         (self)->marker.isReferenced

#define IoObject_dataPointer(self)          (self)->data.ptr
#define IoObject_setDataPointer_(self, v)   (self)->data.ptr = (void *)(v);

#define IoObject_dataDouble(self)           (self)->data.d
#define IoObject_setDataDouble_(self, v)    (self)->data.d = (double)(v);

#define IoObject_dataUint32(self)           (self)->data.ui32
#define IoObject_setDataUint32_(self, v)    (self)->data.ui32 = (uint32_t)(v);

struct IoObject
{
	CollectorMarker marker;
	
	union {
		void *ptr;
		double d;
		uint32_t ui32;
	} data;
	
	List *listeners;
	
	IoTag *tag; 
	void *state;
	PHash *slots;
	IoObject **protos;
	PID_TYPE persistentId; 
	
	unsigned int hasDoneLookup : 1;    // used to avoid slot lookup loops
	unsigned int ownsSlots : 1;        // if true, free slots Hash when freeing object
	unsigned int isSymbol : 1;         // true if the object is literal - such as a literal string
	
	unsigned int isDirty : 1;          // set to true when the object changes it's storable state
	//unsigned int doesNotOwnData : 1;   // if false, call freeFunc when freeing object
	unsigned int isLocals : 1;         // true if the Object is a locals object
	unsigned int isActivatable : 1;    // if true, upon activation, call activate slot
};

typedef IoObject *(IoMethodFunc)(IoObject *, IoObject *, IoObject *);

typedef struct  
{
    char *name;
    IoMethodFunc *func;
} IoMethodTable;

#ifdef __cplusplus
}
#endif
#endif
