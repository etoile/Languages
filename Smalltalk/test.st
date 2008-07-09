NSObject subclass: TestObject 
[ 
	addObject: r
	[ | a b c|
		r log.
		b := 12.
		c := b + 5.
		a := NSMutableArray array. 
		a log.
		r run:[ a log. ].
	]

	wibble: a
	[
		^a.
	]
]
