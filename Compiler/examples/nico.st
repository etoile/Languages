NSObject subclass: SmalltalkTool [
	run
	[
	| x v |
		x := 'test'. v :=  x componentsSeparatedByString: ':'.
		x log. v log.
	]	
]
