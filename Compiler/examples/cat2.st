NSObject subclass: SmalltalkTool [
| foo |
	run [
		'Setting ivar' log.
		foo := 12.
		'set ivar' log.
		self wibble.
	]
]
SmalltalkTool extend [
	wibble [ 'foo' log. foo log. ]
]
