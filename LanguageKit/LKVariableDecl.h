#import "LKAST.h"

@class LKToken;

@interface LKVariableDecl : LKAST {
	LKToken *variableName;
}
+ (LKVariableDecl*) variableDeclWithName:(LKToken*) declName;
@end
