/*
Parser definition file.  This uses LEMON (from the SQLite project), a public
domain parser generator, to produce an Objective-C parser.
*/
%include {
#include <stdlib.h>
#include <string.h>
#import <EtoileFoundation/EtoileFoundation.h>
#import "AST.h"
#import "ArrayExpr.h"
#import "AssignExpr.h"
#import "BlockExpr.h"
#import "DeclRef.h"
#import "Literal.h"
#import "MessageSend.h"
#import "Method.h"
#import "Module.h"
#import "Parser.h"
#import "Return.h"
#import "Subclass.h"
}
%token_prefix TOKEN_
%token_type {id}
%left WORD.
%extra_argument {Parser* p}

file ::= module(M).
{
	[M check];
	p->delegate = M;
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
module(M) ::= .
{
	M = [CompilationUnit new];
}

subclass(S) ::= WORD(C) SUBCLASS COLON WORD(N) LSQBRACK local_list(L) method_list(M) RSQBRACK.
{
	S = [[Subclass alloc] initWithName:N superclass:C ivars:L methods:M];
}

method_list(L) ::= method_list(A) method(M).
{
	if (L == nil)
	[A addObject:M];
	L = A;
}
method_list(L) ::= .
{
	L = [NSMutableArray array];
}

category(C) ::= WORD(C) EXTEND LSQBRACK method(M) RSQBRACK.

method(M) ::= messageSignature(S) LSQBRACK local_list(L) statement_list(E) RSQBRACK.
{
	MethodSymbolTable *ST = [[MethodSymbolTable alloc] initWithLocals:L args:[(MessageSend*)S arguments]];
	SmalltalkMethod *Meth = [[[SmalltalkMethod alloc] initWithSymbolTable:ST] autorelease];
	[ST release];
	[Meth setSignature:S];
	Meth->statements = E;
	M = Meth;
}

messageSignature(S) ::= WORD(M).
{
	S = [[MessageSend alloc] init];
	[S addSelectorComponent:M];
}
messageSignature(M) ::= method_signature_with_arguments(A).
{
	M = A;
}
method_signature_with_arguments(M) ::= method_signature_with_arguments(N) WORD(S) COLON WORD(E).
{
	[N addArgument:E];
	[N addSelectorComponent:S];
	[N addSelectorComponent:@":"];
	M = N;
}
method_signature_with_arguments(M) ::= WORD(S) COLON WORD(E).
{ 
	M = [[MessageSend alloc] init];
	[M addArgument:E];
	[M addSelectorComponent:S];
	[M addSelectorComponent:@":"];
}

local_list(LL) ::= BAR locals(L) BAR. 
{
	LL = L;
}
local_list ::= .

locals(L) ::= locals(LL) WORD(W).
{
  [LL addObject:W];
  L = LL;
}
locals(L) ::= .
{
  L = [NSMutableArray array];
}

%syntax_error {NSLog(@"Syntax error.");}

%type statement {AST*}
statement_list(LL) ::= statement_list(L) statement(S).
{
  [L addObject:S];
  LL = L;
}
statement_list(L) ::= .
{
  L = [NSMutableArray array];
}

statement(S) ::= assignment(A) STOP.
{
	S = A;
}
statement(S) ::= expression(E) STOP.
{
	S = E;
}
statement(S) ::= return(R) STOP.
{
	S = R;
}

return(R) ::= RETURN expression(E).
{
	R = [[Return alloc] initWithExpr:E];
}

message_send(S) ::= expression(T) message(M).
{
	[M setTarget:T];
	S = M;
}

message(M) ::= message_with_arguments(A).
{
	M = A;
}
message(M) ::= WORD(S).
{
	M = [[MessageSend alloc] init];
	[M addSelectorComponent:S];
}

//message_with_arguments(M) ::= WORD(S) COLON expression(E) message_with_arguments(N).
message_with_arguments(M) ::= message_with_arguments(N) WORD(S) COLON expression(E).
{
	[N addArgument:E];
	[N addSelectorComponent:S];
	[N addSelectorComponent:@":"];
	M = N;
}
message_with_arguments(M) ::= WORD(S) COLON expression(E).
{ 
	M = [[MessageSend alloc] init];
	[M addArgument:E];
	[M addSelectorComponent:S];
	[M addSelectorComponent:@":"];
}

assignment(A) ::= WORD(T) COLON EQ expression(E).
{
	DeclRef * declref = [[DeclRef alloc] init];
	declref->symbol = T;
	AssignExpr *AE = [AssignExpr new];
	AE->target = declref;
	AE->expr = E;
	A = AE;
}

expression(E) ::= LBRACK expression(X) RBRACK.
{
	[X setBracketed:YES];
	E = X;
}

expression(E) ::= WORD(V).
{
	DeclRef * declref = [[DeclRef alloc] init];
	declref->symbol = V;
	E = declref;
}

expression(E) ::= block(B).
{
	E = B;
}
expression(E) ::= message_send(M).
{
	E = M;
}
expression(E) ::= STRING(N).
{
	E = [StringLiteral literalFromString:N];
}
expression(E) ::= NUMBER(N).
{
	E = [NumberLiteral literalFromString:N];
}
expression(E) ::= HASH LBRACK expression_list(L).
{
	E = [ArrayExpr arrayWithElements:L];
}
expression_list(L) ::= expression(E) COMMA expression_list(T).
{
	[T addObject:E];
	L = T;
}
expression_list(L) ::= expression(E) RBRACK.
{
	L = [NSMutableArray arrayWithObject:E];
}

block(B) ::= LSQBRACK argument_list(A)  statement_list(S) RSQBRACK.
{
//FIXME: block locals
	BlockSymbolTable *ST = [[BlockSymbolTable alloc] initWithLocals:nil args:A];
	B = [[[BlockExpr alloc] initWithSymbolTable:ST] autorelease];
	[ST release];
	[B setStatements:S];
}
block(B) ::= LSQBRACK statement_list(S) RSQBRACK.
{
	BlockSymbolTable *ST = [[BlockSymbolTable alloc] init];
	B = [[[BlockExpr alloc] initWithSymbolTable:ST] autorelease];
	[B setStatements:S];
}

argument_list(A) ::= COLON WORD(H) argument_list(T).
{
	[T addObject:H];
	A = T;
}
argument_list(A) ::= BAR.
{
	A = [NSMutableArray array];
}
