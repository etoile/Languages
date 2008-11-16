/*
Parser definition file.  This uses LEMON (from the SQLite project), a public
domain parser generator, to produce an Objective-C parser.
*/
%include {
#import <EtoileFoundation/EtoileFoundation.h>
#import <LanguageKit/LanguageKit.h>
#import "SmalltalkParser.h"
}
%token_prefix TOKEN_
%token_type {id}
%extra_argument {SmalltalkParser *p}
%left BINARY EQ.
%left WORD.

file ::= module(M).
{
	[M check];
	[p setDelegate:M];
}

module(M) ::= module(O) subclass(S).
{
	[O addClass:S];
	M = O;
}
module(M) ::= module(O) category(C).
{
	[O addCategory:C];
	M = O;
}
module(M) ::= module(O) comment.
{
	M = O;
}
module(M) ::= .
{
	M = [[[LKCompilationUnit alloc] init] autorelease];
}

subclass(S) ::= WORD(C) SUBCLASS COLON WORD(N) LSQBRACK local_list(L) method_list(M) RSQBRACK.
{
	S = [LKSubclass subclassWithName:N superclass:C ivars:L methods:M];
}

category(D) ::= WORD(C) EXTEND LSQBRACK method_list(M) RSQBRACK.
{
	D = [LKCategoryDef categoryWithClass:C methods:M];
}

local_list(L) ::= BAR locals(T) BAR. 
{
	L = T;
}
local_list ::= .

locals(L) ::= locals(T) WORD(W).
{
	[T addObject:W];
	L = T;
}
locals(L) ::= .
{
	L = [NSMutableArray array];
}

method_list(L) ::= method_list(T) method(M).
{
	[T addObject:M];
	L = T;
}
method_list(L) ::= method_list(T) comment.
{
	L = T;
}
method_list(L) ::= .
{
	L = [NSMutableArray array];
}

method(M) ::= signature(S) LSQBRACK local_list(L) statement_list(E) RSQBRACK.
{
	M = [LKInstanceMethod methodWithSignature:S locals:L statements:E];
}

signature(S) ::= WORD(M).
{
	S = [[[LKMessageSend alloc] init] autorelease];
	[S addSelectorComponent:M];
}
signature(S) ::= keyword_signature(M).
{
	S = M;
}
keyword_signature(S) ::= keyword_signature(M) KEYWORD(K) WORD(E).
{
	S = M;
	[S addSelectorComponent:K];
	[S addArgument:E];
}
keyword_signature(S) ::= KEYWORD(K) WORD(E).
{ 
	S = [[[LKMessageSend alloc] init] autorelease];
	[S addSelectorComponent:K];
	[S addArgument:E];
}

statement_list(L) ::= statement(S) STOP statement_list(T).
{
	[T insertObject:S atIndex:0];
	L = T;
}
statement_list(L) ::= comment(C) statement_list(T).
{
	[T insertObject:C atIndex:0];
	L = T;
}
statement_list(L) ::= statement(S).
{
	L = [NSMutableArray arrayWithObject:S];
}
statement_list(L) ::= .
{
	L = [NSMutableArray array];
}

comment(S) ::= COMMENT(C).
{
	S = [LKComment commentForString:C];
}

statement(S) ::= expression(E).
{
	S = E;
}
statement(S) ::= RETURN expression(E).
{
	S = [LKReturn returnWithExpr:E];
}
statement(S) ::= WORD(T) COLON EQ expression(E).
{
	S = [LKAssignExpr assignWithTarget:[LKDeclRef reference:T] expr:E];
}

%syntax_error 
{
	[NSException raise:@"ParserError" format:@"Parsing failed"];
}

expression(E) ::= keyword_expression(K).
{
	E = K;
}
expression(E) ::= simple_expression(S).
{
	E = S;
}

keyword_expression(E) ::= keyword_expression(M) KEYWORD(K) simple_expression(A).
{
	[M addSelectorComponent:K];
	[M addArgument:A];
	E = M;
}
keyword_expression(M) ::= simple_expression(T) KEYWORD(K) simple_expression(A).
{
	M = [[[LKMessageSend alloc] init] autorelease];
	[M setTarget:T];
	[M addSelectorComponent:K];
	[M addArgument:A];
}

simple_expression(E) ::= WORD(V).
{
	E = [LKDeclRef reference:V];
}
simple_expression(E) ::= SYMBOL(S).
{
	E = [LKSymbolRef reference:S];
}
simple_expression(E) ::= STRING(S).
{
	E = [LKStringLiteral literalFromString:S];
}
simple_expression(E) ::= NUMBER(N).
{
	E = [LKNumberLiteral literalFromString:N];
}
simple_expression(E) ::= AT WORD(S).
{
	E = [LKNumberLiteral literalFromSymbol:S];
}
simple_expression(E) ::= simple_expression(L) WORD(S).
{
	E = [[[LKMessageSend alloc] init] autorelease];
	[E setTarget:L];
	[E addSelectorComponent:S];
}
simple_expression(E) ::= simple_expression(L) BINARY(S) simple_expression(R).
{
	E = [[[LKMessageSend alloc] init] autorelease];
	[E setTarget:L];
	[E addSelectorComponent:S];
	[E addArgument:R];
}
simple_expression(E) ::= simple_expression(L) EQ simple_expression(R).
{
	E = [[[LKMessageSend alloc] init] autorelease];
	[E setTarget:L];
	[E addSelectorComponent:@"isEqual:"];
	[E addArgument:R];
}
simple_expression(E) ::= simple_expression(L) EQ EQ simple_expression(R).
{
	E = [LKCompare compare:L to:R];
}
simple_expression(E) ::= LPAREN expression(X) RPAREN.
{
	[X setBracketed:YES];
	E = X;
}
simple_expression(E) ::= LBRACE expression_list(L) RBRACE.
{
	E = [LKArrayExpr arrayWithElements:L];
}
simple_expression(E) ::= LSQBRACK argument_list(A) statement_list(S) RSQBRACK.
{
	//FIXME: block locals
	E = [LKBlockExpr blockWithArguments:A locals:nil statements:S];
}

argument_list(L) ::= COLON WORD(A) argument_list(T).
{
	[T insertObject:A atIndex:0];
	L = T;
}
argument_list(L) ::= BAR.
{
	L = [NSMutableArray array];
}
argument_list ::= .

expression_list(L) ::= expression(E) STOP expression_list(T).
{
	[T insertObject:E atIndex:0];
	L = T;
}
expression_list(L) ::= expression(E).
{
	L = [NSMutableArray arrayWithObject:E];
}
expression_list(L) ::= .
{
	L = [NSMutableArray array];
}
