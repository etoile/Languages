/*#io
Box ioDoc(
		docCopyright("Steve Dekorte", 2002)
		docLicense("BSD revised")
		docDependsOn("Vector")
		docCategory("Math")
		docDescription("A primitive for fast operations on rectangles.")
		*/

#include "IoBox.h"
#include "IoState.h"
#include "IoNumber.h"
#include <math.h>

#define DATA(self) ((IoBoxData *)IoObject_dataPointer(self))

void *IoMessage_locals_boxArgAt_(IoMessage *self, void *locals, int n)
{
	IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
	
	if (!ISBOX(v)) 
	{
		IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Box");
	}
	
	return v;
}

IoTag *IoBox_tag(void *state)
{
	IoTag *tag = IoTag_newWithName_("Box");
	tag->state = state;
	tag->freeFunc  = (TagFreeFunc *)IoBox_free;
	tag->cloneFunc = (TagCloneFunc *)IoBox_rawClone;
	tag->markFunc  = (TagMarkFunc *)IoBox_mark;
	return tag;
}

IoBox *IoBox_proto(void *state)
{
	IoBox *self = IoObject_new(state);
	self->tag = IoBox_tag(state);
	
	IoObject_setDataPointer_(self, calloc(1, sizeof(IoBoxData)));
	
	DATA(self)->origin = IoVector_newWithSize_(state, 3);
	DATA(self)->size   = IoVector_newWithSize_(state, 3);
	
	IoState_registerProtoWithFunc_(state, self, IoBox_proto);
	
	{
		IoMethodTable methodTable[] = { 
		{"set", IoBox_set},
		{"origin", IoBox_origin},
		{"size", IoBox_size},
			
		{"width", IoBox_width},
		{"height", IoBox_height},
		{"depth", IoBox_depth},
			
		{"setOrigin", IoBox_setOrigin},
		{"setSize", IoBox_setSize},
		{"Union", IoBox_Union},
			
		{"print", IoBox_print},
		{"containsPoint", IoBox_containsPoint},
		{"intersectsBox", IoBox_intersectsBox},
			/*
			 {"asString", IoBox_asString},
			 {"Min", IoBox_Min},
			 {"Max", IoBox_Max},
			 */
		{NULL, NULL},
		};
		IoObject_addMethodTable_(self, methodTable);
	}
	return self;
}

IoBox *IoBox_rawClone(IoBox *proto) 
{ 
	IoBox *self = IoObject_rawClonePrimitive(proto);
	IoObject_setDataPointer_(self, cpalloc(IoObject_dataPointer(proto), sizeof(IoBoxData)));
	
	DATA(self)->origin = IOCLONE(DATA(proto)->origin);
	DATA(self)->size   = IOCLONE(DATA(proto)->size);
	return self; 
}

IoBox *IoBox_new(void *state)
{
	IoBox *proto = IoState_protoWithInitFunction_(state, IoBox_proto);
	return IOCLONE(proto);
}

/* ----------------------------------------------------------- */

void IoBox_rawCopy(IoBox *self, IoBox *other)
{
	IoVector_rawCopy(DATA(self)->origin, DATA(other)->origin);
	IoVector_rawCopy(DATA(self)->size, DATA(other)->size);
}

void IoBox_rawSet(IoBox *self, 
			   NUM_TYPE x, 
			   NUM_TYPE y, 
			   NUM_TYPE z, 
			   NUM_TYPE w, 
			   NUM_TYPE h, 
			   NUM_TYPE d)
{
	IoVector_rawSetXYZ(DATA(self)->origin, x, y, z);
	IoVector_rawSetXYZ(DATA(self)->size,   w, h, d);
}

IoBox *IoBox_newSet(void *state, 
				NUM_TYPE x, 
				NUM_TYPE y, 
				NUM_TYPE z, 
				NUM_TYPE w, 
				NUM_TYPE h, 
				NUM_TYPE d)
{
	IoBox *self = IoBox_new(state);
	IoVector_rawSetXYZ(DATA(self)->origin, x, y, z);
	IoVector_rawSetXYZ(DATA(self)->size,   w, h, d);
	return self;
}

void IoBox_free(IoBox *self) 
{ 
	free(IoObject_dataPointer(self)); 
}

void IoBox_mark(IoBox *self) 
{ 
	IoObject_shouldMark((IoObject *)DATA(self)->origin); 
	IoObject_shouldMark((IoObject *)DATA(self)->size); 
}

IoVector *IoBox_rawOrigin(IoBox *self) 
{ 
	return DATA(self)->origin; 
}

IoVector *IoBox_rawSize(IoBox *self)   
{ 
	return DATA(self)->size; 
}

/* ----------------------------------------------------------- */

IoObject *IoBox_origin(IoBox *self, IoObject *locals, IoMessage *m)
{ 
	/*#io
	docSlot("origin", "Returns the point object for the origin of the box.")
	*/
	
	return DATA(self)->origin;
}

IoObject *IoBox_size(IoBox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("size", "Returns the point object for the size of the box. ")
	*/
	
	return DATA(self)->size;
}

IoObject *IoBox_width(IoBox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("width", "Same as; size x")
	*/
	
	return IoVector_x(DATA(self)->size, locals, m);
}

IoObject *IoBox_height(IoBox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("height", "Same as; size y")
	*/
	
	return IoVector_y(DATA(self)->size, locals, m);
}

IoObject *IoBox_depth(IoBox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("depth", "Same as; size z")
	*/
	
	return IoVector_z(DATA(self)->size, locals, m);
}

IoObject *IoBox_set(IoBox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("set(origin, size)", "Copies the values in origin and size to set the box's origin and size.")
	*/
	
	IoVector_rawCopy(DATA(self)->origin, IoMessage_locals_pointArgAt_(m, locals, 0));
	IoVector_rawCopy(DATA(self)->size,   IoMessage_locals_pointArgAt_(m, locals, 1));
	return self;
}

IoObject *IoBox_setOrigin(IoBox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setOrigin(aPoint)", "Copies the values in aPoint to the box's origin point. ")
	*/
	
	IoVector_rawCopy(DATA(self)->origin, IoMessage_locals_pointArgAt_(m, locals, 0));
	return self;
}

IoObject *IoBox_setSize(IoBox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setSize(aPoint)", "Copies the values in aPoint to the box's size point.  ")
	*/
	
	IoVector_rawCopy(DATA(self)->size, IoMessage_locals_pointArgAt_(m, locals, 0));
	return self;
}

IoObject *IoBox_copy(IoBox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("copy(aBox)", "Copies the values of aBox to the receiver.")
	*/
	
	IoBox *other = IoMessage_locals_boxArgAt_(m, locals, 0);
	IoVector_rawCopy(DATA(self)->origin, DATA(other)->origin);
	IoVector_rawCopy(DATA(self)->size,   DATA(other)->size);
	return self;
}

IoObject *IoBox_print(IoBox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("print", "Prints a string representation of the receiver to the standard output.")
	*/
	
	IoState_print_(IOSTATE, "Box clone set(");
	IoVector_print(DATA(self)->origin, locals, m);
	IoState_print_(IOSTATE, ", ");
	IoVector_print(DATA(self)->size, locals, m);
	IoState_print_(IOSTATE, ")");
	return self; 
}

IoObject *IoBox_Union(IoBox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("Union(aBox)", "Returns a new box containing the 2d union of the receiver and aBox.")
	*/
	
	IoBox *other = IoMessage_locals_boxArgAt_(m, locals, 0);
	IoBox_rawUnion(self, other);
	return self; 
}

void IoBox_rawUnion(IoBox *self, IoBox *other)
{
	NUM_TYPE x1, y1, z1;
	NUM_TYPE x2, y2, z2;
	NUM_TYPE ox1, oy1, oz1;
	NUM_TYPE ox2, oy2, oz2;
	
	IoVector_rawGetXYZ(DATA(self)->origin, &x1, &y1, &z1);
	IoVector_rawGetXYZ(DATA(self)->size, &x2, &y2, &z2);
	x2 += x1; 
	y2 += y1;
	
	/*
	 NUM_TYPE x1 = DATA(self)->origin->x->n;
	 NUM_TYPE y1 = DATA(self)->origin->y->n;
	 NUM_TYPE x2 = x1 + DATA(self)->size->x->n;
	 NUM_TYPE y2 = y1 + DATA(self)->size->y->n;
	 */
	IoVector_rawGetXYZ(DATA(other)->origin, &ox1, &oy1, &oz1);
	IoVector_rawGetXYZ(DATA(other)->size, &ox2, &oy2, &oz2);
	ox2 += ox1; 
	oy2 += oy1;
	
	/*
	 NUM_TYPE ox1 = DATA(other)->origin->x->n;
	 NUM_TYPE oy1 = DATA(other)->origin->y->n;
	 NUM_TYPE ox2 = ox1 + DATA(other)->size->x->n;
	 NUM_TYPE oy2 = oy1 + DATA(other)->size->y->n;
	 */
	{
		NUM_TYPE ux1 = x1 > ox1 ? x1 : ox1;
		NUM_TYPE uy1 = y1 > oy1 ? y1 : oy1;
		NUM_TYPE ux2 = x2 < ox2 ? x2 : ox2;
		NUM_TYPE uy2 = y2 < oy2 ? y2 : oy2;
		
		NUM_TYPE uw = ux2 - ux1;
		NUM_TYPE uh = uy2 - uy1;
		
		IoVector_rawSetXYZ(DATA(self)->origin, ux1, uy1, 0);
		IoVector_rawSetXYZ(DATA(self)->size, uw > 0 ? uw:0, uh > 0 ? uh:0, 0);
		/*
		 DATA(self)->origin->x->n = ux1;
		 DATA(self)->origin->y->n = uy1;
		 DATA(self)->size->x->n = uw > 0 ? uw:0;
		 DATA(self)->size->y->n = uh > 0 ? uh:0;
		 */
	}
}

int IoBox_rawContains3dPoint(IoBox *self, IoVector *otherPoint)
{
	// should really do this with stack allocated Vectors or something
	
	NUM_TYPE px, py, pz;
	NUM_TYPE x, y, z;
	NUM_TYPE w, h, d;
	
	IoVector_rawGetXYZ(otherPoint, &px, &py, &pz);
	IoVector_rawGetXYZ(DATA(self)->origin, &x, &y, &z);
	IoVector_rawGetXYZ(DATA(self)->size, &w, &h, &d);
	
	// Fix to allow Boxes with negative w, h, or d to contain Points. -jk
	if (w < 0) { w = -w; x = -x; px = -px; }
	if (h < 0) { h = -h; y = -y; py = -py; }
	if (d < 0) { d = -d; x = -x; px = -px; }
	
	return (px >= x) && 
		  (py >= y) && 
		  (pz >= z) &&
			    
		  (px <= x + w) && 
		  (py <= y + h) && 
		  (pz <= z + d);
}


IoObject *IoBox_containsPoint(IoBox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("containsPoint(aPoint)", "Returns true if aPoint is within the receiver's bounds, false otherwise.")
	*/
	
	int result;
	
	IoVector *otherPoint = IoMessage_locals_pointArgAt_(m, locals, 0);
	
	/*
	if (IoVector_rawSize(otherPoint) <= 3)
	{
		return IOBOOL(self, IoBox_rawContains3dPoint(self, otherPoint));
	}
	else
	*/
	{	
		Vector *bo = IoVector_rawVector(IoBox_rawOrigin(self));
		Vector *bs = IoVector_rawVector(IoBox_rawSize(self));
		Vector *p = IoVector_rawVector(otherPoint);

		// do a malloc since the vectors might be large
		
		Vector *b1 = Vector_clone(bo);
		Vector *b2 = Vector_clone(bs);
		
		// make bo2 the box endpoint
		
		Vector_addArray_(b2, b1); 
		
		// ensure bo1 is on the left bottom and bo2 is on the top right
		
		Vector_Min(b1, b2);
		Vector_Max(b2, bo);  
		
		result = Vector_greaterThanOrEqualTo_(p, b1) && Vector_greaterThanOrEqualTo_(b2, p);
		
		Vector_free(b1);
		Vector_free(b2);
		
		return IOBOOL(self, result);
	}
}

IoObject *IoBox_intersectsBox(IoBox *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("IoBox_intersectsBox(aBox)", "Returns true if aBox is within the receiver's bounds, false otherwise.")
	*/
	
	int result = 0;
	
	/*
	Vector *bo = IoVector_rawVector(IoBox_rawOrigin(self));
	Vector *bs = IoVector_rawVector(IoBox_rawSize(self));
	Vector *p = IoVector_rawVector(otherPoint);
	
	// do a malloc since the vectors might be large
	
	Vector *bo1 = Vector_clone(bo);
	Vector *bo2 = Vector_clone(bs);
	
	// make bo2 the box endpoint
	
	Vector_addArray_(bo2, bo1); 
	
	// ensure bo1 is on the left bottom and bo2 is on the top right
	
	Vector_Min(bo1, bo2);
	Vector_Max(bo2, bo);  
	
	result = Vector_greaterThanOrEqualTo_(p, bo1) && Vector_greaterThanOrEqualTo_(bo1, p);
	
	Vector_free(bo1);
	Vector_free(bo2);
	*/
	return IOBOOL(self, result);
}


/*

 IoObject *IoBox_Min(IoBox *self, IoObject *locals, IoMessage *m)
 { 
	 IoBox *other = IoMessage_locals_pointArgAt_(m, locals, 0);
	 if (self->x->n > DATA(other)->x->n) self->x->n = DATA(other)->x->n;
	 if (self->y->n > DATA(other)->y->n) self->y->n = DATA(other)->y->n;
	 if (self->z->n > DATA(other)->z->n) self->z->n = DATA(other)->z->n;
	 return self; 
 }
 
 IoObject *IoBox_Max(IoBox *self, IoObject *locals, IoMessage *m)
 { 
	 IoBox *other = IoMessage_locals_pointArgAt_(m, locals, 0);
	 if (self->x->n < DATA(other)->x->n) self->x->n = DATA(other)->x->n;
	 if (self->y->n < DATA(other)->y->n) self->y->n = DATA(other)->y->n;
	 if (self->z->n < DATA(other)->z->n) self->z->n = DATA(other)->z->n;
	 return self; 
 }
 */
