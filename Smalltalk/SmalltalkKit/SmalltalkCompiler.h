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
/**
 * Compile and load a .st file from the application bundle.
 * Omit the .st extension in the name paramater.
 */
+ (BOOL) loadApplicationScriptNamed:(NSString*)name;

@end
