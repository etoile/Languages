#import <Foundation/NSObject.h>
@class NSBundle;
@class NSString;
@class LKModule;
@class LKMethod;
@protocol LKCodeGenerator;

typedef enum
{
	LKDebuggingDisabled = 0,
	LKDebuggingEnabled = 1
} LKDebuggingMode;

/**
 * All languages must use a parser conforming to this protocol.
 */
@protocol LKParser <NSObject>
/**
 * Returns a module AST constructed by parsing the specified source.
 * The result is nil if parsing failed.
 */
- (LKModule*) parseString:(NSString*)source;
/**
 * Returns a method AST constructed by parsing the specified source, or nil if
 * parsing failed or method parsing is not supported by this language.
 */
- (LKMethod*) parseMethod:(NSString*)source;
@end

/**
 * Abstract class implementing a dynamic language compiler.  Concrete
 * subclasses provide implementations for specific languages.
 */
@interface LKCompiler : NSObject {}
/**
 * Returns a new autoreleased compiler for this language.
 */
+ (LKCompiler*) compiler;
/**
 * Specifies whether the compiler should run in debug mode.  Enabling this will
 * spam stderr with a huge amount of debugging information.  Note that this is
 * a global setting and will apply to all compilers.
 */
+ (void) setDebugMode:(LKDebuggingMode)aFlag;
/**
 * Compiles and loads the specified source code.
 */
- (BOOL) compileString:(NSString*)source;
/**
 * Compiles the specified source code to LLVM bitcode.  This can then be
 * optimised with the LLVM opt utility and converted to object code with llc.
 */
- (BOOL) compileString:(NSString*)source output:(NSString*)bitcode;
/**
 * Compiles the specified source code using the given code generator.
 */
- (BOOL) compileString:(NSString*)source withGenerator:(id<LKCodeGenerator>)cg;
/**
 * Compiles and loads a method on the class with the specified name.
 */
- (BOOL) compileMethod:(NSString*)source onClassNamed:(NSString*)name;
/**
 * Compiles a method to LLVM bitcode for the class with the specified name.
 */
- (BOOL) compileMethod:(NSString*)source 
          onClassNamed:(NSString*)name 
                output:(NSString*)bitcode;
/**
 * Compiles a method on the class with the specified name using the given code
 * generator.
 */
- (BOOL) compileMethod:(NSString*)source
          onClassNamed:(NSString*)name
         withGenerator:(id<LKCodeGenerator>)cg;
/**
 * Load a framework with the specified name.
 */
+ (BOOL) loadFrameworkNamed:(NSString*)framework;
/**
 * Compiles and loads a file from the specified bundle.
 * The file name extension will be used to select a suitable compiler.
 */
+ (BOOL) loadScriptNamed: (NSString*)fileName fromBundle: (NSBundle*)bundle;
/**
 * Compiles and loads a file from the specified bundle.
 * Omit the extension in the name paramater.
 */
- (BOOL) loadScriptNamed: (NSString*)fileName fromBundle: (NSBundle*)bundle;
/**
 * Compiles and loads a file from the application bundle.
 * The file name extension will be used to select a suitable compiler.
 */
+ (BOOL) loadApplicationScriptNamed:(NSString*)fileName;
/**
 * Compiles and loads a file from the application bundle.
 * Omit the extension in the name paramater.
 */
- (BOOL) loadApplicationScriptNamed:(NSString*)name;
/**
 * Loads all scripts for all known languages from the specified bundle.
 */
+ (BOOL) loadScriptsFromBundle:(NSBundle*) aBundle;
/**
 * Loads all scripts written in this language from the specified bundle.
 */
- (BOOL) loadScriptsFromBundle:(NSBundle*) aBundle;
/**
 * Loads all scripts for all known languages from the application bundle.
 */
+ (BOOL) loadAllScriptsForApplication;
/**
 * Loads all scripts written in this language from the application bundle.
 */
- (BOOL) loadAllScriptsForApplication;
/**
 * Loads a bundle containing an LKInfo.plist file.  This should contain a
 * Source key giving an array of source files in the order in which they should
 * be compiled, a Frameworks key giving an array of frameworks and a
 * PrincipalClass key for the class that should be instantiated when it is
 * loaded.
 *
 * Returns (Class)-1 in case of error, or the principal class if loading
 * succeeds (Nil if no principal class is specified).
 */
+ (Class) loadLanguageKitBundle:(NSBundle*)bundle;
/**
 * Loads all of the LanguageKit plugins for the current application.
 */
+ (BOOL) loadAllPlugInsForApplication;
/**
 * <override-subclass /> 
 * Returns the extension used for scripts in this language.
 *
 * Implemented by subclasses.
 */
+ (NSString*) fileExtension;
/**
 * <override-subclass /> 
 * Returns the name of the language supported by this compiler.
 *
 * Implemented by subclasses.
 */
+ (NSString*) languageName;
/**
 * Returns the languages for which compilers are currently supported.
 */
+ (NSArray*) supportedLanguageNames;
/**
 *	Returns the compiler for a named language. 
 */
+ (Class) compilerForLanguage:(NSString*) aLanguage;
/**
 * Returns the compiler for files with a given extension.
 */
+ (Class) compilerClassForFileExtension:(NSString*) anExtension;
/**
 * <override-subclass /> 
 * Returns the parser used by this language.
 */
+ (Class) parserClass;
@end

@interface LKCompiler (JTL)
+ (void) justTooLateCompileBundle: (NSBundle*)aBundle;
@end
