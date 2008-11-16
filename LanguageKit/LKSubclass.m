#import "LKSubclass.h"

@implementation LKSubclass
+ (id) subclassWithName:(NSString*)aName
             superclass:(NSString*)aClass
                  ivars:(NSArray*)anIvarList
                methods:(NSArray*)aMethodList
{
	return [[[self alloc] initWithName: aName
	                        superclass: aClass
	                             ivars: anIvarList
	                           methods: aMethodList] autorelease];
}
- (id) initWithName:(NSString*)aName
         superclass:(NSString*)aClass
              ivars:(NSArray*)anIvarList
            methods:(NSArray*)aMethodList
{
	SELFINIT;
	ASSIGN(classname, aName);
	ASSIGN(superclass, aClass);
	ASSIGN(ivars, anIvarList);
	ASSIGN(methods, aMethodList);
	return self;
}
- (void) check
{
	Class SuperClass = NSClassFromString(superclass);
	//Construct symbol table.
	if (Nil == SuperClass)
	{
		symbols = 
			[[LKObjectSymbolTable symbolTableForNewClassNamed:superclass] copy];
		if (symbols == nil)
		{
			[NSException raise:@"SemanticError"
						format:@"Unable to find superclass %@ for %@", 
				superclass, classname];
		}
	}
	else
	{
		symbols = [[LKObjectSymbolTable alloc] initForClass:SuperClass];
	}
	if (Nil != NSClassFromString(classname))
	{
		[NSException raise:@"SemanticError"
					format:@"Can not create new class %@ - a class of this name already exists.", classname];
	}
	//Construct symbol table.
	FOREACH(ivars, ivar, NSString*)
	{
		[symbols addSymbol:ivar];
	}
    [(LKObjectSymbolTable*)symbols registerNewClass:classname];
	FOREACH(methods, method, LKAST*)
	{
		[method setParent:self];
		[method check];
	}
}
- (NSString*) description
{
	NSMutableString *str = [NSMutableString stringWithFormat:@"%@ subclass: %@ [ \n",
   		superclass, classname];
	if ([ivars count])
	{
		[str appendString:@"| "];
		FOREACH(ivars, ivar, NSString*)
		{
			[str appendFormat:@"%@ ", ivar];
		}
		[str appendString:@"|\n"];
	}
	FOREACH(methods, method, LKAST*)
	{
		[str appendString:[method description]];
	}
	[str appendString:@"\n]"];
	return str;
}
- (void*) compileWith:(id<LKCodeGenerator>)aGenerator
{
	const char *ivarNames[[ivars count] + 1];
	const char *ivarTypes[[ivars count] + 1];
	int ivarOffsets[[ivars count] + 1];
	for (int i=0; i<[ivars count]; i++)
	{
		ivarNames[i] = [[ivars objectAtIndex: i] UTF8String];
		ivarTypes[i] = "@";
		ivarOffsets[i] = [symbols offsetOfIVar: [ivars objectAtIndex: i]];
	}
	ivarNames[[ivars count]] = NULL;
	ivarTypes[[ivars count]] = NULL;
	ivarOffsets[[ivars count]] = 0;

	[aGenerator createSubclass:classname
                   subclassing:superclass
                 withIvarNames:ivarNames
                         types:ivarTypes
                       offsets:ivarOffsets];
	FOREACH(methods, method, LKAST*)
	{
		[method compileWith:aGenerator];
	}
	// Create dealloc method
	if ([ivars count] > 0)
	{
		const char* deallocty = sel_get_type(sel_get_any_typed_uid("dealloc"));

		[aGenerator beginInstanceMethod:"dealloc" withTypes:deallocty locals:0];
		void *selfptr = [aGenerator loadSelf];
		const char* releasety = sel_get_type(sel_get_any_typed_uid("release"));
		for (unsigned i=0 ; i<[ivars count] ; i++)
		{
			void *ivar = [aGenerator loadValueOfType:@"@"
			                                atOffset:ivarOffsets[i]
			                              fromObject:selfptr];
			[aGenerator sendMessage:"release"
			                  types:releasety
			               toObject:ivar
						   withArgs:NULL
							  count:0];
		}
		[aGenerator sendSuperMessage:"dealloc"
							   types:deallocty
							withArgs:NULL
							   count:0];
		[aGenerator endMethod];
	}
	[aGenerator endClass];
	if ([[LKAST code] objectForKey: classname] == nil)
	{
		[[LKAST code] setObject: [NSMutableArray array] forKey: classname];
	}
	[[[LKAST code] objectForKey: classname] addObject: self];
	return NULL;
}
@end
