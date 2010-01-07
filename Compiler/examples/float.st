NSObject subclass: SmalltalkTool
[
	run [
	| a |
		a := '1.23' floatValue.
		a := a + '2.34'.
		a := a + 12.
		a log.
	]	
]
