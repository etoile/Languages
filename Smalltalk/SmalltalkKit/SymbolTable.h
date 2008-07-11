#import <EtoileFoundation/EtoileFoundation.h>

@class AST;

/** 
 * Enumerated type representing the scope of a variable.
 */
typedef	enum 
{
  invalid = 0,
  external, // Bound variable referenced in a block
  promoted, // Bound variable promoted into a block
	argument, // Variable passed in to method / block
	local,    // Variable declared in this lexical scope
	object,   // Instance variable
	global,   // Global (only clas names)
  builtin   // Variable with special semantics (e.g. self / super)
} SymbolScope;

/**
 * Symbol table.  Base class, with subclasses for each scope.
 */
@interface SymbolTable : NSObject {
  /** Name to type string mappings. */
	NSDictionary *types;
@public
  /** The parent scope for this symbol table. */
	SymbolTable *enclosingScope;
}
/**
 * Adds a new class to the global symbol table.  Allows classes to be
 * referenced before they have been compiled. 
 */
+ (void) registerNewClass:(NSString*)aClass;
/**
 * Sets the AST node where this symbol table is valid.
 */
- (void) setASTScope:(AST*)aNode;
/**
 * Add a symbol to this table.
 */
- (void) addSymbol:(NSString*)aSymbol;
/**
 * Returns the scope of a specified symbol.  Calls the non-recursive variant in
 * each of the parent scopes until it reaches the top of the AST.
 */
- (SymbolScope) scopeOfSymbol:(NSString*)aName;
/**
 * Returns the type encoding for a specified symbol.
 */
- (NSString*) typeOfSymbol:(NSString*)aName;
/**
 * Sets the parent for this symbol table.
 */
- (void) setScope:(SymbolTable*)enclosingScope;
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
@interface MethodSymbolTable : SymbolTable {
  /** Local variables declared in the current scope. */
	NSMutableArray * localVariables;
  /** Arguments in the current scope. */
  NSMutableArray * arguments;
}
/**
 * Initialise the symbol table with local variables and arguments.
 */
- (id) initWithLocals:(NSMutableArray*)locals 
                 args:(NSMutableArray*)args;
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
 * Symbol table for a block.
 */
@interface BlockSymbolTable : MethodSymbolTable {
  /** Bound variables in this block.  References to these are promoted to
   * indirect references via the block object. */
  NSMutableDictionary *promotedVars;
}
/**
 * The names of all of the external variables referenced in this scope.
 */
- (NSArray*) promotedVars;
/**
 * The scope of an externally-referenced variable.
 */
- (SymbolScope) scopeOfExternal:(NSString*)aSymbol;
/**
 * The location inside the block object for indirect access to promoted
 * variables.
 */
- (id) promotedLocationOfSymbol:(NSString*)aName;
/**
 * Specify the location inside the block object for indirect access to promoted
 * variables.
 */
- (void) promoteSymbol:(NSString*)aName toLocation:(id)aLocation;
@end

/**
 * Symbol table for an object.  Contains instance variables.
 */
@interface ObjectSymbolTable : SymbolTable {
  /** Instance variables for this object. */
	NSMapTable * instanceVariables;
  /** Next offset at which an ivar can be added. */
	int nextOffset;
}
/**
 * Initialise for the specified class.  All instance variables in the specified
 * class will be imported.
 */
- (SymbolTable*) initForClass:(Class)aClass;
@end
