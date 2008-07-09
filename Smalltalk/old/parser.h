#ifndef __PARSER_H_INCLUDED__
#define __PARSER_H_INCLUDED__
typedef struct _Expression Expression;

typedef struct _Argument
{
	char * selector_component;
	Expression * value;
	struct _Argument * next;
} Argument;

typedef struct
{
	char * target;
	Argument * arguments;
	int temps;
} Message;

struct _Expression
{
	enum {message, variable} type;
	union
	{
		Message * message;
		char * variable;
	} value;
};

typedef struct
{
	char * target;
	Expression * value;
} Assignment;

typedef struct _Statement
{
	enum {assignment, message_pass} type;
	union 
	{
		Assignment * assignment;
		Message * message;
	} value;
	struct _Statement * next;
} Statement;

#endif
