#import "LKSymbolTable.h"
#import "LKCodeGen.h"

/**
 * Root class for AST nodes.  Every node in the Smalltalk abstract syntax tree
 * inherits from this.  It stores the parent, allowing navigation up the tree,
 * and a pointer to the symbol table for this scope.  
 */
@interface LKAST : NSObject {
	/** Node above this one in the tree. */
	LKAST * parent;
	/** Is this a parenthetical expression?  Avoids creating explicit nodes for
	* parenthetical expressions.  Might not be sensible.*/
	BOOL isBracket;
	/** Symbol table for this context.  If no new symbols are defined, this is a
	* pointer to the parent's symbol table. */
	LKSymbolTable * symbols;
}
/**
 * Returns the AST nodes available at runtime for subclasses and categories.
 */
+ (NSMutableDictionary *) code;
/**
 * Initialise a new AST node with the specified symbol table.
 */
- (id) initWithSymbolTable:(LKSymbolTable*)aSymbolTable;
/**
 * Set whether the AST node represents a bracketed expression.
 */
- (void) setBracketed:(BOOL)aFlag;
/**
 * Returns whether the AST node represents a bracketed expression.
 */
- (BOOL) isBracketed;
/**
 * Sets the parent of this node.  Also sets connections between the nodes'
 * symbol tables.
 */
- (void) setParent:(LKAST*)aNode;
/**
 * Returns the symbol table for this node.
 */
- (LKSymbolTable*) symbols;
/**
 * Prints the syntax tree from this node.  Use only for debugging.
 */
- (void) print;
/**
 * Performs semantic analysis on this node and all of its children.
 */
- (void) check;
/**
 * Resolve the scope of a symbol used inside a block (closure) representing a
 * variable declared outside.
 */
- (void) resolveScopeOf:(NSString*)aSymbol;
/**
 * Checks whether the specified node is a valid LValue (i.e. something that can
 * be assigned to).
 */
- (void) checkLValue:(id) aChild;
/**
 * Compile this AST node with the specified code generator.
 */
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator;
@end

#define SAFECAST(type, obj) ([obj isKindOfClass:[type class]] ? (type*)obj : ([NSException raise:@"InvalidCast" format:@"Can not cast %@ to %s", obj, #type], (type*)nil))
