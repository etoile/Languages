/*
 docCopyright("Steve Dekorte", 2002)
 docLicense("BSD revised")
 */

#define IOTAG_C 1
#include "IoTag.h"
#undef IOTAG_C 

#include "IoObject.h"
#include "IoState.h"
#include <string.h>

IoTag *IoTag_new(void)
{
	IoTag *self = (IoTag *)calloc(1, sizeof(IoTag));
	self->performFunc = (TagPerformFunc *)IoObject_perform;
	//self->performFunc = NULL;
	//self->recyclableInstances = Stack_new();
	//self->maxRecyclableInstances = 10000;
	return self;
}

IoTag *IoTag_newWithName_(char *name)
{
	IoTag *self = IoTag_new();
	IoTag_name_(self, name);
	return self;
}

void IoTag_free(IoTag *self) 
{
	//printf("free tag %p\n", (void *)self);
	//printf("%s\n", self->name ? self->name : "NULL");

	if (self->tagCleanupFunc)
	{
		(self->tagCleanupFunc)(self);
	}
	
	if (self->name) 
	{
		free(self->name); 
		self->name = NULL;
	}
	
	//Stack_free(self->recyclableInstances);
	free(self); 
}

void IoTag_name_(IoTag *self, const char *name)
{ 
	self->name = strcpy((char *)realloc(self->name, strlen(name)+1), name); 
}

const char *IoTag_name(IoTag *self) 
{ 
	return self->name; 
}

void IoTag_mark(IoTag *self)
{
	/*
	 if (Stack_count(self->recyclableInstances))
	 { 
		 Stack_do_(self->recyclableInstances, (StackDoCallback *)IoObject_shouldMark); 
	 }
	 */
}

