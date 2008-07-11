NSObject subclass: TestBlock
[
	run: aBlock
	[
		aBlock log.
		aBlock value log.
	]
]
NSObject subclass: SmalltalkTool
[
	run [
	| task nil |
		self log:'a' and:'b'.
		self test:(TestBlock new).		
	]
	log:a and:b
[
a log.
b log.
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
