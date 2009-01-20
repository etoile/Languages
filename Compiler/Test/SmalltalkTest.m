#import <EtoileFoundation/EtoileFoundation.h>
#import <LanguageKit/LKCompiler.h>
#import <AppKit/AppKit.h>
#import <UnitKit/UnitKit.h>

@interface UKWrapper : NSObject
@end
@implementation UKWrapper
- (void) true: (BOOL) x
{
	UKTrue(x);
}
- (void) intEqual: (int) x to: (int) y
{
	UKIntsEqual(x, y);
}
- (void) stringEqual: (NSString *) x to: (NSString *) y
{
        UKStringsEqual(x, y);
}
- (void) objectEqual: (id) x to: (id) y
{
        UKObjectsEqual(x, y);
}
@end


@interface SmalltalkTest : NSObject <UKTest>
@end

@implementation SmalltalkTest

- (void) testStringReturn
{
	UKTrue([LKCompiler loadScriptInBundle: [NSBundle bundleForClass: [self class]] 
	                                named: @"TestStringReturn.st"]);
	UKStringsEqual([[NSClassFromString(@"TestStringReturn") new] test], @"hello");
}

@end
