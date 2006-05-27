// We need this so the runtime will know the correct method signatures

#ifdef GNUSTEP
  #include <AppKit/AppKit.h>
#else
  #import <Cocoa/Cocoa.h>
#endif


@interface MethodDeclarations : NSObject 
{
}

- (int)browser:(NSBrowser *)sender numberOfRowsInColumn:(int)column;

@end
