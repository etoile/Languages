NSObject subclass: TestBlock
[
	run: aBlock [
		aBlock log.
		'Evaluating block' log.
		aBlock value log.
	]
]
NSObject subclass: SmalltalkTool
[
	run [
	| task |
		"Test that messages with two arguments are parsed correctly"
		self log:'a' and:'b'.
		self log.
		"Test instantiation of other Smalltalk classes"
		self test:(TestBlock new).		
	]
	log [
		'Testing superclass message log.' log.
		super log.
	]

	log:a and:b [
		a log.
		b log.
	]

	test: r [
	| a b c|
		"Test array creation."
		a := {NSObject new. NSString new. NSNumber new}.
		a log.
		"Test passing Smalltalk blocks to Objective-C objects"
		b := a map:[ :x | x log. x class. ].
		b map:[ :x | x log. x. ].
		a insertObject:'test' atIndex:2.
		b := a objectAtIndex:2.
		"You can't do this in Objective-C:"
		(a objectAtIndex:'2') log.
		((a objectAtIndex:2) == 'test') ifTrue:[ 'Array insert (auto-unboxing) worked!' log. ].
		a log.
		r log.
		"Test SmallInt messaging"
		b := 12.
		c := b + 5.
		b stringValue log.
		'Comparison result:' log.
		('wibble' == 'wible') log.
		('wibble' == 'wibble') log.
		('wibble' = 'wible') log.
		(12 = 12) log.
		('wibble' = 'wibble') log.
		'Wibble?' log.
		self wibble:c.
		b log.
		a := NSMutableArray array. 
		a log.
		self objectAtIndex: 12.
		"Test passing blocks to other bits of Smalltalk code"
		r run:[ a log. ].
		"Test message sends to nil."
		nil log.
		"Load an ObjC symbolic constant"
		@NSViewMaxYMargin log.
		(1 < 2) ifTrue:[ '1 < 2' log. ].
	]
	objectAtIndex: i [
		"This parameter will be passed as an unsigned int, because NSArray
		defines the type of this selector."
		'Boxed then unboxed value: ' log.
		i log.
	]

	wibble: a [
		'wibble called' log.
		a log.
		^a.
	]
]
