#include "IoState.h"
#include "IoObject.h"

IoObject *IoVector_proto(void *state);
IoObject *IoBox_proto(void *state);

void IoAAVectorInit(IoState *self, IoObject *context)
{
	IoObject_setSlot_to_(context, SIOSYMBOL("Vector"), IoVector_proto(self));
	IoObject_setSlot_to_(context, SIOSYMBOL("Box"), IoBox_proto(self));
  {
  char *s = "Point := Vector clone setSize(2) \n"
	"Color := Vector clone setSize(4)\n"
	"Box copy := method(box,\n"
	"    self origin copy(box origin)\n"
	"    self size copy(box size)\n"
	")\n"
	"Sequence asVector := method(Vector clone copy(self))\n"
	"vector := method(\n"
	"\tv := Vector clone setSize(0)\n"
	"\tcall message arguments foreach(i, arg, \n"
	"\t\tv atPut(i, call sender doMessage(arg))\n"
	"\t)\n"
	"\tv\n"
	")\n"
	"Vector asString := method(\n"
	"\tb := Sequence clone\n"
	"\tb appendSeq(\"vector(\")\n"
	"\tsizeMinusOne := size - 1\n"
	"\tfor (i, 0, sizeMinusOne,\n"
	"\t\tb appendSeq(self at(i) asString)\n"
	"\t\tif (i != sizeMinusOne, b appendSeq(\", \"))\n"
	"\t)\n"
	"\tb appendSeq(\")\")\n"
	"\tb asString\n"
	")\n"
	"Vector print := method(self asString print)\n";
  IoState_on_doCString_withLabel_(self, context, s, "Vector.io");
  }

}
