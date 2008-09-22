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
 * Compile and load a .st file from the specified bundle.
 * Omit the .st extension in the name paramater.
 */
+ (BOOL) loadScriptInBundle:(NSBundle*)bundle named:(NSString*)name;
/**
 * Compile and load a .st file from the application bundle.
 * Omit the .st extension in the name paramater.
 */
+ (BOOL) loadApplicationScriptNamed:(NSString*)name;
/**
 * Load all of the Smalltalk scripts in the specified bundle.
 */
+ (BOOL) loadScriptsInBundle:(NSBundle*) aBundle;
/**
 * Load all of the Smalltalk scripts in the application bundle.
 */
+ (BOOL) loadAllScriptsForApplication;

@end
