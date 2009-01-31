#import <EtoileFoundation/EtoileFoundation.h>
#import <LanguageKitRuntime/BlockClosure.h>

@interface BlockClosure (ExceptionHandling)
- (id) onException:(NSString*)exceptionName do:(BlockClosure*)handler;
@end
