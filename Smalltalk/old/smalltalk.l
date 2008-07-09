%{
	#include "parser.h"
	#include "y.tab.h"
	#include <string.h>
	#define YY_NO_UNPUT
	#define RETURN_STRING(x) {/*printf("Found word: %s\n", yytext);*/yylval.stringValue = strdup(yytext); return x;}
%}
%option noyywrap

WORD		[a-zA-Z_]+[a-zA-Z0-9_]*
NUMBER		[0-9]+
WHITE		[ \n \t]+
%%
{WHITE}	/* Skip Whitespace */
{WORD}	RETURN_STRING(WORD)
{NUMBER}RETURN_STRING(NUMBER)
.		{ return (int) yytext[0]; }
%%	
void yyerror(char *s) 
{ 
	fprintf(stderr, "%s\n", s); 
}
