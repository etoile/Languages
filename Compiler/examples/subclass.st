NSObject subclass: Test [
	wibble [
		'wibble' log.
	]
]
Test subclass: SmalltalkTool [
	run [
		self wibble.
	]
]
