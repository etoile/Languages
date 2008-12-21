/*
Parser definition file.  This uses LEMON (from the SQLite project), a public
domain parser generator, to produce an Objective-C parser.
*/
%include {
#import <EtoileFoundation/EtoileFoundation.h>
#import <LanguageKit/LanguageKit.h>
#import "EScriptParser.h"
}
%token_prefix TOKEN_
%token_type {id}
%extra_argument {EScriptParser *p}
%left PLUS MINUS STAR SLASH EQ LT GT.
%left WORD.

file ::= module(M).
{
	[M check];
	[p setDelegate:M];
}

module(M) ::= module(O) LT LT pragma_dict(P) GT GT.
{
	[O addPragmas: P];
	M = O;
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

pragma_dict(P) ::= pragma_dict(D) COMMA WORD(K) EQ pragma_value(V).
{
	[D setObject:V forKey:K];
	P = D;
}
pragma_dict(P) ::= WORD(K) EQ pragma_value(V).
{
	P = [NSMutableDictionary dictionaryWithObject:V forKey:K];
}

pragma_value(V) ::= WORD(W).
{
	V = W;
}
pragma_value(V) ::= STRING(S).
{
	V = S;
}
pragma_value(V) ::= NUMBER(N).
{
	V = N;
}

if_statement(I) ::= IF LPAREN expression(E) RPAREN statements(S).
{
	I = S;
	I = E;
}

while_loop(W) ::= WHILE LPAREN expression(E) RPAREN statements(S).
{
	W = S;
	W = E;
}


variable_declaration ::= VAR WORD(W).
{
	W;
}

new_expression ::= NEW.
{}

function_expression ::= FUNCTION.

subclass(S) ::= WORD(C) SUBCLASS COLON WORD(N) LSQBRACK ivar_list(L) method_list(M) RSQBRACK.
{
	S = [LKSubclass subclassWithName:N
	                      superclass:C
	                           cvars:[L objectAtIndex:1]
	                           ivars:[L objectAtIndex:0]
	                         methods:M];
}

category(D) ::= WORD(C) EXTEND LSQBRACK method_list(M) RSQBRACK.
{
	D = [LKCategoryDef categoryWithClass:C methods:M];
}

ivar_list(L) ::= BAR ivars(T) BAR.
{
	L = T;
}
ivar_list ::= .

ivars(L) ::= ivars(T) WORD(W).
{
	/* First element is the list of instance variables. */
	[[T objectAtIndex:0] addObject:W];
	L = T;
}
ivars(L) ::= ivars(T) PLUS WORD(W).
{
	/* Second element is the list of class variables. */
	[[T objectAtIndex:1] addObject:W];
	L = T;
}
ivars(L) ::= .
{
	/*
	Separate lists need to be built for instance and class variables.
	Put them in a fixed size array since we can only pass one value.
	*/
	L = [NSArray arrayWithObjects: [NSMutableArray array],
	                               [NSMutableArray array],
	                               nil];
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
method(M) ::= PLUS signature(S) LSQBRACK local_list(L) statement_list(E) RSQBRACK.
{
	M = [LKClassMethod methodWithSignature:S locals:L statements:E];
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

statement_list(L) ::= statements(T).
{
	L = T;
}
statement_list(L) ::= statements(T) statement(S).
{
	[T addObject:S];
	L = T;
}

statements(L) ::= statements(T) statement(S) STOP.
{
	[T addObject:S];
	L = T;
}
statements(L) ::= statements(T) comment(C).
{
	[T addObject:C];
	L = T;
}
statements(L) ::= .
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

message(M) ::= keyword_message(K).
{
	M = K;
}
message(M) ::= simple_message(S).
{
	M = S;
}

keyword_message(M) ::= keyword_message(G) KEYWORD(K) simple_expression(A).
{
	M = G;
	[M addSelectorComponent:K];
	[M addArgument:A];
}
keyword_message(M) ::= KEYWORD(K) simple_expression(A).
{
	M = [[[LKMessageSend alloc] init] autorelease];
	[M addSelectorComponent:K];
	[M addArgument:A];
}

simple_message(M) ::= WORD(S).
{
	M = [[[LKMessageSend alloc] init] autorelease];
	[M addSelectorComponent:S];
}
simple_message(M) ::= binary_selector(S) simple_expression(R). [PLUS]
{
	M = [[[LKMessageSend alloc] init] autorelease];
	[M addSelectorComponent:S];
	[M addArgument:R];
}

binary_selector(S) ::= PLUS.
{
	S = @"plus:";
}
binary_selector(S) ::= MINUS.
{
	S = @"sub:";
}
binary_selector(S) ::= STAR.
{
	S = @"mul:";
}
binary_selector(S) ::= SLASH.
{
	S = @"div:";
}
binary_selector(S) ::= EQ.
{
	S = @"isEqual:";
}
binary_selector(S) ::= LT.
{
	S = @"isLessThan:";
}
binary_selector(S) ::= GT.
{
	S = @"isGreaterThan:";
}
binary_selector(S) ::= LT EQ.
{
	S = @"isLessThanOrEqualTo:";
}
binary_selector(S) ::= GT EQ.
{
	S = @"isGreaterThanOrEqualTo:";
}

expression(E) ::= cascade_expression(C).
{
	E = C;
}
expression(E) ::= keyword_expression(K).
{
	E = K;
}
expression(E) ::= simple_expression(S).
{
	E = S;
}

cascade_expression(E) ::= cascade_expression(C) SEMICOLON message(M).
{
	[C addMessage:M];
	E = C;
}
cascade_expression(E) ::= simple_expression(T) message(M) SEMICOLON message(G).
{
	E = [LKMessageCascade messageCascadeWithTarget:T messages:
		[NSMutableArray arrayWithObjects:M, G, nil]];
}

keyword_expression(E) ::= simple_expression(T) keyword_message(M).
{
	[M setTarget:T];
	E = M;
}

simple_expression(E) ::= WORD(V).
{
	if ([V isEqualToString:@"true"])
	{
		E = [LKNumberLiteral literalFromString:@"1"];
	}
	else if ([V isEqualToString:@"false"])
	{
		E = [LKNumberLiteral literalFromString:@"0"];
	}
	else
	{
		E = [LKDeclRef reference:V];
	}
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
simple_expression(E) ::= simple_expression(T) simple_message(M).
{
	[M setTarget:T];
	E = M;
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

argument_list(L) ::= arguments(T) BAR.
{
	L = T;
}
argument_list ::= .

arguments(L) ::= arguments(T) COLON WORD(A).
{
	[T addObject:A];
	L = T;
}
arguments(L) ::= .
{
	L = [NSMutableArray array];
}

expression_list(L) ::= expressions(T).
{
	L = T;
}
expression_list(L) ::= expressions(T) expression(E).
{
	[T addObject:E];
	L = T;
}

expressions(L) ::= expressions(T) expression(E) STOP.
{
	[T addObject:E];
	L = T;
}
expressions(L) ::= .
{
	L = [NSMutableArray array];
}
