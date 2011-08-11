<< types = '{"data" = "@8@0:4";}'>>
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
	| +foo i |
	init [ super init. ]
	+alloc [
		foo := 'Class variable value set in class method'.
		'Class method alloc called' log.
		^super alloc.
	]
	cascade1 [
		foo log.
		'First cascade message sent' log.
	]
	cascade2 [
		'Second cascade message sent' log.
	]
	testNestedBlocks[
	| a |
		'testing nested blocks' log.
		i := 'Accessing ivar from nested block'.
		a := 'Accessing local from nested block'.
		1 ifTrue:[
			1 ifTrue:[
				1 ifTrue:[
					'Block context: ' log.
					blockContext log.
					a log.
					a := 'Set local in nested block'.
					i log.
					i := 'Set ivar in nested block'.
				]
			].
		].
		i log.
		a log.
	]
	run [
	| task |
		[ :x :y :z | x log. y log. z log.] value: '1' value: '2' value: '3'.
		'Testing C functions, sqrt(60), fdim(60,12):' log.
		(C sqrt: 60) log.
		(C fdim: {60. 12}) log.
		(C f: 60 dim: 12) log.
		"Test cascade messages:"
		self cascade1; cascade2 ; cascade1; cascade2.
		(TestBlock new) log.
		(NSObject new) log.
		(NSString new) log.
		i := 'ivar'.
		"Test that messages with two arguments are parsed correctly"
		self log:'a' and:'b'.
		nil data.
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

	callBlock: x [
		x log.
		(x value: 'foo') log.
	]
	test: r [
	| a b c|
		C putchar: 'f'.
		C putchar: 'o'.
		C putchar: 'o'.
		"Test array creation."
		a := {NSObject new. NSString new. NSArray new}.
		NSGarbageCollector defaultCollector collectExhaustively.
		a log.
		(a objectAtIndex: 0) flibble.
		'calling map' log.
		self callBlock: [ :x | '1-arg block' log. x log. x ].
		b map: [ :x | x log. x. ].
		([ :x | x class ] value: foo) log.
		"Test passing Smalltalk blocks to Objective-C objects"
		b := a map: [ :x | x log. x class. ].
		'calling map again' log.
		a insertObject:'test' atIndex:2.
		NSGarbageCollector defaultCollector collectExhaustively.
		b := a objectAtIndex:2.
		"You can't do this in Objective-C:"
		(a objectAtIndex:'2') log.
		((a objectAtIndex:2) == 'test') ifTrue:[ 'Array insert (auto-unboxing) worked!' log. ].
		a log.
		r log.
		"Test SmallInt messaging"
		b := 12.
		c := b + 5.
		NSGarbageCollector defaultCollector collectExhaustively.
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
		NSGarbageCollector defaultCollector collectExhaustively.
		self objectAtIndex: 12.
		"Test passing blocks to other bits of Smalltalk code"
		r run:[ a log. ].
		"Test message sends to nil."
		nil log.
		"Load an ObjC symbolic constant"
		@NSViewMaxYMargin log.
		(1 < 2) ifTrue:[ '1 < 2' log. ].
		self testNestedBlocks.
		NSGarbageCollector defaultCollector collectExhaustively.
		#symbol log.
		'Testing if empty methods return self:' log.
		self init log.
		"A really big integer:"
		'BigInt literals: ' log.
		10000000000000000 log.
		'BigInt arithmetic: ' log.
		(10000000000000000 + 12) log.
		'floats:' log.
		((12.5 + '13') + 14) log.
		'nil == nil' log.
		nil == nil log.
		'nil = nil' log.
		nil = nil log.
		NSGarbageCollector defaultCollector collectExhaustively.
	]
	objectAtIndex: i [
		"This parameter will be passed as an unsigned int, because NSArray
		defines the type of this selector."
		'Boxed then unboxed value: ' log.
		i log.
	]
	dealloc [
		'destroying Smalltalk tool.' log.
	]

	wibble: a [
		'wibble called' log.
		a log.
		^a.
	]
]
NSObject extend [
	flibble [ 'category method called' log. ]
]
