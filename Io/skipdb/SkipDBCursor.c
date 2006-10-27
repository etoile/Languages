/*#io
SkipDBCursor ioDoc(
			    docCopyright("Steve Dekorte", 2004)
			    docLicense("BSD revised")
			    docObject("SkipDBCursor")    
			    docDescription("A cursor for a skipdb.")
			    */

#include "SkipDBCursor.h"

SkipDBCursor *SkipDBCursor_new(void)
{
	SkipDBCursor *self = (SkipDBCursor *)calloc(1, sizeof(SkipDBCursor));
	return self;
}

SkipDBCursor *SkipDBCursor_newWithDB_(SkipDB *sdb)
{
	SkipDBCursor *self = SkipDBCursor_new();
	self->sdb = sdb;
	return self;
}

void SkipDBCursor_free(SkipDBCursor *self)
{
	SkipDB_freeCursor_(self->sdb, self);
	free(self);
}

void SkipDBCursor_mark(SkipDBCursor *self)
{
	if (self->record)
	{
		SkipDBRecord_mark(self->record);
	}
}

SkipDBRecord *SkipDBCursor_goto_(SkipDBCursor *self, Datum key)
{
	return (self->record = SkipDB_goto_((SkipDB *)(self->sdb), key));
}

SkipDBRecord *SkipDBCursor_first(SkipDBCursor *self)
{
	return (self->record = SkipDB_firstRecord(self->sdb));
}

SkipDBRecord *SkipDBCursor_last(SkipDBCursor *self)
{
	return (self->record = SkipDB_lastRecord(self->sdb));
}

SkipDBRecord *SkipDBCursor_current(SkipDBCursor *self)
{
	return self->record;
}

SkipDBRecord *SkipDBCursor_previous(SkipDBCursor *self)
{
	if (self->record) 
	{
		self->record = SkipDBRecord_previousRecord(self->record);
		
		if (self->record == SkipDB_headerRecord(self->sdb))
		{
			self->record = 0x0;
		}
	}
	return self->record;
}

SkipDBRecord *SkipDBCursor_next(SkipDBCursor *self)
{
	if (!self->record) 
	{
		return 0x0;
	}
	
	return (self->record = SkipDBRecord_nextRecord(self->record));
}
