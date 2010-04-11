/**
 * This file is a horribly ugly hack.  The old GNU runtime headers are horribly
 * broken with Objective-C++, but are included by GNUstep.  We need to fudge
 * things so that we get th things that we need defined without actually
 * including these headers.
 */
#import <EtoileFoundation/runtime.h>
typedef void* arglist_t;
typedef void* retval_t;
typedef Class MetaClass;
#define __objc_api_INCLUDE_GNU
#define __objc_INCLUDE_GNU
#define __encoding_INCLUDE_GNU

