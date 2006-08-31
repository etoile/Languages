#include "IoState.h"
#include "IoObject.h"

IoObject *IoVector_proto(void *state);
IoObject *IoBox_proto(void *state);

void IoVectorInit(IoObject *context)
{
	IoState *self = ((IoObject *)context)->tag->state;

	IoObject_setSlot_to_(context, SIOSYMBOL("Vector"), IoVector_proto(self));

	IoObject_setSlot_to_(context, SIOSYMBOL("Box"), IoBox_proto(self));
	char *s = "Point := Vector clone setSize(2) \n"
		"Color := Vector clone setSize(4) \n"
		"Box copy := method(box, \n"
		"  self origin copy(box origin) \n"
		"  self size copy(box size) \n"
		") \n"
		"Sequence asVector := method(Vector clone copy(self)) \n"
		"vector := method( \n"
		"  v := Vector clone setSize(0) \n"
		"  call message arguments foreach(i, arg, \n"
		"    v atPut(i, call sender doMessage(arg)) \n"
		"  ) \n"
		"  v \n"
		") \n"
		"Vector do( \n"
		"  asString := method(self serialized asString) \n"
		"  serialized := method(b, \n"
		"    if(b == nil, b := Sequence clone) \n"
		"    b appendSeq(\"vector(\") \n"
		"    if(size > 0, \n"
		"      for (i, 0, size - 1, \n"
		"        if(i > 0, b appendSeq(\", \")) \n"
		"        b appendSeq(self at(i) asString) \n"
		"      ) \n"
		"    ) \n"
		"    b appendSeq(\")\") \n"
		"   ) \n"
		"  asSimpleString := method( \n"
		"    result := Sequence clone appendSeq(\"vector(\") \n"
		"    if(size > 0, \n"
		"      values := result \n"
		"      for(i, 0, size - 1, \n"
		"        if(i > 0, values appendSeq(\", \")) \n"
		"        values appendSeq(self at(i) asString) \n"
		"        if(values == result and values size > 30, \n"
		"          values := Sequence clone \n"
		"        ) \n"
		"      ) \n"
		"      if(values != result, \n"
		"        if(values size <= 10, \n"
		"          result appendSeq(values), \n"
		"          result appendSeq(\", ...\") \n"
		"        ) \n"
		"      ) \n"
		"    ) \n"
		"    result appendSeq(\")\") \n"
		"  ) \n"
		"  print := method(self serialized print) \n"
		") \n"
		"Box do( \n"
		"  asString := method(self serialized asString) \n"
		"  serialized := method(b, \n"
		"    if(b == nil, b := Sequence clone) \n"
		"    b appendSeq(\"Box clone do(\") \n"
		"    b appendSeq(\"setOrigin(\") \n"
		"    origin serialized(b) \n"
		"    b appendSeq(\"); \") \n"
		"    b appendSeq(\"setSize(\") \n"
		"    size serialized(b) \n"
		"    b appendSeq(\"));\") \n"
		"  ) \n"
		"  print := method(self serialized print) \n"
		") \n"
		"Vector \n";

	IoState_on_doCString_withLabel_(self, context, s, "Vector.io");

}
