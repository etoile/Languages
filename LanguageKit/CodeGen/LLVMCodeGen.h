/***
 * Objective-C interface to the LLVM generator component.
 */

/**
 * Debug flag used to set whether excessive amounts of debugging info should be
 * spammed to stderr.
 */
extern int DEBUG_DUMP_MODULES;

#import <Foundation/NSObject.h>
#import <LanguageKit/LKCodeGen.h>

class CodeGenModule;
/**
 * Concrete implementation of the CodeGenerator protocol using LLVM.
 */
@interface LLVMCodeGen : NSObject <LKCodeGenerator>
{
	CodeGenModule *Builder;
	NSMapTable *labelledBasicBlocks;
}
+ (NSString*) smallIntBitcodeFile;
@end
