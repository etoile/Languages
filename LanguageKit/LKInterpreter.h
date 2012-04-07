#import "Runtime/BlockClosure.h"
#import <LanguageKit/LKAST.h>
#import <LanguageKit/LKBlockExpr.h>
#import <LanguageKit/LKMethod.h>
#import <LanguageKit/LKSubclass.h>

extern NSString *LKInterpreterException;

LKMethod *LKASTForMethod(Class cls, NSString *selectorName);


/**
 * Structure for looking up the scope of a variable in interpreter contexts.
 */
typedef struct
{
	/**
	 * The scope of this symbol, once external references have been resolved.
	 */
	LKSymbolScope scope;
	/**
	 * The context where this variable can be accessed.
	 */
	__unsafe_unretained LKInterpreterContext *context;
} LKInterpreterVariableContext;

/**
 * Wrapper around a map table which contains the objects in a
 * Smalltalk stack frame.
 */
@interface LKInterpreterContext : NSObject
{
@public
	LKInterpreterContext *parent;
	LKSymbolTable *symbolTable;
@private
	NSMutableDictionary *objects;
}
@property (unsafe_unretained, nonatomic) id selfObject;
@property (retain, nonatomic) id blockContextObject;
- (id) initWithSymbolTable: (LKSymbolTable*)aTable
                    parent: (LKInterpreterContext*)aParent;
- (void) dealloc;
- (void) setValue: (id)value forSymbol: (NSString*)symbol;
- (id) valueForSymbol: (NSString*)symbol;
- (LKInterpreterVariableContext)contextForSymbol: (LKSymbol*)symbol;
@end


@interface LKAST (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context;
@end

@interface LKBlockExpr (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context;
- (id)executeWithArguments: (__unsafe_unretained id*)args count: (int)count inContext: (LKInterpreterContext*)context;
@end

@interface LKMethod (LKInterpreter)
- (id)executeInContext: (LKInterpreterContext*)context;
- (id)executeWithReciever: (id)receiver arguments: (id*)args count: (int)count;
@end

@interface LKSubclass (LKInterpreter)
- (void)setValue: (id)value forClassVariable: (NSString*)cvar;
- (id)valueForClassVariable: (NSString*)cvar;
@end
