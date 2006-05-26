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

#import <StepTalk/STEngine.h>
#import "IoState.h"
#import "IoObjcBridge.h"


@interface IoLanguageEngine:STEngine
{
	// FIXME: state should probably be stored in -interpretXXX STContext parameter
	IoState *state;
	IoObjcBridge *bridge;
}
@end
