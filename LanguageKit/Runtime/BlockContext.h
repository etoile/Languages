@interface BlockContext : NSObject {
@public
	BlockContext *parent;
	int count;
	char **symbolTable;
	id objects[0];
}
- (BOOL)setValue: (id)aValue forSymbol: (NSString*)aSymbol;
- (id)valueForSymbol: (NSString*)aSymbol;
@end

@interface StackContext : BlockContext {}
@end

@interface RetainedStackContext : StackContext {}
@end
