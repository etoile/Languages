/*
 docCopyright("Steve Dekorte", 2002)
 docLicense("BSD revised")
 */

#include "IoState.h"
#include "IoObject.h"

void IoState_break(IoState *self, IoObject *v)
{
	self->stopStatus = MESSAGE_STOP_STATUS_BREAK;
	self->returnValue = v;
}

void IoState_continue(IoState *self)
{ 
	self->stopStatus = MESSAGE_STOP_STATUS_CONTINUE; 
}

void IoState_eol(IoState *self)
{ 
	self->stopStatus = MESSAGE_STOP_STATUS_EOL; 
}

void IoState_return(IoState *self, IoObject *v)
{ 
	self->stopStatus = MESSAGE_STOP_STATUS_RETURN; 
	self->returnValue = v;
}

void IoState_resetStopStatus(IoState *self)
{  
	self->stopStatus = MESSAGE_STOP_STATUS_NORMAL; 
}

int IoState_handleStatus(IoState *self)
{
	switch (self->stopStatus)
	{
		case MESSAGE_STOP_STATUS_RETURN:
			return 1;
			
		case MESSAGE_STOP_STATUS_BREAK:
			IoState_resetStopStatus(self); 
			return 1;
			
		case MESSAGE_STOP_STATUS_CONTINUE:
			IoState_resetStopStatus(self); 
                        return 0;

                default:
                        return 0;
	}
}
