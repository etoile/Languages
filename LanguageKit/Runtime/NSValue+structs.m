#import <Foundation/NSValue.h>

id LKBoxValue(void *bytes, const char *typeEncoding)
{
	if (NULL == bytes)
	{
		return nil;
	}
	return [NSValue valueWithBytes:bytes objCType:typeEncoding];
}
void LKUnboxValue(id boxed, void *buffer, const char *typeEncoding)
{
	if (nil == boxed)
	{
		NSUInteger size, align;
		NSGetSizeAndAlignment(typeEncoding, &size, &align);
		memset(buffer, 0, size);
	}
	assert(strcmp(typeEncoding, [boxed objCType]) == 0);
	return [boxed getValue: buffer];
}
