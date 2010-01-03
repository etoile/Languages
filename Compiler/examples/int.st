NSObject subclass: SmalltalkTool [
	run []

	intValue [ 
	| a |
		a := 6.
		a := a + 7.
		^a.
	]
]
