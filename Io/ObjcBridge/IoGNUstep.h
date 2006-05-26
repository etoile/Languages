#ifndef __IO_GNUSTEP__
#define __IO_GNUSTEP__

/*
 * This is a set of macros that translate Apple Objective-C runtime call to
 * GNU Objective-C runtime call.
 */
#ifdef GNUSTEP
 #include <Foundation/Foundation.h>
 #undef sel_getUid

 #include <objc/objc.h>
 #include <objc/objc-list.h>
 #include <objc/objc-api.h>

 #define Method Method_t;
 #define class_getInstanceMethod class_get_instance_method
 #define sel_getUid sel_get_uid
 #define sel_getName sel_get_name
 #define objc_getClass objc_get_class
 #define objc_lookUpClass objc_lookup_class
 #define objc_getClassList(a,b) GSClassList((a),(b),YES)
 #define objc_addClass(a) GSObjCAddClasses([NSArray arrayWithObject:(a)])
#endif

#endif
