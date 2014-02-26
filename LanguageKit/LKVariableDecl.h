#import <LanguageKit/LKAST.h>

@class LKToken;

@interface LKVariableDecl : LKAST {
	LKToken *variableName;
}
+ (LKVariableDecl*) variableDeclWithName:(LKToken*) declName;
- (NSString*)name;
@end
