NSObject subclass: SmalltalkTool
[
	run [
	| a |
		"You can explicitly create floats from most objects."
		a := '1.23e3' floatValue.
		"Or you can do it implicitly"
		a := a + '2.34'.
		a := a + 12.
		"Floats are objects"
		a log.
		"Currently instances of the BoxedFloat class"
		a class log.
	]
]
