NSObject subclass: SmalltalkTool [
	run [
		(self foo) log.
	]
	foo [
	| a |
		a :=  'pop!'.
		^ 'pip!'.
	]
]

