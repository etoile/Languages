#import <EtoileFoundation/EtoileFoundation.h>
#import <LanguageKit/Runtime/BlockClosure.h>

@interface BlockClosure (ExceptionHandling)
- (id) onException:(NSString*)exceptionName do:(BlockClosure*)handler;
@end
