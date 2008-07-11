NSObject subclass: SmalltalkTool
[
	run 
	[
		self test:(TestBlock new).		
	]
	test: r
	[ | a b c|
		a := #(NSObject new, NSString new, NSNumber new).
		a log.
		b := a map:[ :x | x log. x class. ].
		b map:[ :x | x log. x. ].
		r log.
		b := 12.
		c := b + 5.
		b stringValue log.
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
