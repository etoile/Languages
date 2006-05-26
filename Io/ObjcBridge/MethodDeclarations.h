// We need this so the runtime will know the correct method signatures

#import <Cocoa/Cocoa.h>


@interface MethodDeclarations : NSObject 
{
}

- (int)browser:(NSBrowser *)sender numberOfRowsInColumn:(int)column;

@end
