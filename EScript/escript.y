/*
Parser definition file.  This uses LEMON (from the SQLite project), a public
domain parser generator, to produce an Objective-C parser.
*/
%include {
#import <EtoileFoundation/EtoileFoundation.h>
#include <assert.h>
#import <LanguageKit/LanguageKit.h>
#import "EScriptParser.h"
#import "EScriptPreamble.h"
#import "EScriptTransform.h"
}
%name EScriptParse
%token_prefix TOKEN_
%token_type {id}
%extra_argument {EScriptParser *p}
%right IF ELSE.
%right EQ.
%nonassoc EQEQ.
%left EQEQEQ.
%nonassoc LT GT.
%right PLUSEQ MINUSEQ.
%right MULEQ DIVEQ MODEQ.
%left PLUS MINUS.
%left MUL DIV MOD.
%left DOT LBRACK.

file ::= module(M) script(S).
{
	// NSLog(@"%@", S);
	[M addClass:S];
	[p setAST:M];
	NSLog(@"Parsed AST: %@", M);
	[M check];
	[M visitWithVisitor: [[EScriptHoistIvars new] autorelease]];
	[M visitWithVisitor: [[EScriptHiddenClassTransform new] autorelease]];
	NSLog(@"Parsed AST: %@", M);
}

module(M) ::= module(O) LT LT pragma_dict(P) GT GT.
{
	[O addPragmas:P];
	M = O;
}
module(M) ::= .
{
	M = [LKModule module];
}

pragma_dict(P) ::= pragma_dict(D) COMMA ident(K) EQ pragma_value(V).
{
	[D setObject:V forKey:K];
	P = D;
}
pragma_dict(P) ::= ident(K) EQ pragma_value(V).
{
	P = [NSMutableDictionary dictionaryWithObject:V forKey:K];
}

pragma_value(V) ::= ident(W).  { V = W; }
pragma_value(V) ::= STRING(S). { V = S; }
pragma_value(V) ::= NUMBER(N). { V = N; }

script(S) ::= statement_list(L).
{
	id globals = [NSArray arrayWithObjects:@"Object", @"Array", nil];

	[L insertObject:[EScriptPreamble preamble] atIndex:0];
	// Ugly hack.  The +load method is being given a BOOL return type because
	// someone did something stupid somewhere, and we're crashing in the auto-unboxing.
	//[L addObject: [LKReturn returnWithExpr: [LKDeclRef referenceWithSymbol:@"nil"]]];

	//id m = [LKClassMethod methodWithSignature:[LKMessageSend messageWithSelectorName:@"load"]
	id m = [LKInstanceMethod methodWithSignature: [LKMessageSend messageWithSelectorName:@"run"]
	                                   locals:nil
	                               statements:L];

	//S = [LKSubclass subclassWithName:[[ETUUID UUID] stringValue]
	S = [LKSubclass subclassWithName: @"SmalltalkTool"
	                 superclassNamed:@"NSObject"
	                           cvars:globals
	                           ivars:nil
	                         methods:[NSArray arrayWithObject:m]];
}

statement_list(L) ::= statement_list(T) statement(S).
{
	[T addObject:S];
	L = T;
}
statement_list(L) ::= statement_list(T) COMMENT(C).
{
	[T addObject:[LKComment commentWithString:C]];
	L = T;
}
statement_list(L) ::= statement_list(T) FUNCTION ident(F)
                                        LPAREN  argument_list(A) RPAREN
                                        LBRACE statement_list(B) RBRACE.
{
	/* LanguageKit uses the result of the last statement as the return value
	   of the function. Simply reference nil to return that by default. */
	[B addObject:[LKDeclRef referenceWithSymbol:@"nil"]];
	[T addObject:[LKVariableDecl variableDeclWithName:F]];
	LKMessageSend *this = [LKMessageSend messageWithSelectorName: @"slotValueForKey:"];
	[this setTarget: [LKDeclRef referenceWithSymbol: @"blockContext"]];
	[this addArgument: [LKStringLiteral literalFromString: @"this"]];
	NSMutableArray *constructThis = [NSMutableArray array];
	[constructThis addObject: [LKVariableDecl variableDeclWithName: (LKToken*)@"this"]];
	[constructThis addObject: [LKAssignExpr assignWithTarget: [LKDeclRef referenceWithSymbol: @"this"]
	                                                    expr: this]];
	NSArray *storeContext = [NSArray arrayWithObject: [LKAssignExpr assignWithTarget: [LKDeclRef referenceWithSymbol: @"this"]
	                                                                            expr: [LKDeclRef referenceWithSymbol: @"blockContext"]]];
	[constructThis addObject: [LKIfStatement ifStatementWithCondition: [LKCompare comparisonWithLeftExpression: [LKDeclRef referenceWithSymbol: @"this"]
	                                                                                           rightExpression: [LKDeclRef referenceWithSymbol: @"nil"]]
	                                                             then: storeContext
	                                                             else: nil]];
	[constructThis addObjectsFromArray: B];
	[T addObject:
		[LKAssignExpr assignWithTarget:[LKDeclRef referenceWithSymbol:F]
		                          expr:[LKBlockExpr blockWithArguments:A
		                                                        locals:nil
		                                                    statements:constructThis]]];
	L = T;
}
statement_list(L) ::= statement_list(T) VAR declarations(A) SEMI.
{
	[T addObjectsFromArray:A];
	L = T;
}
statement_list(L) ::= .
{
	L = [NSMutableArray array];
}

declarations(L) ::= declarations(T) COMMA ident(V).
{
	[T addObject:[LKVariableDecl variableDeclWithName:V]];
	L = T;
}
declarations(L) ::= declarations(T) COMMA ident(V) EQ expression(E).
{
	[T addObject:[LKVariableDecl variableDeclWithName:V]];
	[T addObject:
		[LKAssignExpr assignWithTarget:[LKDeclRef referenceWithSymbol:V]
		                          expr:E]];
	L = T;
}
declarations(L) ::= ident(V).
{
	L = [NSMutableArray arrayWithObject:
			[LKVariableDecl variableDeclWithName:V]];
}
declarations(L) ::= ident(V) EQ expression(E).
{
	L = [NSMutableArray arrayWithObjects:
			[LKVariableDecl variableDeclWithName:V],
			[LKAssignExpr assignWithTarget:[LKDeclRef referenceWithSymbol:V]
			                          expr:E],
			nil];
}

statement(S) ::= RETURN SEMI.
{
	S = [LKBlockReturn returnWithExpr:[LKDeclRef referenceWithSymbol:@"nil"]];
}
statement(S) ::= RETURN expression(E) SEMI.
{
	S = [LKBlockReturn returnWithExpr:E];
}
statement(S) ::= statement_expression(E) SEMI.
{
	S = E;
}
statement(S) ::= IF LPAREN expression(C) RPAREN body(T).
{
	S = [LKIfStatement ifStatementWithCondition:C then:T else:nil];
}
statement(S) ::= IF LPAREN expression(C) RPAREN body(T) ELSE body(E).
{
	S = [LKIfStatement ifStatementWithCondition:C then:T else:E];
}
statement(S) ::= loop_statement(L).
{
	S = L;
}
statement(S) ::= ident(K) COLON loop_statement(L).
{
	[L setLabel:K];
	S = L;
}
loop_statement(S) ::= FOR LPAREN expression_list(I) SEMI
                                maybe_expression(C) SEMI
                                 expression_list(U) RPAREN body(B).
{
	S = [LKLoop loopWithStatements:B];
	[S setInitStatements:I];
	[S setPreCondition:C];
	[S setUpdateStatements:U];
}
loop_statement(S) ::= FOR LPAREN VAR declarations(I) SEMI
                                 maybe_expression(C) SEMI
                                  expression_list(U) RPAREN body(B).
{
	S = [LKLoop loopWithStatements:B];
	[S setInitStatements:I];
	[S setPreCondition:C];
	[S setUpdateStatements:U];
}
loop_statement(S) ::= FOR LPAREN ident(V) IN expression(E) RPAREN body(B).
{
	// TODO
	S = V = E = B;
}
loop_statement(S) ::= FOR LPAREN VAR ident(V) IN expression(E) RPAREN body(B).
{
	// TODO
	S = V = E = B;
}
loop_statement(S) ::= WHILE LPAREN expression(C) RPAREN body(B).
{
	S = [LKLoop loopWithStatements:B];
	[S setPreCondition:C];
}
loop_statement(S) ::= DO body(B) WHILE LPAREN expression(C) RPAREN SEMI.
{
	S = [LKLoop loopWithStatements:B];
	[S setPostCondition:C];
}
statement(S) ::= BREAK maybe_ident(L) SEMI.
{
	S = [LKBreak breakWithLabel:L];
}
statement(S) ::= CONTINUE maybe_ident(L) SEMI.
{
	S = [LKContinue continueWithLabel:L];
}

maybe_expression(E) ::= expression(C). { E = C; }
maybe_expression ::= .

maybe_ident(I) ::= ident(W). { I = W; }
maybe_ident ::= .

body(L) ::= LBRACE statement_list(S) RBRACE.
{
	L = S;
}
/*
// FIXME: Parsing conflicts
body(L) ::= statement(S).
{
	L = [NSArray arrayWithObject:S];
}
*/
body ::= SEMI.

/* Constructs allowed both as statements and as expressions.
   This includes all assignments and function applications. */
statement_expression(E) ::= ident(T) EQ expression(V).
{
	E = [LKAssignExpr assignWithTarget:[LKDeclRef referenceWithSymbol:T]
	                              expr:V];
}
statement_expression(E) ::= expression(T) DOT ident(K) EQ expression(V). [EQ]
{
	E = [LKMessageSend messageWithSelectorName:@"setValue:forKey:"];
	[E setTarget:T];
	[E addArgument:V];
	[E addArgument:[LKStringLiteral literalFromString:K]];
}
statement_expression(E) ::= expression(T) LBRACK expression(K) RBRACK
                                              EQ expression(V). [EQ]
{
	E = [LKMessageSend messageWithSelectorName:@"setValue:forKey:"];
	[E setTarget:T];
	[E addArgument:V];
	[E addArgument:K];
}
statement_expression(E) ::= ident(T) shortcut_assign(S) expression(R). [PLUSEQ]
{
	E = [LKMessageSend messageWithSelectorName:S];
	[E setTarget:[LKDeclRef referenceWithSymbol:T]];
	[E addArgument:R];
	E = [LKAssignExpr assignWithTarget:[LKDeclRef referenceWithSymbol:T]
	                              expr:E];
}
statement_expression(E) ::= ident(T) increment(S).
{
	// FIXME: Should return old value.
	E = [LKMessageSend messageWithSelectorName:S];
	[E setTarget:[LKDeclRef referenceWithSymbol:T]];
	[E addArgument:[LKNumberLiteral literalFromString:@"1"]];
	E = [LKAssignExpr assignWithTarget:[LKDeclRef referenceWithSymbol:T]
	                              expr:E];
}
statement_expression(E) ::= increment(S) ident(T).
{
	E = [LKMessageSend messageWithSelectorName:S];
	[E setTarget:[LKDeclRef referenceWithSymbol:T]];
	[E addArgument:[LKNumberLiteral literalFromString:@"1"]];
	E = [LKAssignExpr assignWithTarget:[LKDeclRef referenceWithSymbol:T]
	                              expr:E];
}
statement_expression(E) ::= expression(T) DOT ident(K) shortcut_assign(S)
                                                       expression(R). [PLUSEQ]
{
	id old = [LKMessageSend messageWithSelectorName:@"slotValueForKey:"];
	[old setTarget:T];
	[old addArgument:[LKStringLiteral literalFromString:K]];
	
	id new = [LKMessageSend messageWithSelectorName:S];
	[new setTarget:old];
	[new addArgument:R];

	E = [LKMessageSend messageWithSelectorName:@"setValue:forKey:"];
	[E setTarget:T];
	[E addArgument:new];
	[E addArgument:[LKStringLiteral literalFromString:K]];
}
statement_expression(E) ::= expression(T) LBRACK expression(K) RBRACK
                              shortcut_assign(S) expression(R). [PLUSEQ]
{
	id old = [LKMessageSend messageWithSelectorName:@"slotValueForKey:"];
	[old setTarget:T];
	[old addArgument:K];
	
	id new = [LKMessageSend messageWithSelectorName:S];
	[new setTarget:old];
	[new addArgument:R];

	E = [LKMessageSend messageWithSelectorName:@"setValue:forKey:"];
	[E setTarget:T];
	[E addArgument:new];
	[E addArgument:K];
}
statement_expression(E) ::= ident(V) LPAREN RPAREN.
{
	E = [LKMessageSend messageWithSelectorName:@"value"];
	[E setTarget:[LKDeclRef referenceWithSymbol:V]];
}
statement_expression(E) ::= ident(V) LPAREN expressions(A) RPAREN.
{
	E = [LKMessageSend message];
	[E setTarget:[LKDeclRef referenceWithSymbol:V]];
	FOREACH(A, arg, LKAST*)
	{
		[E addSelectorComponent:@"value:"];
		[E addArgument:arg];
	}
}
statement_expression(E) ::= expression(T) DOT keyword_selector(S) LPAREN RPAREN.
{
	E = [LKMessageSend messageWithSelectorName:S];
	[E setTarget:T];
}
statement_expression(E) ::= expression(T) DOT keyword_selector(S)
                                       LPAREN expressions(A) RPAREN.
{
	S = [S stringByAppendingString:@":"];
	E = [LKMessageSend messageWithSelectorName:S];
	[E setTarget:T];
	FOREACH(A, arg, LKAST*)
	{
		[E addArgument:arg];
	}
}

keyword_selector(S) ::= keyword_selector(T) COLON ident(K).
{
	S = [T stringByAppendingFormat:@":%@", K];
}
keyword_selector(S) ::= ident(K).
{
	S = K;
}

expression(E) ::= statement_expression(S).
{
	E = S;
}
expression(E) ::= NEW WORD(V) LPAREN RPAREN.
{
	E = [LKMessageSend messageWithSelectorName:@"construct"];
	[E setTarget: [LKDeclRef referenceWithSymbol:V]];
}
expression(E) ::= NEW WORD(V) LPAREN expressions(A) RPAREN.
{
	E = [LKMessageSend messageWithSelectorName:@"construct:"];
	[E setTarget: [LKDeclRef referenceWithSymbol:V]];
	// FIXME: pop these in an array.
	FOREACH(A, arg, LKAST*)
	{
		[E addArgument:arg];
	}
}
expression(E) ::= FUNCTION LPAREN  argument_list(A) RPAREN
                           LBRACE statement_list(B) RBRACE.
{
	/* LanguageKit uses the result of the last statement as the return value
	   of the function. Simply reference nil to return that by default. */
	[B addObject:[LKDeclRef referenceWithSymbol:@"nil"]];
	LKMessageSend *this = [LKMessageSend messageWithSelectorName: @"slotValueForKey:"];
	[this setTarget: [LKDeclRef referenceWithSymbol: @"blockContext"]];
	[this addArgument: [LKStringLiteral literalFromString: @"this"]];
	NSMutableArray *constructThis = [NSMutableArray array];
	[constructThis addObject: [LKVariableDecl variableDeclWithName: (LKToken*)@"this"]];
	[constructThis addObject: [LKAssignExpr assignWithTarget: [LKDeclRef referenceWithSymbol: @"this"]
	                                                    expr: this]];
	NSArray *storeContext = [NSArray arrayWithObject: [LKAssignExpr assignWithTarget: [LKDeclRef referenceWithSymbol: @"this"]
	                                                                            expr: [LKDeclRef referenceWithSymbol: @"blockContext"]]];
	[constructThis addObject: [LKIfStatement ifStatementWithCondition: [LKCompare comparisonWithLeftExpression: [LKDeclRef referenceWithSymbol: @"this"]
	                                                                                           rightExpression: [LKDeclRef referenceWithSymbol: @"nil"]]
	                                                             then: storeContext
	                                                             else: nil]];
	[constructThis addObjectsFromArray: B];
	E = [LKBlockExpr blockWithArguments:A locals:nil statements: constructThis];
}
expression(E) ::= expression(T) DOT ident(K).
{
	E = [LKMessageSend messageWithSelectorName:@"slotValueForKey:"];
	[E setTarget:T];
	[E addArgument:[LKStringLiteral literalFromString:K]];
}
expression(E) ::= expression(T) LBRACK expression(K) RBRACK.
{
	E = [LKMessageSend messageWithSelectorName:@"slotValueForKey:"];
	[E setTarget:T];
	[E addArgument:K];
}
expression(E) ::= expression(L) binary_selector(S) expression(R). [PLUS]
{
	E = [LKMessageSend messageWithSelectorName:S];
	[E setTarget:L];
	[E addArgument:R];
}
expression(E) ::= TRUE.
{
	E = [LKNumberLiteral literalFromString:@"1"];
}
expression(E) ::= FALSE.
{
	E = [LKNumberLiteral literalFromString:@"0"];
}
expression(E) ::= NULL.
{
	E = [LKDeclRef referenceWithSymbol:@"nil"];
}
expression(E) ::= ident(V).
{
	E = [LKDeclRef referenceWithSymbol:V];
}
expression(E) ::= STRING(S).
{
	E = [LKStringLiteral literalFromString:S];
}
expression(E) ::= NUMBER(N).
{
	E = [LKNumberLiteral literalFromString:N];
}
expression(E) ::= AT ident(S).
{
	E = [LKNumberLiteral literalFromSymbol:S];
}
expression(E) ::= expression(L) EQEQEQ expression(R).
{
	E = [LKCompare comparisonWithLeftExpression:L
	                            rightExpression:R];
}
expression(E) ::= LPAREN expression(X) RPAREN.
{
	[X setBracketed:YES];
	E = X;
}
expression(E) ::= LBRACK expression_list(L) RBRACK.
{
	E = [LKArrayExpr arrayWithElements:L];
}
expression(E) ::= LBRACE RBRACE.
{
	E = [LKMessageSend messageWithSelectorName:@"clone"];
	[E setTarget:[LKDeclRef referenceWithSymbol:@"Object"]];
}
expression(E) ::= LBRACE keys_and_values(L) RBRACE.
{
	// FIXME: Get rid of the self message
	[L addObject:[LKMessageSend messageWithSelectorName:@"self"]];
	id msg = [LKMessageSend messageWithSelectorName:@"clone"];
	[msg setTarget:[LKDeclRef referenceWithSymbol:@"Object"]];
	E = [LKMessageCascade messageCascadeWithTarget:msg messages:L];
}

keys_and_values(L) ::= keys_and_values(T) COMMA key(K) COLON expression(V).
{
	id msg = [LKMessageSend messageWithSelectorName:@"setValue:forKey:"];
	[msg addArgument:V];
	[msg addArgument:K];
	[T addObject:msg];
	L = T;
}
keys_and_values(L) ::= key(K) COLON expression(V).
{
	id msg = [LKMessageSend messageWithSelectorName:@"setValue:forKey:"];
	[msg addArgument:V];
	[msg addArgument:K];
	L = [NSMutableArray arrayWithObject:msg];
}

key(K) ::= ident(W).
{
	K = [LKStringLiteral literalFromString:W];
}
key(K) ::= STRING(S).
{
	K = [LKStringLiteral literalFromString:S];
}

argument_list(L) ::= arguments(T).
{
	L = T;
}
argument_list ::= .

arguments(L) ::= arguments(T) COMMA ident(A).
{
	[T addObject:A];
	L = T;
}
arguments(L) ::= ident(A).
{
	L = [NSMutableArray arrayWithObject:A];
}

expression_list(L) ::= expressions(T).
{
	L = T;
}
expression_list ::= .

expressions(L) ::= expressions(T) COMMA expression(E).
{
	[T addObject:E];
	L = T;
}
expressions(L) ::= expression(E).
{
	L = [NSMutableArray arrayWithObject:E];
}

%syntax_error 
{
	NSLog(@"Syntax error near: '%@'.", TOKEN);
	[NSException raise:@"ParserError" format:@"Parsing failed"];
}

ident(I) ::= WORD(W). { I = W; }
ident(I) ::= NEW(W).  { I = W; }
ident(I) ::= IN(W).   { I = W; }

binary_selector(S) ::= PLUS.  { S = @"plus:"; }
binary_selector(S) ::= MINUS. { S = @"sub:"; }
binary_selector(S) ::= MUL.   { S = @"mul:"; }
binary_selector(S) ::= DIV.   { S = @"div:"; }
binary_selector(S) ::= MOD.   { S = @"mod:"; }
binary_selector(S) ::= EQEQ.  { S = @"isEqual:"; }
binary_selector(S) ::= LT.    { S = @"isLessThan:"; }
binary_selector(S) ::= GT.    { S = @"isGreaterThan:"; }

shortcut_assign(S) ::= PLUSEQ.  { S = @"plus:"; }
shortcut_assign(S) ::= MINUSEQ. { S = @"sub:"; }
shortcut_assign(S) ::= MULEQ.   { S = @"mul:"; }
shortcut_assign(S) ::= DIVEQ.   { S = @"div:"; }
shortcut_assign(S) ::= MODEQ.   { S = @"mod:"; }

increment(S) ::= PLUSPLUS.   { S = @"plus:"; }
increment(S) ::= MINUSMINUS. { S = @"sub:"; }
