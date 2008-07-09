%{
	#include <stdio.h>
	#include "parser.h"
	#define YY_MAIN 0
	void collect_local(char *);
%}
%union
{
	char* stringValue;
	Message * message;
	Statement * statement;
	Argument * argument;
	Expression * expression;
}
%token <stringValue> WORD
%type <message> message_send
%type <argument> message message_with_arguments;
%type <statement> statement
%type <expression> expression;

//%type <pointerValue> message_send message message_with_arguments
%%
method:
	  local_list statement_list;

local_list:
		  '|' locals '|'
		  |
		  ;

locals:
	  WORD locals
				{
					collect_local($1);
				} 
	  |
	  ;

statement_list:
			  statement_list statement '.' 
			  	{
					addStatement($2);
				}
			  |
			  ;

statement:
		 message_send
		 	{
				$$ = malloc(sizeof(Statement));
				$$->type = message_pass;
				$$->value.message = $1;
			}
		 |
		 assignment
		 	{
			//Assignments not supported yet
		 		$$ = NULL;
			};

message_send:
			WORD message
					{
						$$ = malloc(sizeof(Message));
						$$->target = $1;
						$$->arguments = $2;
					};

message:
	   WORD
			{
				$$ = malloc(sizeof(Argument));
				$$->selector_component = $1;
				$$->value = NULL;
				$$->next = NULL;
			}
	   |
	   message_with_arguments
			{
				$$ = $1;
			}
	   ;

message_with_arguments:
					  WORD ':' expression message_with_arguments 
							{
								$$ = malloc(sizeof(Argument));
								$$->selector_component = $1;
								$$->value = $3;
								$$->next = $4;
							}
					  |
					  { $$ = NULL; }
					  ;

assignment:
		  WORD ':' '=' expression ;

expression:
		  WORD
		  	{
				$$ = malloc(sizeof(Expression));
				$$->type = variable;
				$$->value.variable = $1;
			}
		  ;
%%
int parseSmalltalk(char * program)
{
	//yy_scan_string("| a b c | \n foo bar.");
	yy_scan_string(program);
	return yyparse();
}
