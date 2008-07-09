NSObject subclass: TestObject 
[ 
	addObject: r
	[ | a b c|
		r log.
		b := 12.
		c := b + 5.
		b stringValue log.
		b := NSValue valueWithPointer:c.
		'Wibble?' log.
self wibble:c.
		b log.
		a := NSMutableArray array. 
		a log.
		r run:[ a log. ].
	]

	wibble: a
	[
'wibble called' log.
a log.
		^a.
	]
]
