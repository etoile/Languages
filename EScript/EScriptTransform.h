#import <LanguageKit/LanguageKit.h>

@interface EScriptHoistIvars : LKASTVisitor @end
@interface EScriptHiddenClassTransform : LKASTVisitor
{
	NSMutableDictionary *newClasses;
}
@end
