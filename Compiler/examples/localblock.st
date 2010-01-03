NSObject subclass: SmalltalkTool [
	| i |
	run [
		i := 'foo'.
		self test:100.
	]
	test: n [
	| a|
	a := 12.
	1 ifTrue:[ self log. i log. a log. n log. ].
	]
	
]
