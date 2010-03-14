@interface BlockContext : NSObject {
@public
	BlockContext *parent;
	int count;
	char **symbolTable;
	id objects[0];
}
@end

@interface StackContext : BlockContext {}
@end

@interface RetainedStackContext : StackContext {}
@end
