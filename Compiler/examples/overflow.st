NSObject subclass: SmalltalkTool [

	run [
	| a |
		'Testing multiplication' log.
		a := 2.
		a := a * a.
		a log.
		a := a * a.
		a log.
		a := a * a.
		a log.
		a := a * a.
		a log.
		a := a * a.
		a log.
		'Testing addition' log.
		a := 1000000000.
		a log.
		a := a + a.
		a log.
		a := a + a.
		a log.
		a := a + a.
		a log.
		a := a + a.
		a log.
		'Testing subtraction' log.
		a := 0 - 1000000000.
		a log.
		a := a - 1000000000.
		a log.
		a := a - 1000000000.
		a log.
		a := a - 1000000000.
		a log.
	]
]
