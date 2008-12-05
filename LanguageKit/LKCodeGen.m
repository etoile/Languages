typedef int bool;
#import "LLVMCodeGen.h"
#import "LKSymbolTable.h"
#include <objc/objc-api.h>

@implementation LLVMCodeGen 
+ (void) initialize
{
	if (self == [LLVMCodeGen class])
	{
		const char *path;
		NSFileManager *f = [NSFileManager defaultManager];
		if ([f fileExistsAtPath:@"MsgSendSmallInt.bc"])
		{
			path = "MsgSendSmallInt.bc";
		}
		else 
		{
			path = [[[NSBundle bundleForClass:self] 
				pathForResource:@"MsgSendSmallInt" ofType:@"bc"] UTF8String];
		}
		NSAssert(path, @"Unable to find the location of MsgSendSmallInt.bc.  This must be in either the current working directory or in the Resources directory of the SmalltalkKit framework installed on your system.");
		LLVMinitialise(path);
	}
}
- (void) startModule
{
	Builder = newModuleBuilder(NULL);
}

- (void) endModule
{
	Compile(Builder);
}

- (void) createSubclass:(NSString*)aClass
            subclassing:(NSString*)aSuperclass
          withCvarNames:(const char**)cVarNames 
                  types:(const char**)cVarTypes
          withIvarNames:(const char**)iVarNames 
                  types:(const char**)iVarTypes
                offsets:(int*)offsets
{
	int supersize = 0;
	Class sup = NSClassFromString(aSuperclass);
	if (Nil != sup)
	{
		supersize = sup->instance_size;
	}
	else
	{
		LKObjectSymbolTable * symbols = (LKObjectSymbolTable*)
			[LKObjectSymbolTable symbolTableForNewClassNamed:aSuperclass];
		if (nil != symbols)
		{
			supersize = [symbols instanceSize];
		}
	}
	BeginClass(Builder, [aClass UTF8String], [aSuperclass UTF8String], 
			cVarNames, cVarTypes, iVarNames, iVarTypes, offsets, supersize);
}
- (void) endClass
{
	EndClass(Builder);
}
- (void) createCategoryOn:(NSString*)aClass
                    named:(NSString*)aCategory
{
	BeginCategory(Builder, [aClass UTF8String], [aClass UTF8String]);
}
- (void) endCategory
{
	EndCategory(Builder);
}

- (void) beginClassMethod:(const char*) aName
                withTypes:(const char*)types
                   locals:(unsigned)locals
{
	BeginClassMethod(Builder, aName, types, locals);
}

- (void) beginInstanceMethod:(const char*) aName
                   withTypes:(const char*)types
                      locals:(unsigned)locals
{
	BeginInstanceMethod(Builder, aName, types, locals);
}

- (void*) sendMessage:(const char*)aMessage
                types:(const char*)types
             toObject:(void*)receiver
             withArgs:(void**)argv
                count:(unsigned)argc
{
  return MessageSendId(Builder, receiver, aMessage, types, (LLVMValue*)argv,
      argc);
}
- (void*) sendSuperMessage:(const char*)sel
                     types:(const char*)seltypes
                  withArgs:(void**)argv
                     count:(unsigned)argc
{
	return MessageSendSuper(Builder, sel, seltypes, (LLVMValue*)argv, argc);
}
- (void*) sendMessage:(const char*)aMessage
                types:(const char*)types
                   to:(void*)receiver
             withArgs:(void**)argv
                count:(unsigned)argc
{
  return MessageSend(Builder, receiver, aMessage, types, (LLVMValue*)argv,
      argc);
}
- (void) storeValue:(void*)rval 
    inClassVariable:(NSString*) aClassVar
{
	StoreClassVar(Builder, [aClassVar UTF8String], rval);
}
- (void*) loadClassVariable:(NSString*) aSymbol
{
	return LoadClassVar(Builder, [aSymbol UTF8String]);
}
- (void) storeValue:(void*)aVal 
     inLocalAtIndex:(unsigned)index
lexicalScopeAtDepth:(unsigned) scope
{
	StoreValueInLocalAtIndex(Builder, aVal, index, scope);
}
- (void) storeValue:(void*)aVal inLocalAtIndex:(unsigned)index
{
	StoreValueInLocalAtIndex(Builder, aVal, index, 0);
}
- (void) storeValue:(void*)aValue
              ofType:(NSString*)aType
            atOffset:(unsigned)anOffset
          fromObject:(void*)anObject
{
	StoreValueOfTypeAtOffsetFromObject(Builder, aValue, [aType UTF8String],
			anOffset, anObject);
}
- (void*) loadValueOfType:(NSString*)aType
                 atOffset:(unsigned)anOffset
               fromObject:(void*)anObject
{
	return LoadValueOfTypeAtOffsetFromObject(Builder, [aType UTF8String],
			anOffset, anObject);
}
- (void) beginBlockWithArgs:(unsigned)args
					 locals:(unsigned)locals
{
	BeginBlock(Builder, args, locals);
}
- (void*) endBlock
{
	return EndBlock(Builder);
}
- (void) endMethod
{
	EndMethod(Builder);
}

- (void) setReturn:(void*)aValue
{
	SetReturn(Builder, aValue);
}
- (void) blockReturn:(void*)aValue
{
	SetBlockReturn(Builder, aValue);
}
- (void*) loadBlockVarAtIndex:(unsigned)index
					   offset:(unsigned)offset
{
	return LoadBlockVar(Builder, index, offset);
}
- (void) storeValue:(void*) value
  inBlockVarAtIndex:(unsigned)index
             offset:(unsigned)offset
{
	StoreBlockVar(Builder, value, index, offset);
}
- (void*) loadSelf
{
	return LoadSelf(Builder);
}

- (void*) loadLocalAtIndex:(unsigned)index
	   lexicalScopeAtDepth:(unsigned) scope
{
	return LoadLocalAtIndex(Builder, index, scope);
}
- (void*) loadArgumentAtIndex:(unsigned) index
		  lexicalScopeAtDepth:(unsigned) scope
{
	return LoadArgumentAtIndex(Builder, index, scope);
}
- (void*) loadLocalAtIndex:(unsigned)index
{
	return LoadLocalAtIndex(Builder, index, 0);
}
- (void*) loadArgumentAtIndex:(unsigned) index
{
	return LoadArgumentAtIndex(Builder, index, 0);
}
- (void*) loadClass:(NSString*)aClass
{
	return LoadClass(Builder, [aClass UTF8String]);
}
- (void*) intConstant:(NSString*)aString
{
	return IntConstant(Builder, [aString UTF8String]);
}
- (void*) stringConstant:(NSString*)aString
{
	return StringConstant(Builder, [aString UTF8String]);
}
- (void*) nilConstant
{
	return NilConstant();
}
- (void*) comparePointer:(void*)lhs to:(void*)rhs
{
	return ComparePointers(Builder, lhs, rhs);
}
- (void) dealloc
{
	freeModuleBuilder(Builder);
	[super dealloc];
}
- (void*) generateConstantSymbol:(NSString*)aSymbol
{
	return SymbolConstant(Builder, [aSymbol UTF8String]);
}
@end
@interface LLVMStaticCodeGen : LLVMCodeGen {
	NSString *outFile;
}
- (id) initWithFile:(NSString*)file;
@end
@implementation LLVMStaticCodeGen
- (id) initWithFile:(NSString*)file
{
	SELFINIT;
	ASSIGN(outFile, file);
	return self;
}
- (void) endModule
{
	EmitBitcode(Builder, (char*)[outFile UTF8String], NO);
}
- (void) startModule
{
	Builder = newStaticModuleBuilder(NULL);
}
@end
id <LKCodeGenerator> defaultJIT(void)
{
	return [LLVMCodeGen new];
}
id <LKCodeGenerator> defaultStaticCompilterWithFile(NSString* outFile)
{
	return [[LLVMStaticCodeGen alloc] initWithFile:outFile];
}
