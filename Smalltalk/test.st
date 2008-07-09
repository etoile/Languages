NSObject subclass: TestObject 
[ 
	addObject: r
	[ | a b c|
		r log.
		a := NSMutableArray array. 
		a log.
		r run:[ a log. ].
	]

	wibble: a
	[
		^a.
	]
]
