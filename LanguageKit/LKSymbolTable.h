#import <EtoileFoundation/EtoileFoundation.h>

@class LKAST;

/** 
 * Enumerated type representing the scope of a variable.
 */
typedef	enum 
{
	LKSymbolScopeInvalid = 0,
	LKSymbolScopeExternal, /** Bound variable referenced in a block */
	LKSymbolScopeArgument, /** Variable passed in to method / block */
	LKSymbolScopeLocal,    /** Variable declared in this lexical scope */
	LKSymbolScopeObject,   /** Instance variable */
	LKSymbolScopeClass,    /** Class variable */
	LKSymbolScopeGlobal,   /** Global (only class names) */
} LKSymbolScope;

/**
 * Enumeration describing qualifiers.
 */
typedef enum
{
	/**
	 * Strong reference.
	 */
	LKSymbolQualifierStrong = 0,
	/**
	 * Unsafe unretained reference.  No write barriers should be emitted for
	 * this variable.
	 */
	LKSymbolQualifierUnretained = 2,
	/**
	 * Zeroing weak reference.  Weak read and write barriers should be emitted.
	 */
	LKSymbolQualifierWeak = 1
} LKSymbolQualifier;

/**
 * LKSymbol wraps entries in a symbol table.
 */
@interface LKSymbol : NSObject
/** The name of this symbol. */
@property (nonatomic, retain) NSString *name;
/** The Objective-C type encoding of this variable. */
@property (nonatomic, retain) NSString *typeEncoding;
/** The AST node representing the variable */
@property (nonatomic, assign) id owner;
/** The scope of this symbol. */
@property (nonatomic) LKSymbolScope scope;
/** Index of this symbol.  */
@property (nonatomic, assign) NSInteger index;
/**
 * The number of block scopes that reference this variable.  This does not
 * include the scope in which it is declared.
 */
@property (nonatomic) NSUInteger referencingScopes;
@end

/**
 * Symbol table.  Base class, with subclasses for each scope.
 */
@interface LKSymbolTable : NSObject {
}
/** The AST node (class, block, method) that contains these declarations. */
@property (assign, nonatomic) LKAST *declarationScope;
/** The parent scope for this symbol table. */
@property (assign, nonatomic) LKSymbolTable *enclosingScope;
/** The scope of this symbol table. */
@property (nonatomic) LKSymbolScope tableScope;
/**
 * The symbols stored in this symbol table, indexed by name.
 */
@property (nonatomic, retain) NSMutableDictionary *symbols;
/**
 * Returns the symbol table for a class.  If the class exists, then this will
 * be populated with its instance variables.  If not, then it will be empty.
 */
+ (LKSymbolTable*)symbolTableForClass: (NSString*)aClassName;
/**
 * Returns the symbol table for a class.  If the class exists, then this will
 * be populated with its instance variables. If not, then it return nil.
 */
+ (LKSymbolTable*)lookupTableForClass: (NSString*)aClassName;
/**
 * Add a symbol to this table.
 */
- (void)addSymbol: (LKSymbol*)aSymbol;
/**
 * Adds a set of symbols in one call.  The symbols have the specified scope and
 * initially have no type assigned to them.
 */
- (void)addSymbolsNamed: (NSArray*)anArray ofKind: (LKSymbolScope)kind;
/**
 * Looks up the symbol for a specified name.
 */
- (LKSymbol*)symbolForName: (NSString*)aName;
/**
 * Returns all of the symbols in this table that represent arguments.
 */
- (NSArray*)arguments;
/**
 * Returns all of the symbols in this table that represent local variables.
 */
- (NSArray*)locals;
/**
 * Returns all of the symbols in this table that represent class variables.
 */
- (NSArray*)classVariables;
/**
 * Returns all of the symbols in this table that represent instance variables.
 */
- (NSArray*)instanceVariables;
/**
 * Returns all of the symbols in this table that represent external symbols..
 */
- (NSArray*)byRefVariables;
@end

/**
 * Creates an NSString from a string returned by the runtime.  These strings
 * are guaranteed to persist for the duration of the program, so there's no
 * need to copy the data.
 */
__attribute__((unused))
static NSString *NSStringFromRuntimeString(const char *cString)
{
	return [[[NSString alloc] initWithBytesNoCopy: (char*)cString
	                                       length: strlen(cString)
	                                     encoding: NSUTF8StringEncoding
	                                 freeWhenDone: NO] autorelease];
}
