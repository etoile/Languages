NSObject subclass: SmalltalkTool [
	| block |
	run [
		self setBlock.
		self callBlock.
	]
	setBlock [
		| a |
		a := 'Local variable'.
		block := [ a log.  ].
	]
	callBlock [
		'Testing retained block:' log.
		block value.
	]
]
