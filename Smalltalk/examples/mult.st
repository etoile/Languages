NSObject subclass: SmalltalkTool [
	run [
	| a |
		"This simple example just repeatedly multiplies a number by 12.  It
		demonstrates the overflow capabilities of Smalltalk.  At some point in
		the middle the value will be promoted to a big integer object - there
		will never be a loss of precision."
		a := 100.
		a := a * 12.
		a log.
		a := a * 12.
		a log.
		a := a * 12.
		a log.
		a := a * 12.
		a log.
		a := a * 12.
		a log.
		a := a * 12.
		a log.
		a := a * 12.
		a log.
		a := a * 12.
		a log.
		a := a * 12.
		a log.
		a := a * 12.
		a log.
		a := a * 12.
		a log.
	]
]
