/*   Copyright (c) 2003, Steve Dekorte
*   All rights reserved. See _BSDLicense.txt.
*/

#include "IoObjcBridge.h"
#include "List.h"
#include "IoState.h"
#include "IoList.h"
#include "IoVector.h"
#include "ObjcSubclass.h"
#include "IoVector.h"
#include "IoBox.h"
#include "IoNumber.h"

#ifdef GNUSTEP
#include <GNUstepBase/GSObjCRuntime.h>
#include <objc/objc.h>
#else
#import <objc/objc-runtime.h>
#endif
#include "Io2Objc.h"
#include "Objc2Io.h"

#define DATA(self) ((IoObjcBridgeData *)IoObject_dataPointer(self))

static IoObjcBridge *sharedBridge = 0x0;

IoObjcBridge *IoObjcBridge_sharedBridge(void) 
{ 
	return sharedBridge; 
}

/*unsigned char IoObjcBridge_respondsTo(IoObject *self, IoObject *slotName) { return 1; }*/

List *IoObjcBridge_allClasses(IoObjcBridge *self)
{
	int n;
	
	if (DATA(self)->allClasses) 
	{ 
		return DATA(self)->allClasses; 
	}
	else
	{
		int numClasses = 0, newNumClasses = 0;
		Class *classes = NULL;
#ifdef GNUSTEP
		// max here is not important because buffer is nil
		newNumClasses = GSClassList(NULL, 0, NO);
#else
		newNumClasses = objc_getClassList(NULL, 0);
#endif
		
		DATA(self)->allClasses = List_new();
		
		while (numClasses < newNumClasses) 
		{
			numClasses = newNumClasses;
			// GNUStep need to hold max+1 classes.
			classes = realloc(classes, sizeof(Class) * numClasses+1);
#ifdef GNUSTEP
			newNumClasses += GSClassList(classes, numClasses, NO);
#else
			newNumClasses = objc_getClassList(classes, numClasses);
#endif
		}
		
		for (n = 0; n < numClasses; n ++)
		{ 
			List_append_(DATA(self)->allClasses, classes[n]); 
		}
		free(classes); // memory leak - test
		return DATA(self)->allClasses;
	}
}

IoTag *IoObjcBridge_tag(void *state)
{
	IoTag *tag = IoTag_newWithName_("ObjcBridge");
	tag->state = state;
	tag->cloneFunc = (TagCloneFunc *)IoObjcBridge_rawClone;
	tag->freeFunc  = (TagFreeFunc *)IoObjcBridge_free;
	tag->markFunc  = (TagMarkFunc *)IoObjcBridge_mark;
	/*tag->respondsToFunc = (TagRespondsToFunc *)IoObjcBridge_respondsTo;*/
	return tag;
}

IoObjcBridge *IoObjcBridge_proto(void *state)
{
	IoObject *self = IoObject_new(state);
	self->tag = IoObjcBridge_tag(state);
	
	IoObject_setDataPointer_(self, calloc(1, sizeof(IoObjcBridgeData)));
	DATA(self)->io2objcs = Hash_new();
	DATA(self)->objc2ios = Hash_new();
	IoObjcBridge_setMethodBuffer_(self, "nop");
	
	sharedBridge = self;
	
	IoState_registerProtoWithFunc_(state, self, IoObjcBridge_proto);
	
	{
		IoMethodTable methodTable[] = {
		{"classNamed", IoObjcBridge_classNamed},
		{"debugOn", IoObjcBridge_debugOn},
		{"debugOff", IoObjcBridge_debugOff},
		{"newClassWithNameAndProto", IoObjcBridge_newClassNamed_withProto_},
		{"autoLookupClassNamesOn",IoObjcBridge_autoLookupClassNamesOn},
		{"autoLookupClassNamesOff",IoObjcBridge_autoLookupClassNamesOff},
			// Extras
			//{"NSSelectorFromString", IoObjcBridge_NSSelectorFromString},
			//{"NSStringFromSelector", IoObjcBridge_NSStringFromSelector},
		{NULL, NULL},
		};
		IoObject_addMethodTable_(self, methodTable);
	}
	return self;
}

IoObjcBridge *IoObjcBridge_rawClone(IoObjcBridge *self) 
{
	return self; 
}

IoObjcBridge *IoObjcBridge_new(void *state)
{ 
	return IoState_protoWithInitFunction_(state, IoObjcBridge_proto); 
}

void IoObjcBridge_free(IoObjcBridge *self)
{
	sharedBridge = 0x0;
	{
		void *k = Hash_firstKey(DATA(self)->objc2ios);
		
		while (k)
		{
			id v = Hash_at_(DATA(self)->objc2ios, k);
			[v autorelease];
			k = Hash_nextKey(DATA(self)->objc2ios);
		}
	}
	
	Hash_do_(DATA(self)->io2objcs, (HashDoCallback *)Io2Objc_nullObjcBridge);
	
	Hash_free(DATA(self)->io2objcs);
	Hash_free(DATA(self)->objc2ios);
	free(DATA(self)->methodNameBuffer);
	free(IoObject_dataPointer(self));
}

void IoObjcBridge_mark(IoObjcBridge *self)
{
	IoObject *k;
	
	/* --- mark Io2Objc objects --- */
	/*
	 k = Hash_firstKey(DATA(self)->io2objcs);
	 while (k)
	 {
		 IoObject *v = Hash_at_(DATA(self)->io2objcs, k);
		 IoObject_shouldMark(v);
		 k = Hash_nextKey(DATA(self)->io2objcs);
	 }
	 */
	
	/* --- mark io values referenced by Objc2Io objects --- */
	k = Hash_firstKey(DATA(self)->objc2ios);
	
	while (k)
	{
		id v = Hash_at_(DATA(self)->objc2ios, k);
		[v mark];
		k = Hash_nextKey(DATA(self)->objc2ios);
	}
	
	[ObjcSubclass mark]; // mark io protos for ObjcSubclasses
}

/* ----------------------------------------------------------------- */

int IoObjcBridge_rawDebugOn(IoObjcBridge *self)
{ 
	return DATA(self)->debug; 
}

IoObject *IoObjcBridge_autoLookupClassNamesOn(IoObjcBridge *self, IoObject *locals, IoMessage *m)
{
	IoState_doCString_(IOSTATE, "Lobby forward := method(m := thisMessage name; v := ObjcBridge classNamed(m); if (v, return v, Exception raise(\"Lookup error\",  \"slot '\" .. m ..\"' not found\")))");
	return self; 
}

IoObject *IoObjcBridge_autoLookupClassNamesOff(IoObjcBridge *self, IoObject *locals, IoMessage *m)
{
	IoState_doCString_(IOSTATE, "Lobby removeSlot(\"forward\")");
	return self; 
}

IoObject *IoObjcBridge_debugOn(IoObjcBridge *self, IoObject *locals, IoMessage *m)
{ 
	DATA(self)->debug = 1; 
	return self; 
}

IoObject *IoObjcBridge_debugOff(IoObjcBridge *self, IoObject *locals, IoMessage *m)
{ 
	DATA(self)->debug = 0; 
	return self; 
}

/*
 IoObject *IoObjcBridge_NSSelectorFromString(IoObjcBridge *self, IoObject *locals, IoMessage *m)
 { 
	 IoSymbol *name = IoMessage_locals_symbolArgAt_(m, locals, 0);
	 NSString *s = [NSString stringWithCString:CSTRING(name)];
	 SEL sel = NSSelectorFromString(s);
	 return IONUMBER((int)sel); 
 }
 
 IoObject *IoObjcBridge_NSStringFromSelector(IoObjcBridge *self, IoObject *locals, IoMessage *m)
 { 
	 int s = IoMessage_locals_intArgAt_(m, locals, 0);
	 NSString *string = NSStringFromSelector((SEL)s);
	 if (!string) return IONIL(self);
	 return IOSYMBOL([string cString]); 
 }
 */

IoObject *IoObjcBridge_classNamed(IoObjcBridge *self, IoObject *locals, IoMessage *m)
{
	IoSymbol *name = IoMessage_locals_symbolArgAt_(m, locals, 0);
	//id obj = objc_lookUpClass(CSTRING(name));
        id obj = NSClassFromString([NSString stringWithCString: CSTRING(name) encoding: NSASCIIStringEncoding]);
	
	if (!obj) 
	{
		return IONIL(self);
	}
	
	return IoObjcBridge_proxyForId_(self, obj);
}

void *IoObjcBridge_proxyForId_(IoObjcBridge *self, id obj)
{
	Io2Objc *v = Hash_at_(DATA(self)->io2objcs, obj);
	
	if (!v)
	{
		v = Io2Objc_new(IOSTATE);
		Io2Objc_setBridge(v, self);
		Io2Objc_setObject(v, obj);
		Hash_at_put_(DATA(self)->io2objcs, obj, v);
	}
	return v;
}

void *IoObjcBridge_proxyForIoObject_(IoObjcBridge *self, IoObject *v)
{
	Objc2Io *obj = Hash_at_(DATA(self)->objc2ios, v);
	if (!obj)
	{
		obj = [[[Objc2Io alloc] init] autorelease]; /* when released by objc, we'll remove it */
		[obj setBridge:self];
		[obj setIoObject:v];
		//Hash_at_put_(DATA(self)->objc2ios, IOREF(v), obj);
		IoObjcBridge_addValue_(self, v, obj);
	}
	return obj;
}

void IoObjcBridge_removeId_(IoObjcBridge *self, id obj)
{
	/* Called by Io2Objc instance when freed */
	Hash_removeKey_(DATA(self)->io2objcs, obj);
}

void IoObjcBridge_removeValue_(IoObjcBridge *self, IoObject *v)
{
	/* Called by Obj2Io instance when dealloced */
	Hash_removeKey_(DATA(self)->objc2ios, v);
}

void IoObjcBridge_addValue_(IoObjcBridge *self, IoObject *v, id obj)
{
	/* Called by Obj2Io instance when alloced */
	Hash_at_put_(DATA(self)->objc2ios, IOREF(v), obj);
}


// ----------------------------------------------------------------- 
//  Objective-C  -> Io
// ----------------------------------------------------------------- 

IoObject *IoObjcBridge_ioValueForCValue_ofType_(IoObjcBridge *self, void *cValue, char *cType)
{
	IoState *state = IOSTATE;
	switch (*cType)
	{
		case '@':
		{
			//id obj = *((id *)cValue);
			id obj = (id)cValue;
			//if ((*cValue) == 0x2) obj = *((id *)cValue);
			if (!obj) { return IONIL(self); }
			/*
			 else if (!(*cValue)) // || (*cValue) == 0x2)
			 {
				 if (*cValue) printf("Error: *cValue= %i\n", *cValue);
				 return IONIL(self);
			 }
			 */
			/*
			 if (obj==1) 
			 {
				 printf("ObjcBridge error: type is object but value is 1\n"); 
				 return IoNumber_newWithDouble_(state, 1);
			 }
			 */
			
			else if ([obj isKindOfClass:[NSString class]])
			{ 
				return IOSYMBOL((char *)[obj cString]); 
			}
			
			else if ([obj isKindOfClass:[NSNumber class]])
			{ 
				return IONUMBER([obj doubleValue]); 
			}
			
			else if ([obj isKindOfClass:[Objc2Io class]])
			{ 
				return [obj ioValue]; 
			}
			
			return IoObjcBridge_proxyForId_(self, obj);
    }
		case '#':
		{
			id obj = *((id *)cValue);
			
			if (!obj) 
			{ 
				return IONIL(self); 
			}
			
			else if ([obj isKindOfClass:[NSString class]])
			{ 
				return IOSYMBOL((char *)[obj cString]); 
			}
			
			else if ([obj isKindOfClass:[NSNumber class]])
			{ 
				return IONUMBER([obj doubleValue]); 
			}
			
			else if ([obj isKindOfClass:[Objc2Io class]])
			{ 
				return [obj ioValue]; 
			}
			
			return IoObjcBridge_proxyForId_(self, obj);
		}
		case ':': 
		{
			const char *s = sel_getName(*(SEL *)cValue); // ???
			return IOSYMBOL(s);
		}
		case 'c': 
		{
			char c;
			/*memcpy(&c, &cValue, 1);*/
			c = *((char *)cValue);
			return IoNumber_newWithDouble_(state, c);
		}
		case 'C': 
		{
			unsigned char c;
			/*memcpy(&c, &cValue, 1);*/
			c = *((unsigned char *)cValue);
			return IoNumber_newWithDouble_(state, c);
		}
		case 's': return IoNumber_newWithDouble_(state, *(short *)cValue);
		case 'S': return IoNumber_newWithDouble_(state, *(unsigned short *)cValue);
		case 'i': return IoNumber_newWithDouble_(state, (*(int *)(cValue))); // ?
		case 'I': return IoNumber_newWithDouble_(state, *(unsigned int *)(cValue)); // ?
		case 'l': return IoNumber_newWithDouble_(state, *(long *)cValue);
		case 'L': return IoNumber_newWithDouble_(state, *(unsigned long *)cValue);
		case 'f': 
		{ 
			float f;
			memcpy(&f, &cValue, sizeof(float));
			return IoNumber_newWithDouble_(state, (double)f);
		}
		case 'd': return IoNumber_newWithDouble_(state, *(double *)cValue);
		case 'b': return IoNumber_newWithDouble_(state, *(int *)cValue);  // ? Not correct
		case 'v': return IoNumber_newWithDouble_(state, (long)cValue);  // ????
		case 'r': return IoState_symbolWithCString_(state, (char *)cValue);
		case '*': return IoState_symbolWithCString_(state, *(char **)cValue);  // assume a string??
  }
	
	
	if (!strcmp(cType, "{_NSPoint=ff}"))
	{
		NSPoint p = *(NSPoint *)cValue;
		return IoVector_newX_y_z_(state, p.x, p.y, 0);
	}
	else if (!strcmp(cType, "{_NSSize=ff}"))
	{
		NSSize s = *(NSSize *)cValue;
		return IoVector_newX_y_z_(state, s.width, s.height, 0);
	}
	else if (!strcmp(cType, "{_NSRect={_NSPoint=ff}{_NSSize=ff}}"))
	{
		NSRect r = *(NSRect *)cValue;
		return IoBox_newSet(state, 
						r.origin.x, r.origin.y, 0, 
						r.size.width, r.size.height, 0);
	}
	
	return IONIL(self);
}

// ----------------------------------------------------------------- 
//  Io -> Objective-C 
// ----------------------------------------------------------------- 

void *IoObjcBridge_cValueForIoObject_ofType_error_(IoObjcBridge *self, IoObject *value, char *cType, char **error)
{
	*error = 0x0;
	
	if (*cType == '@')
	{
		if (ISSYMBOL(value))
		{ 
			id obj;
			//ByteArray *ba = ((ByteArray *)(((IoSymbol *)value)->data));
			char *s = CSTRING((IoSymbol *)value);
			//printf("ba=%p ", ba);
			//printf("%p \"%s\"", value, s);
			obj = [NSString stringWithCString:s];
			DATA(self)->cValue.o = obj; 
		}
		
		else if (ISNUMBER(value))
		{ 
			DATA(self)->cValue.o = [NSNumber numberWithInt:IoNumber_asInt((IoNumber *)value)]; 
		}
		
		else if (ISIO2OBJC(value))
		{ 
			DATA(self)->cValue.o = Io2Objc_object((Io2Objc *)value); 
		}
		
		else if (ISNIL(value))
		{ 
			DATA(self)->cValue.o = nil; 
		}
		
		else /*if (ISOBJECT(value))*/
		{ 
			DATA(self)->cValue.o = IoObjcBridge_proxyForIoObject_(self, value); 
		}
		
		return (void *)&(DATA(self)->cValue.o);
	}
	// else if ( *cType == '#')
	else if ( *cType == ':') /* selector */
			{
		if (ISSYMBOL(value))
		{
			DATA(self)->cValue.sel = sel_getUid(CSTRING((IoSymbol *)value));
			if (!DATA(self)->cValue.sel)
			{ 
				*error = "no selector found by that name"; 
			}
			else
			{ 
				return (void *)&(DATA(self)->cValue.sel); 
			}
		}
		else 
		{ 
			*error = "requires a string"; 
		}
			} 
			else if ( *cType == 'c' || *cType == 'C')
			{
				if (ISNUMBER(value))
				{
					DATA(self)->cValue.c = (char)IoNumber_asDouble((IoNumber *)value);
					return (void *)&(DATA(self)->cValue.c);
				}
				*error = "requires a number";
			}
			else if ( *cType == 's' || *cType == 'S' )
			{
				if (ISNUMBER(value))
				{
					DATA(self)->cValue.s = IoNumber_asInt((IoNumber *)value);
					return (void *)&(DATA(self)->cValue.i);
				}
				*error = "requires a number";
			}  
			else if ( *cType == 'i' || *cType == 'I' )
			{
				if (ISNUMBER(value))
				{
					DATA(self)->cValue.i = IoNumber_asInt((IoNumber *)value);
					return (void *)&(DATA(self)->cValue.i);
				}
				*error = "requires a number";
			}
			else if ( *cType == 'l' || *cType == 'L' )
			{
				if (ISNUMBER(value))
				{
					DATA(self)->cValue.l = (long)IoNumber_asDouble((IoNumber *)value);
					return (void *)&(DATA(self)->cValue.l);
				}
				*error = "requires a number";
			}
			else if ( *cType == 'f')
			{
				if (ISNUMBER(value))
				{
					DATA(self)->cValue.f = (float)IoNumber_asDouble((IoNumber *)value);
					return (void *)&(DATA(self)->cValue.f);
				}
				*error = "requires a number";
			}
			else if ( *cType == 'd')
			{
				if (ISNUMBER(value))
				{
					DATA(self)->cValue.d = IoNumber_asDouble((IoNumber *)value);
					return (void *)&(DATA(self)->cValue.f);
				}
				*error = "requires a number";
			}
			// bitfield
			// void
			else if ( *cType == 'r' || *cType == '*' )
			{
				DATA(self)->cValue.cp = (void *)CSTRING((IoSymbol *)value);
				return (void *)&(DATA(self)->cValue.cp);
			}
			
			else if (!strcmp(cType, "{_NSPoint=ff}"))
			{
				if (ISPOINT(value))
				{
					IoVector *p = (IoVector *)value;
					NUM_TYPE x, y, z;
					IoVector_rawGetXYZ(p, &x, &y, &z);
					DATA(self)->cValue.point.x = x;
					DATA(self)->cValue.point.y = y;
					return (void *)&(DATA(self)->cValue.point);
				}
				*error = "requires a Point";
			}
			else if (!strcmp(cType, "{_NSSize=ff}"))
			{
				if (ISPOINT(value))
				{
					IoVector *p = (IoVector *)value;
					NUM_TYPE x, y, z;
					IoVector_rawGetXYZ(p, &x, &y, &z);
					DATA(self)->cValue.size.width  = x;
					DATA(self)->cValue.size.height = y;
					return (void *)&(DATA(self)->cValue.size);
				}
				*error = "requires a Point";
			}
			else if (!strcmp(cType, "{_NSRect={_NSPoint=ff}{_NSSize=ff}}"))
			{
				if (ISLIST(value))
				{
					IoList *l = (IoList *)value;
					List *list = IoList_rawList(l);
					IoVector *p1 = List_at_(list, 0);
					IoVector *p2 = List_at_(list, 1);
					
					if (p1 && p2 && ISVECTOR(p1) && ISVECTOR(p2))
					{
						NUM_TYPE x1, y1, z1;
						NUM_TYPE x2, y2, z2;
						IoVector_rawGetXYZ(p1, &x1, &y1, &z1);
						IoVector_rawGetXYZ(p2, &x2, &y2, &z2);
						
						DATA(self)->cValue.rect.origin.x    = x1;
						DATA(self)->cValue.rect.origin.y    = y1;
						DATA(self)->cValue.rect.size.width  = x2;
						DATA(self)->cValue.rect.size.height = y2;
						return (void *)&(DATA(self)->cValue.rect);
					}
				}
				*error = "requires a List containing 2 points";
			}
			else
			{ 
				*error = "no match for argument type"; 
			}
			
			return 0x0;
}

/* --- method name buffer ----------------------------------- */

void IoObjcBridge_setMethodBuffer_(IoObjcBridge *self, char *name)
{
	int length = strlen(name);
	if (length > DATA(self)->methodNameBufferSize)
	{
		DATA(self)->methodNameBuffer = realloc(DATA(self)->methodNameBuffer, length+1);
		DATA(self)->methodNameBufferSize = length;
	}
	strcpy(DATA(self)->methodNameBuffer, name);
}

char *IoObjcBridge_ioMethodFor_(IoObjcBridge *self, char *name)
{
	/*
	 IoObjcBridge_setMethodBuffer_(self, name);
	 {
		 char *s = DATA(self)->methodNameBuffer;
		 while (*s) { if (*s == ':') {*s = '_';} s++; }
	 }
	 return DATA(self)->methodNameBuffer;
	 */
	return name;
}

NSString *IoObjcBridge_objcMethodFor_(IoObjcBridge *self, char *name)
{
	/*
	 IoObjcBridge_setMethodBuffer_(self, name);
	 {
		 char *s = DATA(self)->methodNameBuffer;
		 while (*s) { if (*s == '_') {*s = ':';} s++; }
	 }
	 return DATA(self)->methodNameBuffer;
	 */
	return [NSString stringWithCString: name encoding: NSASCIIStringEncoding];
}

/* --- new classes -------------------------------------------- */

IoObject *IoObjcBridge_newClassNamed_withProto_(IoObjcBridge *self, IoObject *locals, IoMessage *m)
{
	IoSymbol *ioSubClassName = IoMessage_locals_symbolArgAt_(m, locals, 0);
	IoObject *proto          = IoMessage_locals_valueArgAt_(m, locals, 1);
	char *subClassName   = CSTRING(ioSubClassName);
	//Class sub = objc_lookUpClass(subClassName);
        Class sub = NSClassFromString([NSString stringWithCString: subClassName encoding: NSASCIIStringEncoding]);
	
	if (sub)
	{
		IoState_error_(IOSTATE, m,
					"Io ObjcBridge newClassNamed_withProto_ '%s' class already exists",
					subClassName);
	}
	
	sub = [ObjcSubclass newClassNamed:ioSubClassName proto:proto];
	return IoObjcBridge_proxyForId_(self, sub);;
}

char *IoObjcBridge_nameForTypeChar_(IoObjcBridge *self, char type)
{
	switch (type)
	{
		case '@': return "id";
		case '#': return "Class";
		case ':': return "SEL";
		case 'c': return "char";
		case 'C': return "unsigned char";
		case 's': return "short";
		case 'S': return "unsigned short";
		case 'i': return "int";
		case 'I': return "unsigned int";
		case 'l': return "long";
		case 'L': return "unsigned long";
		case 'f': return "float";
		case 'd': return "double";
		case 'b': return "bitfield";
		case 'v': return "void";
		case '?': return "undefined";
		case '^': return "pointer";
		case '*': return "char *";
		case '[': return "array B";
		case ']': return "array E";
		case '(': return "union B";
		case ')': return "union E";
		case '{': return "struct B";
		case '}': return "struct A";
	}
	return "?";
}
