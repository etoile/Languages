#include <EtoileFoundation/EtoileFoundation.h>
#import "SymbolTable.h"
#import "Parser.h"


int main(void)
{
	[NSAutoreleasePool new];
	Parser * p = [[Parser alloc] initWithClass:[Test class]];
  [[p parseString:	@"doStuff | a b c| \na bar: [ :x | b do:c.].\nb := fish eat:banana with:mouth.\n foo bar:wibble fish:banana."] print];

	return 0;
}
