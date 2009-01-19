#import <Foundation/NSObject.h>
@class NSBundle;
@class NSString;
@class LKAST;

/**
 * Informal protocol for parsers to conform to.
 */
@interface NSObject (LKParser)
/**
 * Parse a string and return an AST for the contained program.
 */
- (LKAST*) parseString:(NSString*)aProgram;
@end

/**
 * Abstract class implementing a dynamic language compiler.  Concrete
 * subclasses provide implementations for specific languages.
 */
@interface LKCompiler : NSObject {}
/**
 * Specifies whether the compiler should run in debug mode.  Enabling this will
 * spam stderr with a huge amount of debugging information.  Note that this is
 * a global setting and will apply to all compilers.
 */
+ (void) setDebugMode:(BOOL)aFlag;
/**
 * Compile and load the specified string.
 *
 * Must be implemented by subclasses.
 */
+ (BOOL) compileString:(NSString*)s;
/**
 * Compiles the specified source code to LLVM bitcode.  This can then be
 * optimised with the LLVM opt utility and converted to object code with llc.
 */
+ (BOOL) compileString:(NSString*) source output:(NSString*)bitcode;
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
/**
 * Returns the extension used for scripts in this language.
 *
 * Implemented by subclasses.
 */
+ (NSString*) fileExtension;
/**
 * Returns the name of the language supported by this compiler.
 *
 * Implemented by subclasses.
 */
+ (NSString*) languageName;
/**
 * Returns the languages for which compilers are currently supported.
 */
+ (NSArray*) supportedLanguages;
/**
 *	Returns the compiler for a named language. 
 */
+ (Class) compilerForLanguage:(NSString*) aLanguage;
/**
 * Returns the compiler for files with a given extension.
 */
+ (Class) compilerForExtension:(NSString*) anExtension;
/**
 * Returns the parser used by this language.
 */
+ (Class) parser;
@end
