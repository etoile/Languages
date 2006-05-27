/* 
    IoLanguageEngine.h

    Main interpreter class to execute Io code in StepTalk context
   
    Copyright (C) 2006 Yen-Ju Chen

    Author:  Yen-Ju Chen
             Quentin Mathe <qmathe@club-internet.fr>
    Date:  May 2006
   
    This file is part of the Etoile desktop environment.

    All Rights Reserved. See COPYING.
*/ 

#import "IoLanguageEngine.h"
#import "Objc2Io.h"

static BOOL firstRun = YES;

/* Taken from IoBindingsInit.c generated from io language */
void IoAAVectorInit(IoState *self, IoObject *context);
void IoObjcBridgeInit(IoState *self, IoObject *context);

void IoBindingsInit(IoState *self, IoObject *context)
{
        IoAAVectorInit(self, context);
        IoObjcBridgeInit(self, context);
}

@implementation IoLanguageEngine

+ (STEngine *) engineForLanguage:(NSString *)name
{
	firstRun = YES;

	return [super engineForLanguage: name];
}

- (id)interpretScript:(NSString *)script
            inContext:(STContext *)context
{
    IoObject *result;

	if (firstRun)
	{
		state = IoState_new();

    	IoState_setBindingsInitCallback(state, (IoStateBindingsInitCallback *)IoBindingsInit);
    	IoState_init(state);
    	/* FIXME: We could be able to retrieve args with STContext instance
		   IoState_argc_argv_(state, argc, argv); */

		firstRun = NO;
	}

    result = IoState_doCString_(state, [script cString]);

	/* Now we process the result, possibly looking for an error. */

    bridge = IoObjcBridge_sharedBridge();

    char type = '@'; /* For objc object */
    char *error;
    void *cValue = IoObjcBridge_cValueForIoObject_ofType_error_(bridge, result, 
		&type, &error);

    if (error) 
	{
		return [NSString stringWithCString: error encoding: NSASCIIStringEncoding];
    } 
	else 
	{
      	/* return NSStringFromClass([*(id*)cValue class]);
      	   return [NSString stringWithFormat: @"%p", cValue]; */

		return *(id*)cValue;
    }

    return @"Io language: Failed";
}

@end
