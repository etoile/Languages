#import "EScriptTransform.h"

@implementation EScriptHoistIvars
- (id)visitVariableDecl: (LKVariableDecl*)decl
{
	return decl;
	LKMethod *method = (LKMethod*)[decl parent];
	LKSubclass *cls = (LKSubclass*)[method parent];
	// Make sure that this is a decl in a method.
	if (![cls isKindOfClass: [LKSubclass class]])
	{
		return decl;
	}
	NSString * name = [decl name];

	[[[method symbols] symbols] removeObject: name];
	[cls addInstanceVariable: name];
	return nil;
}
@end

@implementation EScriptHiddenClassTransform
- (id)init
{
	SUPERINIT;
	newClasses = [NSMutableDictionary new];
	return self;
}
- (void)dealloc
{
	[newClasses release];
	[super dealloc];
}
#define EXPECT_CLASS(x, cls) \
	if (![(x) isKindOfClass: [cls class]])\
	{\
		return msg;\
	}
- (id)visitMessageSend: (LKMessageSend*)msg
{
	return msg;
	// Check that this message is setting a slot
	if (![@"setValue:forKey:" isEqualToString: [msg selector]])
	{
		return msg;
	}
	NSArray *args = [msg arguments];
	// Check that we are setting a constant string key
	EXPECT_CLASS([args objectAtIndex: 1], LKStringLiteral);
	NSString *className = [[args objectAtIndex: 1] stringValue];
	EXPECT_CLASS([msg parent], LKBlockExpr);
	EXPECT_CLASS([[msg parent] parent], LKAssignExpr);
	// If the value being assigned is a block, then this is a method.  If not,
	// then it is an ivar.  If it's a method, then it is also an ivar
	// containing the block, just to confuse matters.
	BOOL isMethod = 
		[[[msg arguments] objectAtIndex: 0] isKindOfClass: [LKBlockExpr class]];

	NSLog(@"Parent: %@", [[msg parent] class]);
	NSLog(@"Parent parent: %@", [[[msg parent] parent] class]);
	LKSubclass *cls = [newClasses objectForKey: className];
	if (nil == cls)
	{
		cls = [LKSubclass subclassWithName: className
		                   superclassNamed: @"EScriptObject"
		                             cvars: nil
		                             ivars: nil
		                           methods: nil];
		[newClasses setObject: cls forKey: className];
	}
	
	return msg;
}
@end
