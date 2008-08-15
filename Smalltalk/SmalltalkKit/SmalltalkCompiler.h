#import <Foundation/NSObject.h>

@interface SmalltalkCompiler : NSObject {}
/**
 * Specifies whether the compiler should run in debug mode.  Enabling this will
 * spam stderr with a huge amount of debugging information.
 */
+ (void) setDebugMode:(BOOL)aFlag;
/**
 * Compile and load the specified string.
 */
+ (BOOL) compileString:(NSString*)s;
/**
 * Load a framework with the specified name.
 */
+ (BOOL) loadFramework:(NSString*)framework;
@end
