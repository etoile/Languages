#import "LKTypeHelpers.h"

void LKSkipQualifiers(const char **typestr)
{
	char c = **typestr;
	while ((c >= '0' && c <= '9') || c == 'r' || c == 'n' || c == 'N' ||
		   c == 'o' || c == 'O' || c == 'R' || c == 'V')
	{
		(*typestr)++;
		c = **typestr;
	}
}

#define SKIPNAME \
	while (**typestr != '=')\
	{\
		(*typestr)++;\
	}\
	(*typestr)++

void LKNextType(const char **typestr)
{
	LKSkipQualifiers(typestr);
	switch (**typestr)
	{
		case '{':
			(*typestr)++;
			SKIPNAME;
			while (**typestr != '}')
			{
				LKNextType(typestr);
			}
			(*typestr)++;
			break;
		case '(':
			(*typestr)++;
			SKIPNAME;
			while (**typestr != ')')
			{
				LKNextType(typestr);
			}
			(*typestr)++;
			break;
		case '[':
			(*typestr)++;
			while (**typestr != '[')
			{
				LKNextType(typestr);
			}
			(*typestr)++;
			break;
		case '^':
			(*typestr)++;
			LKNextType(typestr);
			break;
		default:
			(*typestr)++;
	}
}

int LKCountObjCTypes(const char *objctype)
{
	if (NULL == objctype)
	{
		return 0;
	}
	int count = 0;
	while (*objctype != '\0')
	{
		LKNextType(&objctype);
		LKSkipQualifiers(&objctype);
		count++;
	}
	return count;
}

