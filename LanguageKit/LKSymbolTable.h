#import <EtoileFoundation/EtoileFoundation.h>

@class LKAST;

/** 
 * Enumerated type representing the scope of a variable.
 */
typedef	enum 
{
	LKSymbolScopeInvalid = 0,
	LKSymbolScopeExternal, /** Bound variable referenced in a block */
	LKSymbolScopePromoted, /** Bound variable promoted into a block */
	LKSymbolScopeArgument, /** Variable passed in to method / block */
	LKSymbolScopeLocal,    /** Variable declared in this lexical scope */
	LKSymbolScopeObject,   /** Instance variable */
	LKSymbolScopeClass,    /** Class variable */
	LKSymbolScopeGlobal,   /** Global (only clas names) */
	LKSymbolScopeBuiltin   /** Variable with special semantics 
							 (e.g. self / super) */
} LKSymbolScope;

/**
 * Symbol table.  Base class, with subclasses for each scope.
 */
@interface LKSymbolTable : NSObject {
	/** Name to type string mappings. */
	NSDictionary *types;
@public
  /** The parent scope for this symbol table. */
	LKSymbolTable *enclosingScope;
}
/**
 * Adds a global symbol for the class name.  This allows the class to be
 * referenced, but not subclassed, before it is defined.
 */
+ (void) forwardDeclareNewClass: (NSString*) className;
/**
 * Add a symbol to this table.
 */
- (void) addSymbol:(NSString*)aSymbol;
/**
 * Returns the scope of a specified symbol.  Calls the (private) non-recursive
 * variant in each of the parent scopes until it reaches the top of the AST.
 */
- (LKSymbolScope) scopeOfSymbol:(NSString*)aName;
/**
 * Returns the type encoding for a specified symbol.
 */
- (NSString*) typeOfSymbol:(NSString*)aName;
/**
 * Sets the parent for this symbol table.
 */
- (void) setScope:(LKSymbolTable*)enclosingScope;
/**
 * Returns the index of a local variable, recursively navigating up the tree
 * until it encounters the definition.
 */
- (int) offsetOfLocal:(NSString*)aName;
/**
 * Returns the offset of an instance variable, recursively navigating up the
 * tree until it encounters the definition.
 */
- (int) offsetOfIVar:(NSString*)aName;
/**
 * Returns the index of an argument, recursively navigating up the tree until
 * it encounters the definition.
 */
- (int) indexOfArgument:(NSString*)aName;
@end

/**
 * Symbol table for a method.
 */
@interface LKMethodSymbolTable : LKSymbolTable {
	/** Local variables declared in the current scope. */
	NSMutableArray * localVariables;
	/** Arguments in the current scope. */
	NSMutableArray * arguments;
}
/**
 * Initialise the symbol table with local variables and arguments.
 */
- (id) initWithLocals:(NSArray*)locals 
                 args:(NSArray*)args;
/**
 * Returns the local variables declared in this scope.
 */
- (NSArray*) locals;
/**
 * returns the arguments declared in this scope.
 */
- (NSArray*) args;
@end

/**
 * External symbols are those that reside in a lexical scope outside of the
 * current one.  This structure stores the symbol table and the distance to
 * where a variable is declared, in scopes.
 */
typedef struct
{
	int depth;
	LKSymbolTable *scope;
} LKExternalSymbolScope;
/**
 * Symbol table for a block.
 */
@interface LKBlockSymbolTable : LKMethodSymbolTable {
}
/**
 * The scope of an externally-referenced variable.
 */
- (LKExternalSymbolScope) scopeOfExternalSymbol:(NSString*)aSymbol;
@end


/**
 * Symbol table for an object.  Contains instance variables.
 */
@interface LKObjectSymbolTable : LKSymbolTable {
	/** Instance variables for this object. */
	NSMapTable * instanceVariables;
	/** Class variables for this object. */
	NSMutableArray * classVariables;
	/** Next offset at which an ivar can be added. */
	int nextOffset;
}
/**
 * Add a class variable to this symbol table. 
 */
- (void) addClassVariable: (NSString*) aClassVar;
/**
 * Returns the size of instances of this class.
 */
- (int) instanceSize;
/**
 * Adds a new class to the global symbol table.  Allows classes to be
 * referenced before they have been compiled. 
 */
- (void) registerNewClassNamed:(NSString*)aClass;
/**
 * Initialise for the specified class.  All instance variables in the specified
 * class will be added to the symbol table.
 */
- (LKSymbolTable*) initForClass:(Class)aClass;
/**
 * Returns the symbol table for a newly-created class.
 */
+ (LKSymbolTable*) symbolTableForNewClassNamed:(NSString*)aClass;
@end
