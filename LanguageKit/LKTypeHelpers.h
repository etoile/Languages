#import <Foundation/Foundation.h>

/** 
 * Functions for working with Objective-C type strings
 */

void LKSkipQualifiers(const char **typestr);
void LKNextType(const char **typestr);
int LKCountObjCTypes(const char *objctype);
