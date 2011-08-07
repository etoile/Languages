#import <EtoileFoundation/EtoileFoundation.h>
#import "Runtime/BigInt.h"
#import "Runtime/BoxedFloat.h"
#import "Runtime/Symbol.h"
#import "LanguageKit/LanguageKit.h"
#import "LKInterpreter.h"
#import "LKInterpreterRuntime.h"
#import <EtoileFoundation/runtime.h>
#include <math.h>

NSString *LKInterpreterException = @"LKInterpreterException";

static NSMutableDictionary *LKClassVariables;
static NSMutableDictionary *LKMethodASTs;

LKMethod *LKASTForMethod(Class cls, NSString *selectorName)
{
	BOOL isClassMethod = class_isMetaClass(cls);
	LKMethod *ast = nil;
	do
	{
		ast = [LKMethodASTs valueForKey:
			[NSString stringWithFormat: @"%s%c%@", 
				class_getName(cls), isClassMethod ? '+' : '-', selectorName]];
		cls = class_getSuperclass(cls);
	} while (ast == nil && cls != nil);
	return ast;
}

static void StoreASTForMethod(NSString *classname, BOOL isClassMethod,
                              NSString *selectorName, LKMethod *method)
{
	[LKMethodASTs setValue: method
	                forKey: [NSString stringWithFormat: @"%@%@%@", 
	                                         classname,
                                                 isClassMethod ? @"+" : @"-",
	                                         selectorName]];
}


@interface LKBlockReturnException : NSException
{
}
+ (void)raiseWithValue: (id)returnValue;
- (id)returnValue;
@end
@implementation LKBlockReturnException
+ (void)raiseWithValue: (id)returnValue
{
	[[LKBlockReturnException exceptionWithName: LKSmalltalkBlockNonLocalReturnException
	                                    reason: @""
	                                  userInfo: D(returnValue, @"returnValue", nil)] raise];
}
- (id)returnValue
{
	return [[self userInfo] valueForKey: @"returnValue"];
}
@end

@implementation LKInterpreterContext
- (id) initWithSymbolTable: (LKSymbolTable*)aTable
                    parent: (LKInterpreterContext*)aParent
{
	SUPERINIT;
	ASSIGN(parent, aParent);
	ASSIGN(symbolTable, aTable);
	objects = [NSMutableDictionary new];
	return self;
}
- (void) dealloc
{
	[objects release];
	[symbolTable release];
	[parent release];
	[super dealloc];
}
- (LKInterpreterContext *) parent
{
	return parent;
}
- (LKSymbolTable *) symbolTable
{
	return symbolTable;
}
- (void) setValue: (id)value forSymbol: (NSString*)symbol
{
	[objects setObject: value forKey: symbol];
}
- (id) valueForSymbol: (NSString*)symbol
{
	return [objects objectForKey: symbol];
}
- (LKInterpreterVariableContext)contextForSymbol: (LKSymbol*)symbol
{
	LKInterpreterVariableContext context;
	context.context = self;
	context.scope = [symbol scope];
	if (context.scope == LKSymbolScopeExternal)
	{
		return [parent contextForSymbol: symbol];
	}
	return context;
}
- (id)selfObject
{
	if (nil != parent)
	{
		return [parent selfObject];
	}
	return [objects objectForKey: @"self"];
}
@end


@implementation LKAST (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	[self subclassResponsibility: _cmd];
	return nil;
}
@end

@interface LKArrayExpr (LKInterpreter)
@end
@implementation LKArrayExpr (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	unsigned int count = [elements count];
	__unsafe_unretained id interpretedElements[count];
	for (unsigned int i=0; i<count; i++)
	{
		[[elements objectAtIndex: i] retain];
		interpretedElements[i] =
			[(LKAST*)[elements objectAtIndex: i] interpretInContext: context];
		[[elements objectAtIndex: i] release];
	}
	return [NSMutableArray arrayWithObjects: interpretedElements count: count];
}
@end

@interface LKAssignExpr (LKInterpreter)
@end
@implementation LKAssignExpr (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)currentContext
{
	[expr retain];
	id rvalue = [expr interpretInContext: currentContext];
	[expr release];

	LKInterpreterVariableContext context = [currentContext contextForSymbol: target->symbol];

	NSString *symbolName = [target->symbol name];
	switch (context.scope)
	{
		case LKSymbolScopeLocal:
		{
			[context.context setValue: rvalue
			                forSymbol: symbolName];
			break;
		}
		case LKSymbolScopeObject:
		{
			LKSetIvar([context.context selfObject], symbolName, rvalue);
			break;
		}
		case LKSymbolScopeClass:
		{
			LKAST *p = [self parent];
			while (NO == [p isKindOfClass: [LKSubclass class]] && nil != p)
			{
				p = [p parent];
			}
			[(LKSubclass*)p setValue: rvalue forClassVariable: symbolName];
			break;
		}
		default:
			NSAssert1(NO, @"Don't know how to assign to %@", symbolName);
			break;
	}
	return rvalue;
}
@end


@implementation LKBlockExpr (LKInterpreter)
- (id)executeWithArguments: (id*)args count: (int)count inContext: (LKInterpreterContext*)context
{
	NSArray *arguments = [[self symbols] arguments];
	for (int i=0; i<count; i++)
	{
		[context setValue: args[i]
		        forSymbol: [[arguments objectAtIndex: i] name]];
	}

	id result = nil;
	FOREACH(statements, statement, LKAST*)
	{
		[statement retain];
		result = [statement interpretInContext: context];
		// FIXME: should be in @finally
		[statement release];
	}
	return result;
}
- (id)interpretInContext: (LKInterpreterContext*)parentContext
{
	int count = [[[self symbols] arguments] count];
	return [Block_copy(^(id arg0, ...) {
		LKInterpreterContext *context = [[LKInterpreterContext alloc]
		            initWithSymbolTable: [self symbols]
		                         parent: parentContext];
		__unsafe_unretained id params[count];
		
		va_list arglist;
		va_start(arglist, arg0);
		if (count > 0)
		{
			params[0] = arg0;
		}
		for (int i = 1; i < count; i++)
		{
			params[i] = (id) va_arg(arglist, __unsafe_unretained id);
		}
		va_end(arglist);
		
		@try
		{
			return [self executeWithArguments: params
			                            count: count
			                        inContext: context];
		}
		@finally
		{
			[context release];
		}
		return nil;
	}) autorelease];
}
@end

@interface LKCategoryDef (LKInterpreter)
@end
@implementation LKCategoryDef (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	Class cls = NSClassFromString(classname);
	if (cls == Nil)
	{
		[NSException raise: LKInterpreterException
		            format: @"Tried to create category %@ on non-existing class %@",
		                    categoryName, classname];
	}
	FOREACH(methods, method, LKMethod*)
	{
		BOOL isClassMethod = [method isKindOfClass: [LKClassMethod class]];
		NSString *methodName = [[method signature] selector];
		SEL sel = NSSelectorFromString(methodName);
		//FIXME: check the superclass type explicitly
		const char *type = [[[(LKModule*)[self parent] typesForMethod: methodName] objectAtIndex: 0] UTF8String];
		Class destClass = isClassMethod ? object_getClass(cls) : cls;
		class_replaceMethod(destClass, sel, LKInterpreterMakeIMP(destClass, type), type);
		StoreASTForMethod(classname, isClassMethod, methodName, method);
	}
	return nil;
}
@end


@interface LKComment (LKInterpreter)
@end
@implementation LKComment (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	return nil;
}
@end

@interface LKCompare (LKInterpreter)
@end
@implementation LKCompare (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	[lhs retain];
	id lhsInterpreted = [lhs interpretInContext: context];
	[lhs release];
	
	[rhs retain];
	id rhsInterpreted = [rhs interpretInContext: context];
	[rhs release];

	return [BigInt bigIntWithLong: lhsInterpreted == rhsInterpreted];
}
@end

@interface LKDeclRef (LKInterpreter)
@end
@implementation LKDeclRef (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)currentContext
{
	LKInterpreterVariableContext context = [currentContext contextForSymbol: symbol];
	NSString *symbolName = [symbol name];
	switch (context.scope)
	{
		case LKSymbolScopeObject:
		{
			return LKGetIvar([currentContext selfObject], symbolName); 
		}
		case LKSymbolScopeLocal:
		case LKSymbolScopeArgument:
		{
			return [context.context valueForSymbol: symbolName];
		}
		case LKSymbolScopeBuiltin:
			if ([symbolName isEqualToString: @"self"] || [symbolName isEqualToString: @"super"])
			{
				return [context.context selfObject];
			}
			else if ([symbolName isEqualToString: @"nil"] || [symbolName isEqualToString: @"Nil"])
			{
				return nil;
			}
		case LKSymbolScopeGlobal:
			return NSClassFromString(symbolName);
		case LKSymbolScopeClass:
		{
			LKAST *p = [self parent];
			while (NO == [p isKindOfClass: [LKSubclass class]] && nil != p)
			{
				p = [p parent];
			}
			return [(LKSubclass*)p valueForClassVariable: symbolName];
		}
		default:
			break;
	}
	NSAssert(NO, @"Can't interpret decl ref");
	return nil;
}
@end

@interface LKIfStatement (LKInterpreter)
@end
@implementation LKIfStatement (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	id result = nil;
	[condition retain];
	NSArray *statements = [[condition interpretInContext: context] boolValue] ?
		thenStatements : elseStatements;
	[condition release];
	FOREACH(statements, statement, LKAST*)
	{
		[statement retain];
		result = [statement interpretInContext: context];
		// FIXME: should be in @finally
		[statement release];
	}
	return result;
}
@end

@interface LKStringLiteral (LKInterpreter)
@end
@implementation LKStringLiteral (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	return value;
}
@end

@interface LKNumberLiteral (LKInterpreter)
@end
@implementation LKNumberLiteral (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	return [BigInt bigIntWithCString: [value UTF8String]];
}
@end

@interface LKFloatLiteral (LKInterpreter)
@end
@implementation LKFloatLiteral (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	return [BoxedFloat boxedFloatWithCString: [value UTF8String]];
}
@end

@interface LKMessageSend (LKInterpreter)
@end
@implementation LKMessageSend (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context forTarget: (id)receiver
{
	NSString *receiverClassName = nil;
	if ([target isKindOfClass: [LKDeclRef class]] && 
		[[((LKDeclRef*)target)->symbol name] isEqualToString: @"super"])
	{
		LKAST *ast = [self parent];
		while (nil != ast && ![ast isKindOfClass: [LKSubclass class]])
		{
			ast = [ast parent];
		}
		receiverClassName = [(LKSubclass*)ast superclassname];
	}
	unsigned int argc = [arguments count];
	__unsafe_unretained id argv[argc];
	for (unsigned int i=0 ; i<argc ; i++)
	{
		LKAST *arg = [arguments objectAtIndex: i];
		arg = [arg retain];
		@try
		{
			argv[i] = [arg interpretInContext: context];
		}
		@finally
		{
			[arg release];
		}
	}
	return LKSendMessage(receiverClassName, receiver, selector, argc, argv);
}
- (id)interpretInContext: (LKInterpreterContext*)context
{
	[target retain];
	id result = [self interpretInContext: context
	                      forTarget: [(LKAST*)target interpretInContext: context]];
	[target release];
	return result;
}
@end


@interface LKMessageCascade (LKInterpreter)
@end
@implementation LKMessageCascade (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	id result = nil;
	[receiver retain];
	id target = [receiver interpretInContext: context];
	[receiver release];
	FOREACH(messages, message, LKMessageSend*)
	{
		[message retain];
		result = [message interpretInContext: context forTarget: target];
		// FIXME: should be in @finally
		[message release];
	}
	return result;
}
@end

@implementation LKMethod (LKInterpreter)
- (id)executeInContext: (LKInterpreterContext*)context
{
	id result = [context selfObject];
	@try
	{
		FOREACH([self statements], element, LKAST*)
		{
			@try
			{
				[element retain];
				[element interpretInContext: context];
			}
			@finally 
			{
				[element retain];
			}
		}
	}
	@catch (LKBlockReturnException *ret)
	{
		result = [ret returnValue];
	}
	return result;
}
- (id)executeWithReciever: (id)receiver arguments: (id*)args count: (int)count
{
	NSMutableArray *symbolnames = [NSMutableArray array];
	LKMessageSend *signature = [self signature];
	if ([signature arguments])
	{
		[symbolnames addObjectsFromArray: [signature arguments]];
	}
	[symbolnames addObjectsFromArray: [symbols locals]];
	
	LKInterpreterContext *context = [[LKInterpreterContext alloc]
							initWithSymbolTable: symbols
							             parent: nil];
	[context setValue: receiver forSymbol: @"self"];
	for (unsigned int i=0; i<count; i++)
	{
		[context setValue: args[i]
		        forSymbol: [[signature arguments] objectAtIndex: i]];
	}

	id result = nil;
	// FIXME: @try {
	result = [self executeInContext: context];
	// } @finally {
	[context release];
	// }

	return result;
}
@end


@interface LKModule (LKInterpreter)
@end
@implementation LKModule (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	FOREACH(classes, class, LKAST*)
	{
		[class retain];
		[class interpretInContext: context];
		[class release];
	}
	FOREACH(categories, category, LKAST*)
	{
		[category retain];
		[category interpretInContext: context];
		[category release];
	}
	return nil;
}
@end


@interface LKBlockReturn (LKInterpreter)
@end
@implementation LKBlockReturn (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	[ret retain];
	id value = [ret interpretInContext: context];
	[ret release];
	return value;
}
@end


@interface LKReturn (LKInterpreter)
@end
@implementation LKReturn (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	[ret retain];
	id value = [ret interpretInContext: context];
	[ret release];

	[LKBlockReturnException raiseWithValue: value];
	return nil;
}
@end


@implementation LKSubclass (LKInterpreter)

+ (void)initialize
{
	if (self == [LKSubclass class])
	{
		LKMethodASTs = [[NSMutableDictionary alloc] init];
		LKClassVariables = [[NSMutableDictionary alloc] init];
	}
}

- (void)setValue: (id)value forClassVariable: (NSString*)cvar
{
	if (nil == [LKClassVariables valueForKey: [self classname]])
	{
		[LKClassVariables setValue: [NSMutableDictionary dictionary]
		                    forKey: [self classname]];
	}
	[[LKClassVariables valueForKey: [self classname]] setValue: value
	                                                    forKey: cvar];
}

- (id)valueForClassVariable: (NSString*)cvar
{
	return [[LKClassVariables valueForKey: [self classname]] valueForKey: cvar];
}

static uint8_t logBase2(uint8_t x)
{
	uint8_t result = 0;
	while (x > 1)
	{
		result++;
		x = x >> 1;
	}
	return result;
}

- (id)interpretInContext: (LKInterpreterContext*)context
{
	// Make sure the superclass is interpreted first
	Class supercls = NSClassFromString(superclass);
	if (Nil == supercls)
	{
		FOREACH([(LKModule*)[self parent] allClasses], class, LKSubclass*)
		{
			if ([[class classname] isEqualToString: superclass])
			{
				[class retain];
				supercls = [class interpretInContext: context];
				[class release];
				break;
			}
		}
		if (Nil == supercls)
		{
			[NSException raise: LKInterpreterException
						format: @"Superclass %@ (of class %@) not found",
			                    superclass, classname];
		}
	}
	
	Class cls = NSClassFromString(classname);
	BOOL alreadyExists = (Nil != cls);
	if (!alreadyExists)
	{
		cls = objc_allocateClassPair(supercls, [classname UTF8String], 0);
	}
	else
	{
		NSLog(@"LKInterpreter: class %@ is already defined", cls);
	}

	FOREACH(ivars, ivar, NSString*)
	{
		class_addIvar(cls, [ivar UTF8String], sizeof(id), logBase2(__alignof__(id)), "@");
	}

	FOREACH(methods, method, LKMethod*)
	{
		BOOL isClassMethod = [method isKindOfClass: [LKClassMethod class]];
		NSString *methodName = [[method signature] selector];
		SEL sel = NSSelectorFromString(methodName);
		//FIXME: If overriding, check the superclass type explicitly
		const char *type = [[[(LKModule*)[self parent] typesForMethod: methodName] objectAtIndex: 0] UTF8String];
		Class destClass = isClassMethod ? object_getClass(cls) : cls;
		class_addMethod(destClass, sel, LKInterpreterMakeIMP(destClass, type), type);
		StoreASTForMethod(classname, isClassMethod, methodName, method);
	}
	
	if (!alreadyExists)
	{
		objc_registerClassPair(cls);
		[cls load];
	}	
	return cls;
}
@end
@implementation LKVariableDecl (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	[context setValue: nil forSymbol: (NSString*)variableName];
	return nil;
}
@end
@implementation LKLoop (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	// FIXME: @try for LKBreak support.
	BOOL cond = YES;
	
	for (LKAST *statement in initStatements)
	{
		[statement interpretInContext: context];
	}
	while (cond)
	{
		if (nil != preCondition)
		{
			cond = [[preCondition interpretInContext: context] boolValue];
			if (!cond) { break; }
		}
		for (LKAST *statement in statements)
		{
			[statement interpretInContext: context];
		}
		for (LKAST *statement in updateStatements)
		{
			[statement interpretInContext: context];
		}
		if (nil != postCondition)
		{
			cond = [[postCondition interpretInContext: context] boolValue];
		}
	}
	return nil;
}
@end


@interface LKSymbolRef (LKInterpreter)
@end
@implementation LKSymbolRef (LKInterpreter)
- (id)interpretInContext: (LKInterpreterContext*)context
{
	return [[Symbol SymbolForString: symbol] autorelease];
}
@end
