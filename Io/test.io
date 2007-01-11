/* #io
   docCopyright("Yen-Ju Chen", 2006)
   docLicense("BSD revised") */

ObjcBridge autoLookupClassNamesOn
//ObjcBridge debugOn

/*** Test basic classes of Foundation ***/

/* NSDate */
ObjcDate := ObjcBridge classNamed("NSDate")

date := ObjcDate date
date println

/* NSNumber */
N := ObjcBridge classNamed("NSNumber")
n := N numberWithInt:(200)
n println

/* NSString */
ObjcString := ObjcBridge classNamed("NSString")
string := ObjcString stringWithCString:("Test")
string println
/* Warning: string doesn't become io object automatically. 
   Therefore, it is still NSString */

/*** Test bridge crossing and direct calls of Objc methods written in Io ***/

TestObject := Object clone

/* Create Io object */
TestObject do(
	myMethod := method(
		return 5
	)

	test := method(
		"Object clone test" println
		self myMethod println
	)
)

myTest := TestObject clone
myTest test

/* Create Objc class in Io as a subclass of NSObject */
TestObject := NSObject newSubclassNamed:("TestObject")

/* Add Objc methods written in Io */
TestObject do(
	myMethod := method(
		return 5
	)

	test := method(
		"NSObject newSubclassNamed alloc init test" println
		self myMethod println
	)
)

/* Run a bunch tests to check the inheritance chain set up for the previous
   class on Io side */

/* For the class first */
"TestObject class slots " print
TestObject slotNames println
"TestObject class protos " print
TestObject protos println
"TestObject class "
TestObject println

/* Then for the instance */
myTest := TestObject alloc init //myTest prependProto(TestObject)
"TestObject instance slots " print
myTest slotNames println
"TestObject instance protos " print
myTest protos println
"TestObject instance " print
myTest println
myTest test

/*** Test ApplicationKit ***/

/* Does not work now
application := ObjcBridge classNamed("NSApplication") sharedApplication
frame := Box clone set( vector(200, 500), vector(420, 150) )

Window := ObjcBridge classNamed("NSWindow")
win := Window alloc initWithContentRect:styleMask:backing:defer:(frame, 15, 2, 0)
win setTitle:("Io Window")

frame := Box clone set( vector(330, 10), vector(76, 25) )
quitButton := ObjcBridge classNamed("NSButton") alloc initWithFrame:(frame)
quitButton setBezelStyle:(4)
quitButton setTarget:(NSApp)
quitButton setAction:("terminate:")
quitButton setTitle:("Quit")

win contentView addSubview:(quitButton)
win display
win orderFrontRegardless

application run
*/
